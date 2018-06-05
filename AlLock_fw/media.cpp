#include "media.h"
#include <string.h>
#include "vs.h"
#include "shell.h"

Sound_t ESnd;

static THD_WORKING_AREA(waESndThread, 128);
__noreturn
static void ESndThread(void *arg) {
    chRegSetThreadName("ESnd");
    while(true) {
        ESnd.Task();
    }
}

void Sound_t::Task() {
    switch (State) {
        case sndPlaying:
            if (!Vs.IsBusy()) UploadData();
            break;

        case sndMustStop:
            if (!Vs.IsBusy()) StopNow();
            break;

        default: break; // just get out
    } // switch
}

void Sound_t::Init() {
    State = sndStopped;
    // Create and start thread
    chThdCreateStatic(waESndThread, sizeof(waESndThread), NORMALPRIO, (tfunc_t)ESndThread, NULL);
}

void Sound_t::Play(const char* AFilename) {
    if (State == sndPlaying) StopNow();
    FRESULT rslt;
    // Open file
    //klPrintf("%S\r", AFilename);
    rslt = f_open(&IFile, AFilename, FA_READ+FA_OPEN_EXISTING);
    if (rslt != FR_OK) {
        if (rslt == FR_NO_FILE) Printf("%S: file not found\r", AFilename);
        else Printf("OpenFile error: %u\r", rslt);
        return;
    }
    // Check if zero file
    if (IFile.obj.objsize == 0) {
        f_close(&IFile);
        Printf("Empty file\r");
        return;
    }
    // Fill buffer
    rslt = f_read(&IFile, Buf1.Arr, SND_BUF_SIZE, &Buf1.Size);
    if (rslt != FR_OK) {
        Printf("ReadFile 1 error: %u", rslt);
        State = sndStopped;
        return;
    }
//    if (Buf1.Size == SND_BUF_SIZE) {    // if first block is filled, there is reason to read second block
//        rslt = f_read(&IFile, Buf2.Arr, SND_BUF_SIZE, &Buf2.Size);
//        if (rslt != FR_OK) {
//            UART_StrUint("ReadFile2 error: ", rslt);
//            State = sndStopped;
//            return;
//        }
//    }
    // Setup buffers
    CBuf = &Buf1;
    Vs.AmplifierOn();
    // Load first bunch of data
    //klPrintf("Playing...\r");
    State = sndPlaying;
    UploadData();
}

void Sound_t::UploadData() {
    Vs.WriteData(CBuf->Arr, CBuf->Size);
    if(IFile.fptr < IFile.obj.objsize) { // Not EOF
        // Switch to other buffer
        CBuf = (CBuf == &Buf1)? &Buf2 : &Buf1;
        // Fill the buffer
        FRESULT rslt = f_read(&IFile, CBuf->Arr, SND_BUF_SIZE, &CBuf->Size);
        if (rslt != FR_OK) {
            Printf("ReadFileU error: %u\r", rslt);
            StopNow();
            return;
        }
    } // if not EOF
    else StopNow();
}

void Sound_t::StopNow() {
    Vs.AmplifierOff();
    Vs.Stop();
    Vs.WriteTrailingZeroes();
    f_close(&IFile);
    State = sndStopped;
    if (EvtPlayEnd != 0) EvtPlayEnd();
    //Vs.Disable();
    //klPrintf("Stopped\r");
}
