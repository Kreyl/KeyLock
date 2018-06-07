#include "hal.h"
#include "MsgQ.h"
#include "kl_lib.h"
#include "color.h"
#include "Sequences.h"
#include "shell.h"
#include "uart.h"
#include "kl_sd.h"
#include "kl_fs_utils.h"
#include "kl_i2c.h"
#include "leds_pca.h"
#include "sound.h"
#include "btns.h"
#include "kl_adc.h"
//#include "battery_consts.h"

#if 1 // =============== Low level ================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart{&CmdUartParams};
void OnCmd(Shell_t *PShell);
void ITask();

// ==== External UART ====
static const UartParams_t ExtUartParams(115200, EXT_UART_PARAMS);
CmdUart_t ExtUart {&ExtUartParams};

// Leds
LedPcaBlinker_t LedA(0), LedB(2);

// ==== Settings ====
// File names
#define SETTINGS_FNAME      "lock.cfg"
#define SND_BTN_BEEP        "beep.wav"
#define SND_BTN_DROP        "drop.wav"
#define SND_CRASH           "sparks.wav"
#define SND_ERROR           "error.wav"
#define SND_OPEN            "open.wav"
#define SND_CLOSE           "close.wav"
// Master toys
#define SND_ENTER_CODE      "entrcode.wav"
#define SND_CODE_SAVED      "ncodesvd.wav"
#define SND_EMPTY_SAVED     "ecodesvd.wav"

#define CODE_MAX_LEN    36  // 6 digits
// If length == 0 then code is empty. If code is negative (-1 for examle) then side is crashed

class Settings_t {
public:
    // Codes (Len+1 because of trailing \0)
    char CodeA[CODE_MAX_LEN+1], CodeB[CODE_MAX_LEN+1], CodeSrv[CODE_MAX_LEN+1], CodeMaster[CODE_MAX_LEN+1];
    int8_t CodeALen=0, CodeBLen=0, CodeSrvLen=0;     // Length of 0 means empty, negative length means crash
    uint8_t Complexity;
    // Timings
    uint32_t DoorCloseDelay = 12006, KeyDropDelay = 2799;
    // Methods
    uint8_t Load();
    uint8_t Save();
} Settings;

struct ECode_t {
    TmrKL_t Tmr {MS2ST(999), evtIdTimeToDrop, tktOneShot};
    bool IsEnteringSideCode = false;
    char EnteredCode[CODE_MAX_LEN+1];   // Because of trailing \0
    uint8_t EnteredLength;
    void Reset() {
        EnteredLength=0;
        memset(EnteredCode, 0, CODE_MAX_LEN+1);
        Tmr.Stop();
    }
} ECode;

enum DoorState_t {dsClosed, dsOpened, dsOpening, dsClosing};
class Door_t {
private:
    PinOutput_t IPin{LASER_ON_PIN, omPushPull};
    static void EvtJustClosed();
    static void EvtJustOpened();
    void LasersOn () { IPin.SetLo(); }
    void LasersOff() { IPin.SetHi(); }
public:
    TmrKL_t TmrClose {MS2ST(999), evtIdTimeToClose, tktOneShot};
    void Init() {
        IPin.Init();
        EvtJustClosed();
    }
    DoorState_t State;
    void Open();
    void Close();
} Door;

PinOutputPWM_t CoilA(COIL_A_SETUP);
PinOutputPWM_t CoilB(COIL_B_SETUP);

ftVoidVoid EvtOnSndEnd = nullptr;
void BtnHandler(uint8_t KeybrdSide, uint8_t Btn);
#endif

int main() {
    // ==== Setup clock ====
//    Clk.SetCoreClk(cclk48MHz);
    Clk.UpdateFreqValues();

    // ==== Init OS ====
    halInit();
    chSysInit();

    // ==== Init Hard & Soft ====
    JtagDisable();
    EvtQMain.Init();
    Uart.Init();
    Printf("\r%S %S\r\n", APP_NAME, XSTRINGIFY(BUILD_TIME));
    Clk.PrintFreqs();

    ExtUart.Init();
    ExtUart.StartRx();

    BtnsInit();
//    CoilA.Init();
//    CoilB.Init();

    // Leds
    i2c1.Init();
    Pca9635.Init();
    LedA.Init();
    LedB.Init();

    // SD and settings
    SD.Init();
    if (Settings.Load() != retvOk) { // nothing to do if config not read
        Printf("Settings read error\r");
        LedA.StartOrRestart(lsqError);
        LedB.StartOrRestart(lsqError);
        while(true) {
            chThdSleepMilliseconds(999);
        }
    }

    // Sound
    Sound.Init();
    Sound.Play("alive.wav");

    // Adc
    PinSetupAnalog(BAT_MEAS_PIN);
    Adc.Init();

    Door.Init();
    ECode.Reset();

    // ==== Main cycle ====
    ITask();
}

__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
//        Printf("Msg.ID %u\r", Msg.ID);
        switch(Msg.ID) {
            case evtIdShellCmd:
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;

            case evtIdButtons: BtnHandler(Msg.ValueID, Msg.Value); break;

            case evtIdTimeToClose: Door.Close(); break;

            case evtIdSoundEnd:
                if(EvtOnSndEnd != nullptr) EvtOnSndEnd();
                break;

            case evtIdTimeToDrop:
                ECode.Reset();
                ECode.IsEnteringSideCode = false;
                Sound.Play(SND_BTN_DROP);
                break;

            case evtIdAdcRslt:
//                Printf("Battery: %u mv\r", Msg.Value);
                // If battery discharged, indicate it
                if(Door.State == dsClosed and Msg.Value < 2700) {
                    LedA.StartOrContinue(lsqBatteryDischarged);
                }
                break;

            default: break;
        } // switch
    } // while true
}

void BtnHandler(uint8_t KeybrdSide, uint8_t Btn) {
//    Printf("Side: %u; Btn: %u\r", KeybrdSide, Btn);
    // Do not react if door is opening or closing
    if(Door.State == dsOpening or Door.State == dsClosing) return;

    int8_t CodeLen = (KeybrdSide == KBD_SIDE_A)? Settings.CodeALen : Settings.CodeBLen;
    char* Code = (KeybrdSide == KBD_SIDE_A)? Settings.CodeA : Settings.CodeB;
    // Check if side is crashed
    if(CodeLen < 0) {
        Sound.Play(SND_CRASH);
        return;
    }

    if((Btn >= 0) and (Btn <= 9)) { // Digit entered
        Sound.Play(SND_BTN_BEEP);   // Play sound always
        ECode.Tmr.StartOrRestart();
        // Add digit to entered string
        if(ECode.EnteredLength <= CODE_MAX_LEN) {
            ECode.EnteredCode[ECode.EnteredLength++] = '0' + Btn;
        }
    }
    // None-digit entered
    else {
        if(Btn == BTN_STAR) {   // Drop code
            ECode.IsEnteringSideCode = false;
            Sound.Play(SND_BTN_DROP);
        }

        else if(Btn == BTN_HASH) {
            // Close door if open
            if(Door.State == dsOpened) Door.Close();
            // Check ahead if door closed
            else if(Door.State == dsClosed) {
                // Complete entering code by master
                if(ECode.IsEnteringSideCode) {
                    ECode.IsEnteringSideCode = false;
                    if(KeybrdSide == KBD_SIDE_A) {
                        if(ECode.EnteredLength == 0) {  // Empty code
                            Settings.CodeALen = 0;
                            Settings.CodeA[0] = 0;  // Empty string
                            Settings.Save();
                            Sound.Play(SND_EMPTY_SAVED);
                        }
                        else { // Not empty code
                            Settings.CodeALen = strlen(ECode.EnteredCode);
                            strcpy(Settings.CodeA, ECode.EnteredCode);
                            Settings.Save();
                            Sound.Play(SND_CODE_SAVED);
                        }
                    }
                    else { // Side B
                        if(ECode.EnteredLength == 0) {  // Empty code
                            Settings.CodeBLen = 0;
                            Settings.CodeB[0] = 0;  // Empty string
                            Settings.Save();
                            Sound.Play(SND_EMPTY_SAVED);
                        }
                        else { // Not empty code
                            Settings.CodeBLen = strlen(ECode.EnteredCode);
                            strcpy(Settings.CodeB, ECode.EnteredCode);
                            Settings.Save();
                            Sound.Play(SND_CODE_SAVED);
                        }
                    } // side
                } // Complete entering code by master
                // Not entering master code
                else {
                    // Check if master code
                    if(strcmp(ECode.EnteredCode, Settings.CodeMaster) == 0) {
                        Sound.Play(SND_ENTER_CODE);
                        ECode.IsEnteringSideCode = true;
                    }
                    // Not master code
                    else {
                        // Check if side code is empty
                        if(CodeLen == 0) Door.Open();
                        // Not empty, check code
                        else {
                            if(strcmp(ECode.EnteredCode, Code) == 0) Door.Open();
                            else Sound.Play(SND_ERROR); // error
                        }
                    } // Not master code
                } // Not entering master code
            } // if door closed
        } // if hash
        // Reset ECode, be it star or hash
        ECode.Reset();
    } // if digit
}

#if 1 // ================================ Door =================================
void Door_t::Open(void) {
    Door.State = dsOpening;
    Sound.Play(SND_OPEN);
    EvtOnSndEnd = EvtJustOpened;
    TmrClose.StartOrRestart();
    LedA.StartOrRestart(lsqDoorOpening);
    LedB.StartOrRestart(lsqDoorOpening);
}

void Door_t::Close(void) {
    Door.State = dsClosing;
    Sound.Play(SND_CLOSE);
    EvtOnSndEnd = EvtJustClosed;
    TmrClose.Stop();
    LedA.StartOrRestart(lsqDoorClosing);
    LedB.StartOrRestart(lsqDoorClosing);
}

void Door_t::EvtJustClosed(void) {
    EvtOnSndEnd = nullptr;
    LedA.StartOrRestart(lsqDoorClosed);
    LedB.StartOrRestart(lsqDoorClosed);
    Door.LasersOn();
    Door.State = dsClosed;
    Printf("Door is closed\r");
}
void Door_t::EvtJustOpened(void) {
    EvtOnSndEnd = nullptr;
    Door.State = dsOpened;
    LedA.StartOrRestart(lsqDoorOpen);
    LedB.StartOrRestart(lsqDoorOpen);
    Door.LasersOff();
    Printf("Door is opened\r");
}
#endif

#if 1 // ============================== Settings ===============================
uint8_t Settings_t::Load() {
    if(csv::OpenFile(SETTINGS_FNAME) == retvOk) {
        while(true) {
            if(csv::ReadNextLine() != retvOk) break;
            char *Name;
            if(csv::GetNextToken(&Name) != retvOk) continue;
            // Codes
            if(csv::TryLoadString(Name, "CodeA", Settings.CodeA, CODE_MAX_LEN+1) == retvOk) {
                Settings.CodeALen = strlen(Settings.CodeA);
                if(Settings.CodeA[0] == 'N') Settings.CodeALen = 0;  // Check if None
                if(Settings.CodeA[0] == '-') Settings.CodeALen = -1; // Check if crashed
                continue;
            }
            if(csv::TryLoadString(Name, "CodeB", Settings.CodeB, CODE_MAX_LEN+1) == retvOk) {
                Settings.CodeBLen = strlen(Settings.CodeB);
                if(Settings.CodeB[0] == 'N') Settings.CodeBLen = 0;  // Check if None
                if(Settings.CodeB[0] == '-') Settings.CodeBLen = -1; // Check if crashed
                continue;
            }
            if(csv::TryLoadString(Name, "ServiceCode", Settings.CodeSrv, CODE_MAX_LEN+1) == retvOk) {
                Settings.CodeSrvLen = strlen(Settings.CodeSrv);
                continue;
            }
            if(csv::TryLoadString(Name, "MasterCode", Settings.CodeMaster, CODE_MAX_LEN+1) == retvOk) {
                continue;
            }
            // Complexity
            csv::TryLoadParam<uint8_t>(Name, "Complexity", &Settings.Complexity);
            // Timings
            if(csv::TryLoadParam<uint32_t>(Name, "DoorCloseDelay", &Settings.DoorCloseDelay) == retvOk) {
                Door.TmrClose.SetNewPeriod_ms(Settings.DoorCloseDelay);
                continue;
            }
            if(csv::TryLoadParam<uint32_t>(Name, "KeyDropDelay", &Settings.KeyDropDelay) == retvOk) {
                ECode.Tmr.SetNewPeriod_ms(Settings.KeyDropDelay);
                continue;
            }
        } // while true
        csv::CloseFile();
    }
    Printf("Settings loaded\r");
    return retvOk;
}

uint8_t Settings_t::Save() {
    if(TryOpenFileRewrite(SETTINGS_FNAME, &CommonFile) == retvOk) {
        f_printf(&CommonFile, "CodeA = %S\r\n", Settings.CodeA);
        f_printf(&CommonFile, "CodeB = %S\r\n", Settings.CodeB);
        f_printf(&CommonFile, "ServiceCode = %S\r\n", Settings.CodeSrv);
        f_printf(&CommonFile, "Complexity = %u\r\n", Settings.Complexity);
        f_printf(&CommonFile, "DoorCloseDelay = %u\r\n", Settings.DoorCloseDelay);
        f_printf(&CommonFile, "KeyDropDelay = %u\r\n", Settings.KeyDropDelay);
        f_printf(&CommonFile, "MasterCode = %S\r\n", Settings.CodeMaster);
        f_close(&CommonFile);
        return retvOk;
    }
    else return retvFail;
}
#endif

#if 1 // ======================= Command processing ============================
void OnCmd(Shell_t *PShell) {
    Cmd_t *PCmd = &PShell->Cmd;
//    Printf("%S  ", PCmd->Name);

    // Handle command
    if(PCmd->NameIs("Ping")) PShell->Ack(retvOk);
    else if(PCmd->NameIs("Version")) PShell->Printf("%S %S\r", APP_NAME, XSTRINGIFY(BUILD_TIME));

//    else if(PCmd->NameIs("GetBat")) { PShell->Printf("Battery: %u\r", Audio.GetBatteryVmv()); }

    // Codes
    else if(PCmd->NameIs("State")) {    // Get state: "service code","CodeA","CodeB", "Complexity"
        PShell->Printf("%S,%S,%S,%S\r\n", Settings.CodeSrv, Settings.CodeA, Settings.CodeB, Settings.Complexity);
    }

    else if(PCmd->NameIs("SetCodeSrv")) {
        char *S;
        if(PCmd->GetNextString(&S) == retvOk) {
            uint32_t Len = strlen(S);
            if(Len <= CODE_MAX_LEN) {
                if(Len == 0) Settings.CodeSrvLen = 0;
                else {
                    strcpy(Settings.CodeSrv, S);
                    Settings.CodeSrvLen = Len;
                    Settings.Save();
                }
                PShell->Printf("Code changed\r\n");
            }
            else {
                PShell->Printf("Too long code\r\n");
                return;
            }
        }
    }

    else if(PCmd->NameIs("SetCodeA")) {
        char *S;
        if(PCmd->GetNextString(&S) == retvOk) {
            uint32_t Len = strlen(S);
            if(Len <= CODE_MAX_LEN) {
                if(Settings.CodeALen >= 0) { // editing is not allowed if side crashed
                    if(Len == 0) Settings.CodeALen = 0;
                    else {
                        strcpy(Settings.CodeA, S);
                        Settings.CodeALen = Len;
                        Settings.Save();
                    }
                }
                PShell->Printf("Code changed\r\n");
            }
            else {
                PShell->Printf("Too long code\r\n");
                return;
            }
        }
    }

    else if(PCmd->NameIs("SetCodeB")) {
        char *S;
        if(PCmd->GetNextString(&S) == retvOk) {
            uint32_t Len = strlen(S);
            if(Len <= CODE_MAX_LEN) {
                if(Settings.CodeBLen >= 0) { // editing is not allowed if side crashed
                    if(Len == 0) Settings.CodeBLen = 0;
                    else {
                        strcpy(Settings.CodeB, S);
                        Settings.CodeBLen = Len;
                        Settings.Save();
                    }
                }
                PShell->Printf("Code changed\r\n");
            }
            else {
                PShell->Printf("Too long code\r\n");
                return;
            }
        }
    }

    // Door
    else if(PCmd->NameIs("Open")) {
        if(Door.State == dsClosed) {
            Door.Open();
            Door.TmrClose.StartOrRestart();
            PShell->Printf("Door is opening\r\n");
        }
        else PShell->Printf("Door is open\r\n");
    }
    else if(PCmd->NameIs("Close")) {
        if(Door.State == dsOpened) {
            Door.Close();
            PShell->Printf("Door is closing\r\n");
        }
        else PShell->Printf("Door is closed\r\n");
    }

    else if(PCmd->NameIs("?")) {
        PShell->Printf("Commands:\r\nPing\r\nState\r\n"
                "SetCodeSrv\r\nSetCodeA\r\nSetCodeB\r\n"
                "Open\r\nClose\r\n");
    }

    else PShell->Printf("Bad command\r\n");
}
#endif
