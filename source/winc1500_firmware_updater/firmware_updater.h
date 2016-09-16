#ifndef FIRMWARE_UPDATER_H_
#define FIRMWARE_UPDATER_H_

typedef struct __attribute__((__packed__)) {
  uint8_t command;
  uint32_t address;
  uint32_t arg1;
  uint16_t payloadLength;

  // payloadLenght bytes of data follows...
} UartPacket;

void receivePacket(UartPacket *pkt, uint8_t *payload);
void updater_loop();

#endif
