#pragma once

// Transmit message channels
#define TX_CHANNEL0 0
#define TX_CHANNEL1 1
#define TX_CHANNEL2 2

// Receive message channels
#define RX_CHANNEL0 0
#define RX_CHANNEL1 1

// CAN register definitions
#define CNF1        0x2A
#define CNF2        0x29
#define CNF3        0x28
#define CANINTE     0x2B
#define CANINTF     0x2C
#define TEC         0x1C
#define REC         0x1D
#define EFLG        0x2D
#define CANCTRL     0x0F    // 0xXF
#define CANSTAT     0x0E    // 0xXE
#define TXRTSCTRL   0x0D
#define BFPCTRL     0x0C
#define TXB0CTRL    0x30    //these are the three trasmit channels
#define TXB1CTRL    0x40
#define TXB2CTRL    0x50
#define RXB0CTRL    0x60    //two receive buffers
#define RXB1CTRL    0x70
#define TXB0SIDH    0x31
#define TXB1SIDH    0x41
#define TXB2SIDH    0x51
#define TXB0SIDL    0x32
#define TXB1SIDL    0x42
#define TXB2SIDL    0x52
#define TXB0EID8    0x33
#define TXB1EID8    0x43
#define TXB2EID8    0x53
#define TXB0EID0    0x34
#define TXB1EID0    0x44
#define TXB2EID0    0x54
#define TXB0DLC     0x35
#define TXB1DLC     0x45
#define TXB2DLC     0x55
#define TXB0Dm      0x36-0x3D   //todo
#define TXB1Dm      0x46-0x4D   //todo
#define TXB2Dm      0x56-0x5D
#define RXB0SIDH    0x61
#define RXB1SIDH    0x71
#define RXB0SIDL    0x62
#define RXB1SIDL    0x72
#define RXB0EID8    0x63
#define RXB1EID8    0x73
#define RXB0EID0    0x64
#define RXB1EID0    0x74
#define RXB0DLC     0x65
#define RXB1DLC     0x75
#define RXB0DM      0x66-0x6D //todo
#define RXB1DM      0x76-0x7D //todo
#define RXF0SIDH    0x00
#define RXF1SIDH    0x04
#define RXF2SIDH    0x08
#define RXF3SIDH    0x10
#define RXF4SIDH    0x14
#define RXF5SIDH    0x18
#define RXF0SIDL    0x01
#define RXF1SIDL    0x05
#define RXF2SIDL    0x09
#define RXF3SIDL    0x11
#define RXF4SIDL    0x15
#define RXF5SIDL    0x19
#define RXF0EID8    0x02
#define RXF1EID8    0x06
#define RXF2EID8    0x0A
#define RXF3EID8    0x12
#define RXF4EID8    0x16
#define RXF5EID8    0x1A
#define RXF0EID0    0x03
#define RXF1EID0    0x07
#define RXF2EID0    0x0B
#define RXF3EID0    0x13
#define RXF4EID0    0x17
#define RXF5EID0    0x1B
#define RXM0SIDH    0x20
#define RXM1SIDH    0x24
#define RXM0SIDL    0x21
#define RXM1SIDL    0x25
#define RXM0EID8    0x22
#define RXM1EID8    0x26
#define RXM0EID0    0x23
#define RXM1EID0    0x27
