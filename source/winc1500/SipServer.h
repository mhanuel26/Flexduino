#if defined(SIPSERVER)

#ifndef SIPSERVER_H
#define SIPSERVER_H

extern "C" {
	#include "socket/include/socket.h"
	#include "socket/include/socket_buffer.h"
}

#include "osip2/osip.h"
#include <WiFiUdp.h>


#define MESSAGE_MAX_LENGTH SOCKET_BUFFER_UDP_SIZE
#define MAX_ADDR_STR 128
#define MESSAGE_ENTRY_MAX_LENGTH 256
#define SIP_PORT 5060
#define EXPIRES_TIME_INSECS 3600

#define USER_ID "7080"
#define SIP_PROXY "sip:192.168.1.5"
#define SIP_FROM "sip:7080 at 192.168.1.80"
#define SIP_TO "sip:7080 at 192.168.1.80"
#define SIP_CONTACT "sip:7080 at 192.168.1.80"
#define LOCAL_IP "192.168.1.80"


class SipServer : public WiFiUDP {
private:
	osip_t* _osip;
	static WiFiUDP _sipudp;
	int _status;
	uint16_t _port;
	static int cb_sndMsg(osip_transaction_t *tr,osip_message_t *sip, char *host,int port, int out_socket);
	static void cb_rcvICTRes(int type, osip_transaction_t *pott,osip_message_t *pomt);
	static void cb_rcvNICTRes(int type, osip_transaction_t *pott,osip_message_t *pomt);
	static void cb_rcvreq(int type, osip_transaction_t *pott,osip_message_t *pomt);
	void setCallbacks(void);
	int AddSupportedMethods(osip_message_t *msgPtr);
	int bSipRegisterBuild(osip_message_t **regMsgPtrPtr);
	int bSipSend(osip_message_t  *msgPtr, osip_fsm_type_t   transactionType);
	void process(void);
public:
	SipServer();
	SipServer(uint16_t port);
	int bSipRegister(void);
	void begin(void);
	void poll(void);
};

#endif
#endif
