#include "hal.h"
#include "MsgQ.h"
#include "kl_lib.h"
#include "Sequences.h"
#include "shell.h"
#include "uart.h"
//#include "kl_sd.h"
//#include "kl_fs_utils.h"
//#include "battery_consts.h"

#if 1 // =============== Low level ================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartTxParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart;
void OnCmd(Shell_t *PShell);
void ITask();

// ==== External UART ====
// Settings
static const UartParams_t ExtParams = {
        EXT_UART,
        EXTUART_GPIO, EXTUART_TX_PIN,
        true,
        EXTUART_GPIO, EXTUART_RX_PIN,
        // DMA
        EXTUART_DMA_TX, EXTUART_DMA_RX,
        UART_DMA_TX_MODE(0), UART_DMA_RX_MODE(0),
};
CmdUart_t ExtUart {&ExtParams};

//static TmrKL_t TmrOneSecond {MS2ST(999), evtIdEverySecond, tktPeriodic}; // Measure battery periodically
#endif

int main() {
#if 1 // Low level init
    // ==== Setup clock ====
//    Clk.SetCoreClk(cclk24MHz);
    Clk.UpdateFreqValues();

    // ==== Init OS ====
    halInit();
    chSysInit();

    // ==== Init Hard & Soft ====
    EvtQMain.Init();
    Uart.Init(115200);
    Printf("\r%S %S\r\n", APP_NAME, XSTRINGIFY(BUILD_TIME));
    Clk.PrintFreqs();

//    ExtUart.Init(115200);
//    ExtUart.
//    SD.Init();
//    SimpleSensors::Init();

//    i2c2.Init();

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


//            case evtIdSoundFileEnd:
////                Printf("SoundFile end: %u\r", Msg.Value);
//                break;

            default: break;
        } // switch
    } // while true
}

extern "C" {
bool sdc_lld_is_card_inserted(SDCDriver *sdcp) {
    return true;
}
bool sdc_lld_is_write_protected(SDCDriver *sdcp) {
    return false;
}

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