#if defined(SIPSERVER)

#include "fsl_debug_console.h"
#include "SipServer.h"
#include "Serial.h"
#include <stdarg.h>

extern SerialConsole Serial;

void TraceSipError2DbgCsl(char* fi, int li, osip_trace_level_t level, char *chfr, va_list ap)
{
	switch(level) {
	case OSIP_FATAL:
		Serial.print("| OSIP_FATAL |");
		break;
	case OSIP_BUG:
		Serial.print("| OSIP_BUG |");
		break;
	case OSIP_ERROR:
		Serial.print("| OSIP_ERROR |");
		break;
	case OSIP_WARNING:
		Serial.print("| OSIP_WARNING |");
		break;
	case OSIP_INFO1:
		Serial.print("| OSIP_INFO1 | ");
		break;
	case OSIP_INFO2:
		Serial.print("| OSIP_INFO2 |");
		break;
	case OSIP_INFO3:
		Serial.print("| OSIP_INFO3 |");
		break;
	case OSIP_INFO4:
		Serial.print("| OSIP_INFO4 |");
		break;
	default:
		break;
	}
	Serial.print("<");
	Serial.print(fi);
	Serial.print(":");
	Serial.print(li);
	Serial.print("> ");

	PRINTF_FORMATTED(chfr, ap);
	Serial.println();
}


SipServer::SipServer()
{
	SipServer(SIP_PORT);
}


SipServer::SipServer(uint16_t port)
{
	_status = 0;
	TRACE_INITIALIZE(OSIP_WARNING, NULL);
	osip_trace_initialize_func(OSIP_WARNING, &TraceSipError2DbgCsl);
	TRACE_ENABLE_LEVEL(OSIP_FATAL);
	TRACE_ENABLE_LEVEL(OSIP_BUG);
	TRACE_ENABLE_LEVEL(OSIP_ERROR);
	TRACE_ENABLE_LEVEL(OSIP_WARNING);
	TRACE_ENABLE_LEVEL(OSIP_INFO1);
	TRACE_ENABLE_LEVEL(OSIP_INFO2);
	TRACE_ENABLE_LEVEL(OSIP_INFO3);
	TRACE_ENABLE_LEVEL(OSIP_INFO4);
	// Init the Osip library
	int i;
	i = osip_init(&_osip);
	if (i!=0){
		Serial.println("Init osip server fail!");
		return;
	}
	setCallbacks();
	_port = port;
	_status = 1;
	Serial.println("call SipServer constructor");
}

void SipServer::begin(void){
	osip_trace(__FILE__,__LINE__,OSIP_WARNING,NULL,"Check OSIP_TRACE init");
	_sipudp.begin(_port);
}

int SipServer::cb_sndMsg(osip_transaction_t *tr,osip_message_t *sip, char *host,int port, int out_socket)
{
//	int len = 0;		// not used be we can return the number of chars sent
	char *msgP;
	size_t msgLen;
	int i;
//	int status;

	Serial.println("SendMsg");

	if((i = osip_message_to_str(sip, &msgP, &msgLen)) != 0){
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"failed to convert message/n"));
		return -1;
	}
	_sipudp.beginPacket((const char*)host, (uint16_t)port);
	_sipudp.write((const uint8_t*)msgP, msgLen);
	if(_sipudp.endPacket()){
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_INFO1,NULL,"Time: Udp message sent: /n%s/n",msgP));
	}
	return 0;
}

void SipServer::cb_rcvICTRes(int type, osip_transaction_t *pott,osip_message_t *pomt)
{
	Serial.println("cb_rcvICTRes");
}

void SipServer::cb_rcvNICTRes(int type, osip_transaction_t *pott,osip_message_t *pomt)
{
	Serial.println("cb_rcvNICTRes");
}

void SipServer::cb_rcvreq(int type, osip_transaction_t *pott,osip_message_t *pomt)
{
	Serial.println("cb_rcvreq");
}

void SipServer::setCallbacks(void){
	osip_set_cb_send_message(_osip, cb_sndMsg);
	osip_set_message_callback(_osip,OSIP_ICT_STATUS_1XX_RECEIVED,cb_rcvICTRes);
	osip_set_message_callback(_osip,OSIP_NICT_STATUS_1XX_RECEIVED,cb_rcvNICTRes);
	osip_set_message_callback(_osip,OSIP_IST_INVITE_RECEIVED,cb_rcvreq);
}

int SipServer::AddSupportedMethods(osip_message_t *msgPtr)
{
	osip_message_set_allow(msgPtr, "INVITE");
	osip_message_set_allow(msgPtr, "INFO");
	osip_message_set_allow(msgPtr, "ACK");
	osip_message_set_allow(msgPtr, "CANCEL");
	osip_message_set_allow(msgPtr, "BYE");

	return 0;
}

int SipServer::bSipSend(osip_message_t  *msgPtr, osip_fsm_type_t   transactionType)
{
	int status;
	osip_transaction_t *transactionPtr;
	osip_event_t       *sipeventPtr;

	if ( (status = osip_transaction_init(&transactionPtr,transactionType,_osip,msgPtr)) != 0 ){
		Serial.print("Failed to init transaction");
		Serial.println(status);
		return -1;
	}

	if((sipeventPtr = osip_new_outgoing_sipmessage(msgPtr)) == NULL){
		Serial.println("Can't allocate message");
		osip_message_free(msgPtr);
		return -1;
	}

	sipeventPtr->transactionid =  transactionPtr->transactionid;


	if((status = osip_message_force_update(msgPtr)) != 0){
		Serial.print("Failed force update :");
		Serial.println(status);
		osip_message_free(msgPtr);
		return -1;
	}

	if((status = osip_transaction_add_event(transactionPtr, sipeventPtr)) != 0){
		Serial.println("Can't add event");
		osip_message_free(msgPtr);
		return -1;
	}

	return 0;

}

int SipServer::bSipRegisterBuild(osip_message_t **regMsgPtrPtr)
{
	static int gSeqNum = 1;
	int status;
	char *callidNumberStr = NULL;
	char *seqNumStr = NULL;
	osip_call_id_t *callidPtr;
	char temp[MESSAGE_ENTRY_MAX_LENGTH];
	char sipPort[MESSAGE_ENTRY_MAX_LENGTH];
	osip_cseq_t *cseqPtr;
	unsigned int number;
	osip_message_t     *regMsgPtr;
	char expires[10];

	if((status = osip_message_init(&regMsgPtr)) != 0){
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"Can't init message!/n"));
		return -1;
	}
	osip_message_set_method(regMsgPtr, osip_strdup("REGISTER"));

	osip_uri_init(&(regMsgPtr->req_uri));
	if ( ( status = osip_uri_parse(regMsgPtr->req_uri, SIP_PROXY) ) != 0)
	{
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"uri parse failed!/n"));
		osip_message_free(regMsgPtr);
		return -1;
	}
	osip_message_set_version(regMsgPtr, osip_strdup("SIP/2.0"));
	osip_message_set_status_code(regMsgPtr, 0);
	osip_message_set_reason_phrase(regMsgPtr, NULL);

	osip_message_set_to(regMsgPtr, SIP_TO);
	osip_message_set_from(regMsgPtr, SIP_FROM);

	if((status = osip_call_id_init(&callidPtr)) != 0 ){
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"call id failed!/n"));
		osip_message_free(regMsgPtr);
		return -1;
	}
	callidNumberStr = (char *)osip_malloc(MAX_ADDR_STR);
	number = osip_build_random_number();
	sprintf(callidNumberStr,"%u",number);
	osip_call_id_set_number(callidPtr, callidNumberStr);

	osip_call_id_set_host(callidPtr, osip_strdup("10.1.1.63"));

	regMsgPtr->call_id = callidPtr;

	if((status = osip_cseq_init(&cseqPtr)) != 0 ){
		OSIP_TRACE(osip_trace(__FILE__,__LINE__,OSIP_BUG,NULL,"seq init failed!/n"));
		osip_message_free(regMsgPtr);
		return -1;
	}
	gSeqNum++;
	seqNumStr = (char *)osip_malloc(MAX_ADDR_STR);
	sprintf(seqNumStr,"%i", gSeqNum);
	osip_cseq_set_number(cseqPtr, seqNumStr);
	osip_cseq_set_method(cseqPtr, osip_strdup("REGISTER"));
	regMsgPtr->cseq = cseqPtr;

	osip_message_set_max_forwards(regMsgPtr, "70");

	sprintf(sipPort, "%i", SIP_PORT);
	sprintf(temp, "SIP/2.0/%s %s;branch=z9hG4bK%u", "UDP",LOCAL_IP,osip_build_random_number() );
	osip_message_set_via(regMsgPtr, temp);

	osip_message_set_contact(regMsgPtr, SIP_CONTACT);
	sprintf(expires, "%i", EXPIRES_TIME_INSECS);
	osip_message_set_expires(regMsgPtr, expires);

	osip_message_set_content_length(regMsgPtr, "0");

	osip_message_set_user_agent(regMsgPtr, "TotalView 1.0");

	AddSupportedMethods(regMsgPtr);
	*regMsgPtrPtr = regMsgPtr;
	return 0;
}

int SipServer::bSipRegister(void)
{
	osip_message_t *regMsgPtr;

	if(bSipRegisterBuild(&regMsgPtr) != 0){
		Serial.println("Error building register message!");
		return -1;
	}

	if (bSipSend(regMsgPtr,NICT) != 0){
		Serial.println("Error sending message!");
		return -1;
	}
	return 0;
}

void SipServer::poll(void){
	if (_sipudp.parsePacket()) {
		process();
	}
	osip_ict_execute(_osip);
	osip_ist_execute(_osip);
	osip_nict_execute(_osip);
	osip_nist_execute(_osip);
	osip_timers_ict_execute(_osip);
	osip_timers_ist_execute(_osip);
	osip_timers_nict_execute(_osip);
	osip_timers_nist_execute(_osip);
}

void SipServer::process(void)
{
	char msg[MESSAGE_MAX_LENGTH];
	int msgLen;
	osip_event_t *sipevent;
	int status;

	msgLen = _sipudp.available();
	if(msgLen > 0){
		_sipudp.read(msg, msgLen);
		Serial.println("processSipMsg: RECEIVED MSG");
		Serial.println(msg);
		sipevent = osip_parse(msg,msgLen);
		if((sipevent==NULL)||(sipevent->sip==NULL)){
			Serial.println("Could not parse SIP message");
			osip_event_free(sipevent);
			return;
		}
	}

	osip_message_fix_last_via_header(sipevent->sip,(char *)&_sipudp.remoteIP()[0],(int)_sipudp.remotePort());
	if((status = osip_find_transaction_and_add_event(_osip,sipevent)) != 0){
		Serial.println("New transaction!");
		if(MSG_IS_REQUEST(sipevent->sip)){
			PRINTF("Got New Request/n");
		}else if(MSG_IS_RESPONSE(sipevent->sip)){
			Serial.print("Bad Message:");
			Serial.println(msg);
			osip_event_free(sipevent);
		}else{
			Serial.print("Unsupported message:");
			Serial.println(msg);
			osip_event_free(sipevent);
		}
	}
}

WiFiUDP SipServer::_sipudp;

#endif
