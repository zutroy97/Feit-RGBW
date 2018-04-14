#ifndef FEIT72031_H
#define FEIT72031_H

// If a pulse is between these two values, consider it a Header pulse
#define MIN_HEADER_LENGTH 4000
#define MAX_HEADER_LENGTH 8000

// between these two values, cosider it a 0
#define MIN_SPACE_LENGTH 150
#define MAX_SPACE_LENGTH 250

// between these two values, consider it a 1
#define MIN_MARK_LENGTH 500
#define MAX_MARK_LENGTH 750

// Command values for each button
#define BUTTON_POWER_ON     0x01
#define BUTTON_POWER_OFF    0x02
#define BUTTON_UP           0x03
#define BUTTON_DOUBLE_ARROW 0x06
#define BUTTON_MINUS        0x04
#define BUTTON_PLUS         0x08
#define BUTTON_DOWN         0x07
#define BUTTON_RED          0x05
#define BUTTON_GREEN        0x0C
#define BUTTON_BLUE         0x0B
#define BUTTON_WHITE        0x09

// Contains data about the RF packet
typedef union RFPacket {
  struct{
    uint8_t command :8;
    uint16_t remote :16;
    uint8_t notUsed : 8;
  } id;
  uint32_t value;
} ;

typedef struct RFBuilder_t {
  uint32_t lastChange;
  uint32_t interruptTimeStamp;
  byte bitPosition;
  byte isCapturing        : 1;
  byte isHighBitPosition  : 1;
  byte value              : 1;
  byte notUsed            : 5;
} ; //rfBuilder;

// Used when receiving/building a packet
typedef struct RemoteCommand_t
{
  byte count;
  union RFPacket packet;
  byte isReady = 0;
  unsigned long receiveTime;
} ;


void RfLoop();
void handleRfInterrupt();
#endif //ifndef FEIT72031_H