#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "Serial.h"
#include "firmware_updater.h"

extern "C" {
	#include "common/include/nm_common.h"
	#include "spi_flash/include/spi_flash.h"
}

extern SerialConsole Serial;

static const int MAX_PAYLOAD_SIZE = 1024;

#define CMD_READ_FLASH        0x01
#define CMD_WRITE_FLASH       0x02
#define CMD_ERASE_FLASH       0x03
#define CMD_MAX_PAYLOAD_SIZE  0x50
#define CMD_HELLO             0x99


bool isBigEndian() {
  uint32_t test = 0x11223344;
  uint8_t *pTest = reinterpret_cast<uint8_t *>(&test);
  return pTest[0] == 0x11;
}

uint32_t fromNetwork32(uint32_t from) {
  static const bool be = isBigEndian();
  if (be) {
    return from;
  } else {
    uint8_t *pFrom = reinterpret_cast<uint8_t *>(&from);
    uint32_t to;
    to = pFrom[0]; to <<= 8;
    to |= pFrom[1]; to <<= 8;
    to |= pFrom[2]; to <<= 8;
    to |= pFrom[3];
    return to;
  }
}

uint16_t fromNetwork16(uint16_t from) {
  static bool be = isBigEndian();
  if (be) {
    return from;
  } else {
    uint8_t *pFrom = reinterpret_cast<uint8_t *>(&from);
    uint16_t to;
    to = pFrom[0]; to <<= 8;
    to |= pFrom[1];
    return to;
  }
}

uint32_t toNetwork32(uint32_t to) {
  return fromNetwork32(to);
}

uint16_t toNetwork16(uint16_t to) {
  return fromNetwork16(to);
}



void receivePacket(UartPacket *pkt, uint8_t *payload) {
  // Read command
  uint8_t *p = reinterpret_cast<uint8_t *>(pkt);
  uint16_t l = sizeof(UartPacket);
  while (l > 0) {
    int c = Serial.read();
    if (c == -1)
      continue;
    *p++ = c;
    l--;
  }

  // Convert parameters from network byte order to cpu byte order
  pkt->address = fromNetwork32(pkt->address);
  pkt->arg1 = fromNetwork32(pkt->arg1);
  pkt->payloadLength = fromNetwork16(pkt->payloadLength);

  // Read payload
  l = pkt->payloadLength;
  while (l > 0) {
    int c = Serial.read();
    if (c == -1)
      continue;
    *payload++ = c;
    l--;
  }
}

// Allocated statically so the compiler can tell us
// about the amount of used RAM
static UartPacket pkt;
static uint8_t payload[MAX_PAYLOAD_SIZE];

void updater_loop() {
  receivePacket(&pkt, payload);

  if (pkt.command == CMD_HELLO) {
    if (pkt.address == 0x11223344 && pkt.arg1 == 0x55667788)
      Serial.print("v10000");
  }

  if (pkt.command == CMD_MAX_PAYLOAD_SIZE) {
    uint16_t res = toNetwork16(MAX_PAYLOAD_SIZE);
    Serial.write(reinterpret_cast<uint8_t *>(&res), sizeof(res));
  }

  if (pkt.command == CMD_READ_FLASH) {
    uint32_t address = pkt.address;
    uint32_t len = pkt.arg1;
    if (spi_flash_read(payload, address, len) != M2M_SUCCESS) {
      Serial.println("ER");
    } else {
      Serial.write(payload, len);
      Serial.print("OK");
    }
  }

  if (pkt.command == CMD_WRITE_FLASH) {
    uint32_t address = pkt.address;
    uint32_t len = pkt.payloadLength;
    if (spi_flash_write(payload, address, len) != M2M_SUCCESS) {
      Serial.print("ER");
    } else {
      Serial.print("OK");
    }
  }

  if (pkt.command == CMD_ERASE_FLASH) {
    uint32_t address = pkt.address;
    uint32_t len = pkt.arg1;
    if (spi_flash_erase(address, len) != M2M_SUCCESS) {
      Serial.print("ER");
    } else {
      Serial.print("OK");
    }
  }
}
