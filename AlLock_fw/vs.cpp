#include "vs.h"
#include "shell.h"

VS1011_t Vs;

void VS1011_t::WriteData(uint8_t* ABuf, uint16_t ACount) {
    if(ACount == 0) return;
    if(BusyWait() != VS_OK) return;     // Get out in case of timeout
    XDCS_Lo();  // Start transmission
    for(uint16_t i=0; i<ACount; i++) {
        ISpi.ReadWriteByte(ABuf[i]);
    }
    XDCS_Hi();
}
void VS1011_t::WriteTrailingZeroes() {
    if(BusyWait() != VS_OK) return;     // Get out in case of timeout
    XDCS_Lo();                          // Start transmission
    for (uint8_t i=0; i<VS_TRAILING_0_COUNT; i++)
        ISpi.ReadWriteByte(0);         // Send data
    XDCS_Hi();                          // End transmission
}

#if 1 // ================================= IRQ =================================
void VS1011_t::IIrqHandler() {
//    PrintfI("DreqIrq\r");
//    ISendNexChunk();
}

// DMA irq
extern "C"
void SIrqDmaHandler(void *p, uint32_t flags) {
    chSysLockFromISR();
    dmaStreamDisable(VS_DMA);
    Vs.IDmaIsIdle = true;
//    PrintfI("DMAIrq\r");
//    Vs.ISendNexChunk();
    chSysUnlockFromISR();
}
#endif

void VS1011_t::Init() {
    // ==== GPIO init ====
    PinSetupOut(VS_XCS, omPushPull);
    PinSetupOut(VS_XDCS, omPushPull);
    PinSetupOut(VS_RST, omPushPull);
    Rst_Lo();
    XCS_Hi();
    XDCS_Hi();
    PinSetupAlterFunc(VS_XCLK, omPushPull, pudNone, AF1);
    PinSetupAlterFunc(VS_SO,   omPushPull, pudNone, AF1);
    PinSetupAlterFunc(VS_SI,   omPushPull, pudNone, AF1);
    // DREQ IRQ
    IDreq.Init(ttRising);

    // ==== SPI init ====
    uint32_t div;
    if(ISpi.PSpi == SPI1) div = Clk.APB2FreqHz / VS_MAX_SPI_BAUDRATE_HZ;
    else div = Clk.APB1FreqHz / VS_MAX_SPI_BAUDRATE_HZ;
    SpiClkDivider_t ClkDiv = sclkDiv2;
    if     (div > 128) ClkDiv = sclkDiv256;
    else if(div > 64) ClkDiv = sclkDiv128;
    else if(div > 32) ClkDiv = sclkDiv64;
    else if(div > 16) ClkDiv = sclkDiv32;
    else if(div > 8)  ClkDiv = sclkDiv16;
    else if(div > 4)  ClkDiv = sclkDiv8;
    else if(div > 2)  ClkDiv = sclkDiv4;

    ISpi.Setup(boMSB, cpolIdleLow, cphaFirstEdge, ClkDiv);
    ISpi.Enable();
    ISpi.EnableTxDma();

    // ==== DMA ====
    // Here only unchanged parameters of the DMA are configured.
    dmaStreamAllocate     (VS_DMA, IRQ_PRIO_MEDIUM, SIrqDmaHandler, NULL);
    dmaStreamSetPeripheral(VS_DMA, &VS_SPI->DR);
    dmaStreamSetMode      (VS_DMA, VS_DMA_MODE);
    IDmaIsIdle = true;

    // ==== Init VS ====
    Rst_Lo();
    chThdSleepMicroseconds(45);
    Rst_Hi();
    chThdSleepMicroseconds(45);

    // Send init commands
    CmdWrite(VS_REG_MODE, (VS_SM_DIFF | VS_ICONF_ADSDI | VS_SM_SDINEW));
    CmdWrite(VS_REG_CLOCKF, (0x8000 + (12288000/2000)));
    CmdWrite(VS_REG_VOL, 0);
    CmdWrite(VS_REG_MIXERVOL, (VS_SMV_ACTIVE | VS_SMV_GAIN2));
    CmdWrite(VS_REG_RECCTRL, VS_SARC_DREQ512);
    SetVolume(20);
    IZero = 0;
}

bool VS1011_t::IsBusy (void) {
    return (PinIsLo(VS_DREQ) || !IDmaIsIdle);
}

uint8_t VS1011_t::BusyWait(void) {
    uint32_t Timeout = VS_TIMEOUT;
    while(IsBusy()) {
        Timeout--;
        if(Timeout == 0) {
            Printf("VS timeout\r");
            return VS_TIMEOUT_ER;
        }
    }
    return VS_OK;
}

// ==== Amplifier ====
void VS1011_t::AmplifierOn() {
    // Setup GPIO0 as output
    CmdWrite(VS_REG_WRAMADDR, 0xC017);
    CmdWrite(VS_REG_WRAM,     0x0001);
    // Set output high
    CmdWrite(VS_REG_WRAMADDR, 0xC019);
    CmdWrite(VS_REG_WRAM,     0x0001);
}
void VS1011_t::AmplifierOff() {
    // Setup GPIO0 as output
    CmdWrite(VS_REG_WRAMADDR, 0xC017);
    CmdWrite(VS_REG_WRAM,     0x0001);
    // Set output low
    CmdWrite(VS_REG_WRAMADDR, 0xC019);
    CmdWrite(VS_REG_WRAM,     0x0000);
}

// ==== Commands ====
uint8_t VS1011_t::CmdRead(uint8_t AAddr, uint16_t* AData) {
    uint8_t IReply;
    uint16_t IData;
    // Wait until ready
    if ((IReply = BusyWait()) != VS_OK) return IReply; // Get out in case of timeout
    XCS_Lo();   // Start transmission
    ISpi.ReadWriteByte(VS_READ_OPCODE);  // Send operation code
    ISpi.ReadWriteByte(AAddr);           // Send addr
    *AData = ISpi.ReadWriteByte(0);      // Read upper byte
    *AData <<= 8;
    IData = ISpi.ReadWriteByte(0);       // Read lower byte
    *AData += IData;
    XCS_Hi();   // End transmission
    return VS_OK;
}
uint8_t VS1011_t::CmdWrite(uint8_t AAddr, uint16_t AData) {
    uint8_t IReply;
    // Wait until ready
    if ((IReply = BusyWait()) != VS_OK) return IReply; // Get out in case of timeout
    XCS_Lo();                       // Start transmission
    ISpi.ReadWriteByte(VS_WRITE_OPCODE); // Send operation code
    ISpi.ReadWriteByte(AAddr);           // Send addr
    ISpi.ReadWriteByte(AData >> 8);      // Send upper byte
    ISpi.ReadWriteByte(0x00FF & AData);  // Send lower byte
    XCS_Hi();                       // End transmission
    return VS_OK;
}
