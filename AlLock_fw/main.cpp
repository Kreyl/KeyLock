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
#define FNAME_LNG_MAX   36
#define CODE_LNG_MAX    6

static struct Settings_t {
    char SndKeyBeep[FNAME_LNG_MAX], SndKeyDrop[FNAME_LNG_MAX], SndPassError[FNAME_LNG_MAX], SndOpen[FNAME_LNG_MAX];
    char SndClose[FNAME_LNG_MAX], SndKeyCrash[FNAME_LNG_MAX];
    // Codes
    char CodeA[CODE_LNG_MAX+1], CodeB[CODE_LNG_MAX+1], ServiceCode[CODE_LNG_MAX+1]; // Because of trailing \0
    int8_t CodeALength, CodeBLength;     // Length of 0 means empty, negative length means crash
    char Complexity[4];
    // Colors
    Color_t ColorDoorOpen, ColorDoorOpening, ColorDoorClosed, ColorDoorClosing;
    uint32_t BlinkDelay;
    // Timings
    uint32_t DoorCloseDelay, KeyDropDelay;
    // Methods
    uint8_t Read();
} Settings;

enum EntrResult_t {entNA, entError, entOpen};
struct Codecheck_t {
    uint32_t Timer;
    char EnteredCode[CODE_LNG_MAX+1];   // Because of trailing \0
    uint8_t EnteredLength;
    EntrResult_t EnterResult;
    void Task(void);
    void Reset(void) { EnteredLength=0; EnterResult=entNA; memset(EnteredCode, 0, CODE_LNG_MAX); }
} Codecheck;

//static TmrKL_t TmrOneSecond {MS2ST(999), evtIdEverySecond, tktPeriodic}; // Measure battery periodically
#endif

int main() {
#if 1 // Low level init
    // ==== Setup clock ====
    Clk.SetCoreClk(cclk12MHz);
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

    SD.Init();
    if (Settings.Read() != retvOk) {
        Printf("Settings read error\r");
        while(true);    // nothing to do if config not read
    }

//    SimpleSensors::Init();

    // Leds
    i2c1.Init();
    Pca9635.Init();
    LedA.Init();
    LedB.Init();

    // Sound
    Sound.Init();
//    while(true) {
        Sound.Play("alive.wav");
//        chThdSleepMilliseconds(7002);
//    }

//    TmrOneSecond.StartOrRestart();
#endif

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

//            case evtIdButtons:
////                Printf("Btn: %u %u\r", Msg.BtnEvtInfo.BtnID, Msg.BtnEvtInfo.Type);
//                if(Msg.BtnEvtInfo.BtnID == 2 and Msg.BtnEvtInfo.Type == beLongPress) {
//                    PowerOff();
//                }
                break;

            case evtIdEverySecond:
//                Printf("Second\r");
                break;


            case evtIdSoundEnd:
                Printf("Sound end\r");
                break;

            default: break;
        } // switch
    } // while true
}

uint8_t Settings_t::Read() {
    // Sound names
    if(ini::ReadStringTo("lock.ini", "Sound", "KeyBeep",   Settings.SndKeyBeep,   FNAME_LNG_MAX) != retvOk) return retvFail;
    if(ini::ReadStringTo("lock.ini", "Sound", "KeyDrop",   Settings.SndKeyDrop,   FNAME_LNG_MAX) != retvOk) return retvFail;
    if(ini::ReadStringTo("lock.ini", "Sound", "KeyCrash",  Settings.SndKeyCrash,  FNAME_LNG_MAX) != retvOk) return retvFail;
    if(ini::ReadStringTo("lock.ini", "Sound", "PassError", Settings.SndPassError, FNAME_LNG_MAX) != retvOk) return retvFail;
    if(ini::ReadStringTo("lock.ini", "Sound", "Open",      Settings.SndOpen,      FNAME_LNG_MAX) != retvOk) return retvFail;
    if(ini::ReadStringTo("lock.ini", "Sound", "Close",     Settings.SndClose,     FNAME_LNG_MAX) != retvOk) return retvFail;

    // ======= Codes =======
    // If length == 0 then code is empty. If code is negative (-1 for examle) then side is crashed
    // === Code A ===
    if(ini::ReadStringTo("lock.ini", "Code", "CodeA", Settings.CodeA, CODE_LNG_MAX) != retvOk) return retvFail;
    Settings.CodeALength = strlen(Settings.CodeA);
    if (Settings.CodeALength != 0) {
        if (Settings.CodeA[0] == 'N') Settings.CodeALength = 0;     // Check if None
        if (Settings.CodeA[0] == '-') Settings.CodeALength = -1;    // Check if crashed
    }

    // === Code B ===
    if(ini::ReadStringTo("lock.ini", "Code", "CodeB", Settings.CodeB, CODE_LNG_MAX) != retvOk) return retvFail;
    Settings.CodeBLength = strlen(Settings.CodeB);
    if (Settings.CodeBLength !=0) {
        if (Settings.CodeB[0] == 'N') Settings.CodeBLength = 0;     // Check if None
        if (Settings.CodeB[0] == '-') Settings.CodeBLength = -1;    // Check if crashed
    }

    // === Service code ===
    if(ini::ReadStringTo("lock.ini", "Code", "ServiceCode", Settings.ServiceCode, CODE_LNG_MAX) != retvOk) return retvFail;

    // Complexity
    if(ini::ReadStringTo("lock.ini", "Code", "Complexity", Settings.Complexity, 3) != retvOk) return retvFail;

    // Colors
    if(ini::ReadColor("lock.ini", "Colors", "DoorOpen",    &Settings.ColorDoorOpen)    != retvOk) return retvFail;
    if(ini::ReadColor("lock.ini", "Colors", "DoorOpening", &Settings.ColorDoorOpening) != retvOk) return retvFail;
    if(ini::ReadColor("lock.ini", "Colors", "DoorClosed",  &Settings.ColorDoorClosed)  != retvOk) return retvFail;
    if(ini::ReadColor("lock.ini", "Colors", "DoorClosing", &Settings.ColorDoorClosing) != retvOk) return retvFail;
    if(ini::Read<uint32_t>("lock.ini", "Colors", "BlinkDelay", &Settings.BlinkDelay)   != retvOk) return retvFail;

    // Timings
    if(ini::Read<uint32_t>("lock.ini", "Timings", "DoorCloseDelay", &Settings.DoorCloseDelay) != retvOk) return retvFail;
    if(ini::Read<uint32_t>("lock.ini", "Timings", "KeyDropDelay",   &Settings.KeyDropDelay)   != retvOk) return retvFail;

    return retvOk;
}

#if 1 // ======================= Command processing ============================
void OnCmd(Shell_t *PShell) {
    Cmd_t *PCmd = &PShell->Cmd;
//    Printf("%S  ", PCmd->Name);

    // Handle command
    if(PCmd->NameIs("Ping")) PShell->Ack(retvOk);
    else if(PCmd->NameIs("Version")) PShell->Printf("%S %S\r", APP_NAME, XSTRINGIFY(BUILD_TIME));

//    else if(PCmd->NameIs("GetBat")) { PShell->Printf("Battery: %u\r", Audio.GetBatteryVmv()); }

    else PShell->Ack(retvCmdUnknown);
}
#endif
