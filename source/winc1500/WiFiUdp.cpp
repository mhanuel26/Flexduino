/*
  WiFiUdp.cpp - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

extern "C" {
	#include "socket/include/socket.h"
	#include "driver/include/m2m_periph.h"
}

#include <string.h>
#include "WiFi101.h"
#include "WiFiUdp.h"
#include "WiFiClient.h"
#include "WiFiServer.h"


// wrapper stuff for some C library...
extern "C" void* C_WiFiUdp(){ return new WiFiUDP;}
extern "C" int8_t C_WiFiUdp_begin(void* WiFiUDPClass, uint16_t port) { int8_t ret = static_cast<WiFiUDP*>(WiFiUDPClass)->begin(port); return ret ? -1 : ret;}
extern "C" int C_available(void* WiFiUDPClass){ return static_cast<WiFiUDP*>(WiFiUDPClass)->available(); }
extern "C" void C_stop(void* WiFiUDPClass){ return static_cast<WiFiUDP*>(WiFiUDPClass)->stop(); }
extern "C" int C_beginPacket_h( void* WiFiUDPClass, const char *host, uint16_t port){ return static_cast<WiFiUDP*>(WiFiUDPClass)->beginPacket(host, port); }
extern "C" int C_beginPacket_ip( void* WiFiUDPClass, uint32_t ip, uint16_t port){ return static_cast<WiFiUDP*>(WiFiUDPClass)->beginPacket(ip, port); }
extern "C" int C_endPacket(void* WiFiUDPClass){ return static_cast<WiFiUDP*>(WiFiUDPClass)->endPacket(); }
extern "C" size_t C_write( void* WiFiUDPClass, const uint8_t *buffer, size_t size){ return static_cast<WiFiUDP*>(WiFiUDPClass)->write(buffer, size);}
extern "C" int C_parsePacket(void* WiFiUDPClass){ return static_cast<WiFiUDP*>(WiFiUDPClass)->parsePacket(); }
extern "C" int C_read( void* WiFiUDPClass, unsigned char* buf, size_t size){ return static_cast<WiFiUDP*>(WiFiUDPClass)->read(buf, size); }
extern "C" uint32_t C_remoteIP(void* WiFiUDPClass){ return (uint32_t)static_cast<WiFiUDP*>(WiFiUDPClass)->remoteIP(); }
extern "C" uint16_t C_remotePort(void* WiFiUDPClass){ return static_cast<WiFiUDP*>(WiFiUDPClass)->remotePort(); }

/* Constructor. */
WiFiUDP::WiFiUDP()
{
	_socket = -1;
	_port = -1;
	_multiIp = 0;
	_rcvSize = 0;
	_rcvPort = 0;
	_rcvIP = 0;
	_sndSize = 0;
}

/* Start WiFiUDP socket, listening at local port PORT */
uint8_t WiFiUDP::begin(uint16_t port)
{
	struct sockaddr_in addr;
	uint32 u32EnableCallbacks = 0;

	_port = port;
	_rcvSize = 0;
	_rcvPort = 0;
	_rcvIP = 0;
	_sndSize = 0;

	// Initialize socket address structure.
	addr.sin_family = AF_INET;
	addr.sin_port = _htons(_port);
	addr.sin_addr.s_addr = 0;

	// Open UDP server socket.
	if ((_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return 0;
	}

	setsockopt(_socket, SOL_SOCKET, SO_SET_UDP_SEND_CALLBACK, &u32EnableCallbacks, 0);

	socketBufferSetupBuffer(_socket);

	// Bind socket:
	if (bind(_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
		socketBufferClose(_socket);
		close(_socket);
		_socket = -1;
		return 0;
	}

	if (socketBufferBindWait(_socket) == 0) {
		socketBufferClose(_socket);
		close(_socket);
		_socket = -1;
		return 0;
	}

	return 1;
}

uint8_t WiFiUDP::beginMulti(IPAddress ip, uint16_t port)
{
	_multiIp = ip;

	if (!begin(port)) {
		return 0;
	}

	setsockopt(_socket, SOL_SOCKET, IP_ADD_MEMBERSHIP, &_multiIp, sizeof(_multiIp));

	return 1;
}

/* return number of bytes available in the current packet,
   will return zero if parsePacket hasn't been called yet */
int WiFiUDP::available()
{
	m2m_wifi_handle_events(NULL);

	if (_socket != -1) {
		return _rcvSize;
	}
	return 0;
 }

/* Release any resources being used by this WiFiUDP instance */
void WiFiUDP::stop()
{
	if (_socket < 0)
		return;

	socketBufferClose(_socket);
	close(_socket);
	_socket = -1;
}

int WiFiUDP::beginPacket(const char *host, uint16_t port)
{
	IPAddress ip;
	if (WiFi.hostByName(host, ip)) {
		return beginPacket(ip, port);
	}

	return 0;
}

int WiFiUDP::beginPacket(IPAddress ip, uint16_t port)
{
	_sndIP = ip;
	_sndPort = port;
	_sndSize = 0;

	return 1;
}

int WiFiUDP::endPacket()
{
	struct sockaddr_in addr;

	// Network led ON (rev A then rev B).
	m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 0);
	m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = _htons(_sndPort);
	addr.sin_addr.s_addr = _sndIP;

	if (sendto(_socket, (void *)_sndBuffer, _sndSize, 0,
			(struct sockaddr *)&addr, sizeof(addr)) < 0) {
		// Network led OFF (rev A then rev B).
		m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 1);
		m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 1);
		return 0;
	}

	// Network led OFF (rev A then rev B).
	m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 1);
	m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 1);

	return 1;
}

size_t WiFiUDP::write(uint8_t byte)
{
  return write(&byte, 1);
}

size_t WiFiUDP::write(const uint8_t *buffer, size_t size)
{
	if ((size + _sndSize) > sizeof(_sndBuffer)) {
		size = sizeof(_sndBuffer) - _sndSize;
	}

	memcpy(_sndBuffer + _sndSize, buffer, size);

	_sndSize += size;

	return size;
}

int WiFiUDP::parsePacket()
{
	m2m_wifi_handle_events(NULL);

	if (_socket == -1 || !socketBufferIsBind(_socket)) {
		_socket = -1;
		if (_multiIp) {
			beginMulti(_multiIp, _port);
		} else {
			begin(_port);
		}
	}

	if (_socket != -1) {
		if (_rcvSize != 0) {
			return _rcvSize;
		}
		if (socketBufferDataAvailable(_socket)) {
			socketBufferReadUdpHeader(_socket, &_rcvSize, &_rcvPort, &_rcvIP);
			return _rcvSize;
		}
	}
	return 0;
}

int WiFiUDP::read()
{
	uint8_t b;

	if (read(&b, sizeof(b)) == -1) {
		return -1;
	}

	return b;
}

int WiFiUDP::read(unsigned char* buf, size_t size)
{
	int readSize = socketBufferRead(_socket, buf, size);

	if (readSize > 0) {
		_rcvSize -= readSize;
	}

	return readSize;
}

int WiFiUDP::peek()
{
	if (!available())
		return -1;

	return socketBufferPeek(_socket);
}

void WiFiUDP::flush()
{
	while (available())
		read();
}

IPAddress  WiFiUDP::remoteIP()
{
	return _rcvIP;
}

uint16_t  WiFiUDP::remotePort()
{
	return _rcvPort;
}
