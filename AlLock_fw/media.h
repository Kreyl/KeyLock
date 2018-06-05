/*
 * File:   media.h
 * Author: g.kruglov
 *
 * Created on June 15, 2011, 4:44 PM
 */

#pragma once

#include "ff.h"
#include <stdint.h>
#include "kl_lib.h"

#define SND_BUF_SIZE    512

enum sndState_t {sndStopped, sndPlaying, sndMustStop};

typedef struct {
    uint8_t Arr[SND_BUF_SIZE];
    UINT Size;
} SndBuf_t;

class Sound_t {
private:
    FIL IFile;
    SndBuf_t Buf1, Buf2, *CBuf;
    void UploadData();
    void StopNow();
public:
    sndState_t State;
    void Init();
    void Task();
    void Play(const char* AFilename);
    void Stop() { State = sndMustStop; }
    ftVoidVoid EvtPlayEnd;
};

extern Sound_t ESnd;
