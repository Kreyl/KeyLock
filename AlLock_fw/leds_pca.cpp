#include "leds_pca.h"
#include "kl_i2c.h"

Leds_t Leds;

// PCA registers
struct LedsPkt_t {
    uint8_t ControlReg = 0b10000000;   // Autoincrement enable, first register is 0x00
    // Registers
    uint8_t Mode1 = 0b10000000;    // Non-sleep mode, do not respond to extra adresses
    uint8_t Mode2 = 0b00000110;    // Group control = dimming, output not inverted, change on STOP, Push-Pull, high-impedance when OE=1
    uint8_t PWM[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};    // PWM channels
    uint8_t GrpPWM = 0xFF;         // Max common brightness
    uint8_t GrpFreq = 0;           // Don't care as DBLINK bit is 0
    // LED driver has individual brightness and group dimming/blinking and can be controlled through its PWMx register and the GRPPWM registers
    uint8_t LEDOut[4] = {0xFF, 0xFF, 0xFF, 0xFF};
} __attribute__ ((packed));
#define LEDS_PKT_SIZE   sizeof(LedsPkt_t)

static const LedsPkt_t IPkt;

void Leds_t::Init() {
    // Init OE pin
    PinSetupOut(LED_OE_PIN, omPushPull);
    OutputEnable();
    i2c1.Write(LED_I2C_ADDR, (uint8_t *)&IPkt, LEDS_PKT_SIZE);
}

void Leds_t::Task() {
}


// =============================== Light effects ===============================
void Leds_t::SetPWM(uint8_t AChannel, uint8_t AValue) {
    uint8_t Buf[2];
    Buf[0] = AChannel + 2;
    Buf[1] = AValue;
    i2c1.Write(LED_I2C_ADDR, Buf, 2);
}

void Leds_t::SetColor(uint8_t Indx, Color_t Clr) {
    uint8_t Buf[4];
    Buf[0] = (Indx * 3 + 2) | 0xA0;
    Buf[1] = Clr.R;
    Buf[2] = Clr.G;
    Buf[3] = Clr.B;
    i2c1.Write(LED_I2C_ADDR, Buf, 4);
}
