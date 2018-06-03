#include "media.h"
#include <string.h>
#include "kl_util.h"
#include "vs.h"

Sound_t ESnd;

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

void Sound_t::Play(const char* AFilename) {
    if (State == sndPlaying) StopNow();
    FRESULT rslt;
    // Open file
    //klPrintf("%S\r", AFilename);
    rslt = f_open(&IFile, AFilename, FA_READ+FA_OPEN_EXISTING);
    if (rslt != FR_OK) {
        if (rslt == FR_NO_FILE) klPrintf("%S: file not found\r", AFilename);
        else klPrintf("OpenFile error: %u", rslt);
        return;
    }
    // Check if zero file
    if (IFile.fsize == 0) {
        f_close(&IFile);
        klPrintf("Empty file\r");
        return;
    }
    // Fill buffer
    rslt = f_read(&IFile, Buf1.Arr, SND_BUF_SIZE, &Buf1.Size);
    if (rslt != FR_OK) {
        klPrintf("ReadFile 1 error: %u", rslt);
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
    // Setup VS
    Vs.Enable();
    Vs.AmplifierOn();
    // Load first bunch of data
    //klPrintf("Playing...\r");
    State = sndPlaying;
    UploadData();
}

void Sound_t::UploadData() {
    Vs.WriteData(CBuf->Arr, CBuf->Size);
    if(IFile.fptr < IFile.fsize) { // Not EOF
        // Switch to other buffer
        CBuf = (CBuf == &Buf1)? &Buf2 : &Buf1;
        // Fill the buffer
        FRESULT rslt = f_read(&IFile, CBuf->Arr, SND_BUF_SIZE, &CBuf->Size);
        if (rslt != FR_OK) {
            klPrintf("ReadFileU error: %u", rslt);
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
