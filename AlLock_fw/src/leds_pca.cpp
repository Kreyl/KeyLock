#include "leds_pca.h"
#include "kl_util.h"

Leds_t Leds;

void Leds_t::Init() {
    klPrintf("PCA Init\r");
    // Init OE pin
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    this->OutputEnable();
    // ==== Init variables ====
    // Map LED colors to PWM
    Led[0].PWM = (Color_t*)(&IPkt.PWM[0]);
    Led[1].PWM = (Color_t*)(&IPkt.PWM[3]);
    Led[2].PWM = (Color_t*)(&IPkt.PWM[6]);
    Led[3].PWM = (Color_t*)(&IPkt.PWM[9]);
    Led[4].PWM = (Color_t*)(&IPkt.PWM[12]);
    // Setup i2cCmd
    i2cCmd.Address = LED_I2C_ADDR;
    i2cCmd.DataToRead.Length = 0;   // Nothing to read
    i2cCmd.DataToWrite.Buf = (uint8_t *)&IPkt;
    i2cCmd.DataToWrite.Length = LEDS_PKT_SIZE;
    i2cCmd.Callback = 0;
    // Prepare initialization packet
    IPkt.ControlReg = 0b10000000;   // Autoincrement enable, first register is 0x00
    IPkt.Mode1 = 0b10000000;    // Non-sleep mode, do not respond to extra adresses
    //FPkt.Mode2 = 0b00000010;    // Group control = dimming, output not inverted, change on STOP, Open-drain, high-impedance when OE=1
    IPkt.Mode2 = 0b00000110;    // Group control = dimming, output not inverted, change on STOP, Push-Pull, high-impedance when OE=1
    for(uint8_t i=0; i<16; i++) IPkt.PWM[i]=0;  // Set all PWM to 0
    IPkt.GrpPWM = 0xFF;         // Max common brightness
    IPkt.GrpFreq = 0;           // Don't care as DBLINK bit is 0
    IPkt.LEDOut[0] = 0xFF;      // LED driver has individual brightness and group dimming/blinking and can be controlled through its PWMx register and the GRPPWM registers
    IPkt.LEDOut[1] = 0xFF;
    IPkt.LEDOut[2] = 0xFF;
    IPkt.LEDOut[3] = 0xFF;
    // ==== Init device ====
    i2cMgr.WriteBufferPoll(LED_I2C_ADDR, (uint8_t *)&IPkt, LEDS_PKT_SIZE);
}

void Leds_t::Task() {
    bool FHasChanged = false;
    for(uint8_t i=0; i<LED_COUNT; i++) {
        Led[i].Task();
        FHasChanged = FHasChanged || Led[i].HasChanged;
        Led[i].HasChanged = false;
    }
    if (FHasChanged) i2cMgr.AddCmd(i2cCmd);
}


// =============================== Light effects ===============================
void Leds_t::SetPWM(uint8_t AChannel, uint8_t AValue) {
    IPkt.PWM[AChannel] = AValue;
    i2cMgr.AddCmd(i2cCmd);
}

// =================================== Led =====================================
void Led_t::Task() {
    switch (Mode) {
        case lmBlink:
            if (Delay.Elapsed(&Timer, BlinkDelay)) {
                if(IsOn) *PWM = clBlack;
                else *PWM = Color;
                IsOn = !IsOn;
                HasChanged = true;
            }
            break;

        case lmSolid: break;
    }
}

void Led_t::Solid(Color_t AColor) {
    Mode = lmSolid;
    *PWM = AColor;
    HasChanged = true;
}

void Led_t::Blink(Color_t AColor, uint32_t ADelay) {
    if(ADelay == 0) {
        Solid(AColor);
        return;
    }
    Mode = lmBlink;
    Color = AColor;
    BlinkDelay = ADelay;
    IsOn = true;
    *PWM = AColor;
    Delay.Reset(&Timer);
    HasChanged = true;
}

