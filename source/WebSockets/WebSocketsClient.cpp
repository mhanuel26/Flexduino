/**
 * @file WebSocketsClient.cpp
 * @date 20.05.2015
 * @author Markus Sattler
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the WebSockets for Arduino.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "WebSockets.h"
#include "WebSocketsClient.h"


WebSocketsClient::WebSocketsClient() {
    _cbEvent = NULL;
    _client.num = 0;
}

WebSocketsClient::~WebSocketsClient() {
    disconnect();
}


/**
 * calles to init the Websockets server
 */
void WebSocketsClient::begin(const char *host, uint16_t port, const char * url, const char * protocol) {
    _host = host;
    _port = port;
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
    _fingerprint = "";
#endif
    _client.num = 0;
    _client.status = WSC_NOT_CONNECTED;
    _client.tcp = NULL;
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
    _client.isSSL = false;
    _client.ssl = NULL;
#elif (WEBSOCKETS_NETWORK_TYPE == NETWORK_WINC1500)
    _client.isSSL = false;
    _client.ssl = NULL;
#endif
    _client.cUrl = url;
    _client.cCode = 0;
    _client.cIsUpgrade = false;
    _client.cIsWebsocket = true;
    _client.cKey = "";
    _client.cAccept = "";
    _client.cProtocol = protocol;
    _client.cExtensions = "";
    _client.cVersion = 0;
    _client.base64Authorization = "";
    _client.plainAuthorization = "";

#ifdef ESP8266
    randomSeed(RANDOM_REG32);
#else
    // todo find better seed
    randomSeed(millis());
#endif
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266_ASYNC)
    asyncConnect();
#endif
}

void WebSocketsClient::begin(String host, uint16_t port, String url, String protocol) {
    begin(host.c_str(), port, url.c_str(), protocol.c_str());
}

//void WebSocketsClient::begin(String host, uint16_t port, String url) {
//    begin(host.c_str(), port, url.c_str());
//}

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
void WebSocketsClient::beginSSL(const char *host, uint16_t port, const char * url, const char * fingerprint) {
    begin(host, port, url);
    _client.isSSL = true;
    _fingerprint = fingerprint;
}

void WebSocketsClient::beginSSL(String host, uint16_t port, String url, String fingerprint) {
    beginSSL(host.c_str(), port, url.c_str(), fingerprint.c_str());
}
#elif (WEBSOCKETS_NETWORK_TYPE == NETWORK_WINC1500)
void WebSocketsClient::beginSSL(const char *host, uint16_t port, const char * url, const char * protocol) {
    begin(host, port, url, protocol);
    _client.isSSL = true;
}
void WebSocketsClient::beginSSL(String host, uint16_t port, String url, String protocol) {
    beginSSL(host.c_str(), port, url.c_str(), protocol.c_str());
}
#endif

void WebSocketsClient::beginSocketIO(const char *host, uint16_t port, const char * url, const char * protocol) {
    begin(host, port, url, protocol);
    _client.isSocketIO = true;
}

void WebSocketsClient::beginSocketIO(String host, uint16_t port, String url, String protocol) {
    beginSocketIO(host.c_str(), port, url.c_str(), protocol.c_str());
}


/**
 * called in arduino loop
 */
void WebSocketsClient::loop(void) {
    if(!clientIsConnected(&_client)) {

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
        if(_client.isSSL) {
            DEBUG_WEBSOCKETS("[WS-Client] connect wss...\r\n");
            if(_client.ssl) {
                delete _client.ssl;
                _client.ssl = NULL;
                _client.tcp = NULL;
            }
            _client.ssl = new WiFiClientSecure();
            _client.tcp = _client.ssl;
        } else {
            DEBUG_WEBSOCKETS("[WS-Client] connect ws...\r\n");
            if(_client.tcp) {
                delete _client.tcp;
                _client.tcp = NULL;
            }
            _client.tcp = new WiFiClient();
        }
#elif (WEBSOCKETS_NETWORK_TYPE == NETWORK_WINC1500)
        if(_client.isSSL) {
            DEBUG_WEBSOCKETS("[WS-Client] connect wss...\r\n");
            if(_client.ssl) {
                delete _client.ssl;
                _client.ssl = NULL;
                _client.tcp = NULL;
            }
            _client.ssl = new WiFiSSLClient();
            _client.tcp = _client.ssl;
        } else {
            DEBUG_WEBSOCKETS("[WS-Client] connect ws...\r\n");
            if(_client.tcp) {
                delete _client.tcp;
                _client.tcp = NULL;
            }
            _client.tcp = new WEBSOCKETS_NETWORK_CLIENT_CLASS();
        }
#else
        _client.tcp = new WEBSOCKETS_NETWORK_CLIENT_CLASS();
#endif

        if(!_client.tcp) {
            DEBUG_WEBSOCKETS("[WS-Client] creating Network class failed!\r\n");
            return;
        }
        uint8_t cstat;
#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_WINC1500)

        if(_client.isSSL){
        	cstat = _client.tcp->connectSSL(_host.c_str(), _port);
        }else{
        	cstat = _client.tcp->connect(_host.c_str(), _port);
        }
#else
        cstat = _client.tcp->connect(_host.c_str(), _port)
#endif
        if(cstat) {
            DEBUG_WEBSOCKETS("[WS-Client] connected to %s:%u.\r\n", _host.c_str(), _port);

            _client.status = WSC_HEADER;

            // set Timeout for readBytesUntil and readStringUntil
            _client.tcp->setTimeout(WEBSOCKETS_TCP_TIMEOUT);

#if (WEBSOCKETS_NETWORK_TYPE == NETWORK_ESP8266)
            _client.tcp->setNoDelay(true);

            if(_client.isSSL && _fingerprint.length()) {
                if(!_client.ssl->verify(_fingerprint.c_str(), _host.c_str())) {
                    DEBUG_WEBSOCKETS("[WS-Client] certificate mismatch\r\n");
                    WebSockets::clientDisconnect(&_client, 1000);
                    return;
                }
            }
#endif

            // send Header to Server
            sendHeader(&_client);

        } else {
            DEBUG_WEBSOCKETS("[WS-Client] connection to %s:%u Failed\r\n", _host.c_str(), _port);
            delay(10); //some litle delay to not flood the server
        }
    } else {
        handleClientData();
    }
}

/**
 * set callback function
 * @param cbEvent WebSocketServerEvent
 */
void WebSocketsClient::onEvent(WebSocketClientEvent cbEvent) {
    _cbEvent = cbEvent;
}

/**
 * send text data to client
 * @param num uint8_t client id
 * @param payload uint8_t *
 * @param length size_t
 * @param headerToPayload bool  (see sendFrame for more details)
 */
void WebSocketsClient::sendTXT(uint8_t * payload, size_t length, bool headerToPayload) {
    if(length == 0) {
        length = strlen((const char *) payload);
    }
    if(clientIsConnected(&_client)) {
        sendFrame(&_client, WSop_text, payload, length, true, true, headerToPayload);
    }
}

void WebSocketsClient::sendTXT(const uint8_t * payload, size_t length) {
    sendTXT((uint8_t *) payload, length);
}

void WebSocketsClient::sendTXT(char * payload, size_t length, bool headerToPayload) {
    sendTXT((uint8_t *) payload, length, headerToPayload);
}

void WebSocketsClient::sendTXT(const char * payload, size_t length) {
    sendTXT((uint8_t *) payload, length);
}

void WebSocketsClient::sendTXT(String payload) {
    sendTXT((uint8_t *) payload.c_str(), payload.length());
}

/**
 * send binary data to client
 * @param num uint8_t client id
 * @param payload uint8_t *
 * @param length size_t
 * @param headerToPayload bool  (see sendFrame for more details)
 */
void WebSocketsClient::sendBIN(uint8_t * payload, size_t length, bool headerToPayload) {
    if(clientIsConnected(&_client)) {
        sendFrame(&_client, WSop_binary, payload, length, true, true, headerToPayload);
    }
}

void WebSocketsClient::sendBIN(const uint8_t * payload, size_t length) {
    sendBIN((uint8_t *) payload, length);
}

/**
 * disconnect one client
 * @param num uint8_t client id
 */
void WebSocketsClient::disconnect(void) {
    if(clientIsConnected(&_client)) {
        WebSockets::clientDisconnect(&_client, 1000);
    }
}

/**
 * set the Authorizatio for the http request
 * @param user const char *
 * @param password const char *
 */
void WebSocketsClient::setAuthorization(const char * user, const char * password) {
    if(user && password) {
        String auth = user;
        auth += ":";
        auth += password;
        _client.base64Authorization = base64_encode((uint8_t *)auth.c_str(), auth.length());
    }
}

/**
 * set the Authorizatio for the http request
 * @param auth const char * base64
 */
void WebSocketsClient::setAuthorization(const char * auth) {
    if(auth) {
        //_client.base64Authorization = auth;
        _client.plainAuthorization = auth;
    }
}

//#################################################################################
//#################################################################################
//#################################################################################

/**
 *
 * @param client WSclient_t *  ptr to the client struct
 * @param opcode WSopcode_t
 * @param payload  uint8_t *
 * @param lenght size_t
 */
void WebSocketsClient::messageReceived(WSclient_t * client, WSopcode_t opcode, uint8_t * payload, size_t lenght) {
    WStype_t type = WStype_ERROR;

    switch(opcode) {
        case WSop_text:
            type = WStype_TEXT;
            break;
        case WSop_binary:
            type = WStype_BIN;
            break;
        default:
        	break;
    }

    runCbEvent(type, payload, lenght);

}

/**
 * Disconnect an client
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsClient::clientDisconnect(WSclient_t * client) {

    if(client->tcp) {
        if(client->tcp->connected()) {
            client->tcp->flush();
            client->tcp->stop();
        }
        client->tcp = NULL;
    }

    client->cCode = 0;
    client->cKey = "";
    client->cAccept = "";
    client->cProtocol = "";
    client->cVersion = 0;
    client->cIsUpgrade = false;
    client->cIsWebsocket = false;

    client->status = WSC_NOT_CONNECTED;

    DEBUG_WEBSOCKETS("[WS-Client] client disconnected.\r\n");

    runCbEvent(WStype_DISCONNECTED, NULL, 0);

}

/**
 * get client state
 * @param client WSclient_t *  ptr to the client struct
 * @return true = conneted
 */
bool WebSocketsClient::clientIsConnected(WSclient_t * client) {

    if(!client->tcp) {
        return false;
    }

    if(client->tcp->connected()) {
        if(client->status != WSC_NOT_CONNECTED) {
            return true;
        }
    } else {
        // client lost
        if(client->status != WSC_NOT_CONNECTED) {
            DEBUG_WEBSOCKETS("[WS-Client] connection lost.\r\n");
            // do cleanup
            clientDisconnect(client);
        }
    }

    if(client->tcp) {
        // do cleanup
        clientDisconnect(client);
    }

    return false;
}

/**
 * Handel incomming data from Client
 */
void WebSocketsClient::handleClientData(void) {
    int len = _client.tcp->available();
    if(len > 0) {
        switch(_client.status) {
            case WSC_HEADER:
                handleHeader(&_client);
                break;
            case WSC_CONNECTED:
                WebSockets::handleWebsocket(&_client);
                break;
            default:
                WebSockets::clientDisconnect(&_client, 1002);
                break;
        }
    }
#ifdef ESP8266
    delay(0);
#endif
}

/**
 * send the WebSocket header to Server
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsClient::sendHeader(WSclient_t * client) {

    DEBUG_WEBSOCKETS("[WS-Client][sendHeader] sending header...\n");

    uint8_t randomKey[16] = { 0 };

    for(uint8_t i = 0; i < sizeof(randomKey); i++) {
        randomKey[i] = random(0xFF);
    }


    client->cKey = base64_encode(&randomKey[0], 16);

#ifndef NODEBUG_WEBSOCKETS
    unsigned long start = micros();
#endif

    String transport;
    String handshake;
    if(!client->isSocketIO || (client->isSocketIO && client->cSessionId.length() > 0)) {
        if(client->isSocketIO) {
            transport = "&transport=websocket&sid=" + client->cSessionId;
        }
        //                    "Host: " + _host + ":" + _port + "\r\n"
//        wss://" +  _host +
        handshake = "GET " +  client->cUrl + transport + " HTTP/1.1\r\n"
        			"Host: " + _host + "\r\n"
					"Connection: Upgrade\r\n"
					"Pragma: no-cache\r\n"
					"Cache-Control: no-cache\r\n"
					"Upgrade: websocket\r\n"
					"Origin: Flexduino\r\n"
//					"Cookie: artik_cookiesOVerlay=true\r\n"
                    "User-Agent: arduino-WebSocket-Client\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "Sec-WebSocket-Key: " + client->cKey + "\r\n";

        if(client->cProtocol.length() > 0) {
           handshake += "Sec-WebSocket-Protocol: " + client->cProtocol + "\r\n";
        }

        if(client->cExtensions.length() > 0) {
            handshake += "Sec-WebSocket-Extensions: " + client->cExtensions + "\r\n";
        }

    } else {
        handshake = "GET " + client->cUrl + "&transport=polling HTTP/1.1\r\n"
                    "Connection: keep-alive\r\n";
    }

//    handshake +=    "Host: " + _host + ":" + _port + "\r\n"
//                    "Origin: file://\r\n"
//                    "User-Agent: arduino-WebSocket-Client\r\n";

    if(client->base64Authorization.length() > 0) {
        handshake += "Authorization: Basic " + client->base64Authorization + "\r\n";
    }

    if(client->plainAuthorization.length() > 0) {
        handshake += "Authorization: " + client->plainAuthorization + "\r\n";
    }

    handshake += "\r\n";

    client->tcp->write(handshake.c_str(), handshake.length());

#ifndef NODEBUG_WEBSOCKETS
    DEBUG_WEBSOCKETS("[WS-Client][sendHeader] sending header... Done (%uus).\r\n", (micros() - start));
#else
    DEBUG_WEBSOCKETS("[WS-Client][sendHeader] sending header... Done \r\n");
    DEBUG_WEBSOCKETS(handshake.c_str());
    DEBUG_WEBSOCKETS("\r\n**********************************\r\n");
#endif
}

/**
 * handle the WebSocket header reading
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsClient::handleHeader(WSclient_t * client) {

    String headerLine = client->tcp->readStringUntil('\n');
    headerLine.trim(); // remove \r

    if(headerLine.length() > 0) {
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader] RX: %s\r\n", headerLine.c_str());

        if(headerLine.startsWith("HTTP/1.")) {
            // "HTTP/1.1 101 Switching Protocols"
            client->cCode = headerLine.substring(9, headerLine.indexOf(' ', 9)).toInt();
        } else if(headerLine.indexOf(':')) {
            String headerName = headerLine.substring(0, headerLine.indexOf(':'));
            String headerValue = headerLine.substring(headerLine.indexOf(':') + 2);

            if(headerName.equalsIgnoreCase("Connection")) {
                if(headerValue.indexOf("Upgrade") >= 0) {
                    client->cIsUpgrade = true;
                }
            } else if(headerName.equalsIgnoreCase("Upgrade")) {
                if(headerValue.equalsIgnoreCase("websocket")) {
                    client->cIsWebsocket = true;
                }
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Accept")) {
                client->cAccept = headerValue;
                client->cAccept.trim(); // see rfc6455
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Protocol")) {
                client->cProtocol = headerValue;
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Extensions")) {
                client->cExtensions = headerValue;
            } else if(headerName.equalsIgnoreCase("Sec-WebSocket-Version")) {
                client->cVersion = headerValue.toInt();
            } else if(headerName.equalsIgnoreCase("Set-Cookie")) {
                client->cSessionId = headerValue.substring(headerValue.indexOf('=') + 1);
            }
        } else {
            DEBUG_WEBSOCKETS("[WS-Client][handleHeader] Header error (%s)\r\n", headerLine.c_str());
        }

    } else {
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader] Header read fin.\r\n");
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader] Client settings:\r\n");

        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cURL: %s\r\n", client->cUrl.c_str());
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cKey: %s\r\n", client->cKey.c_str());

        DEBUG_WEBSOCKETS("[WS-Client][handleHeader] Server header:\r\n");
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cCode: %d\r\n", client->cCode);
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cIsUpgrade: %d\r\n", client->cIsUpgrade);
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cIsWebsocket: %d\r\n", client->cIsWebsocket);
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cAccept: %s\r\n", client->cAccept.c_str());
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cProtocol: %s\r\n", client->cProtocol.c_str());
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cExtensions: %s\r\n", client->cExtensions.c_str());
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cVersion: %d\r\n", client->cVersion);
        DEBUG_WEBSOCKETS("[WS-Client][handleHeader]  - cSessionId: %s\n", client->cSessionId.c_str());

        bool ok = (client->cIsUpgrade && client->cIsWebsocket);

        if(ok) {
            switch(client->cCode) {
                case 101:  ///< Switching Protocols

                    break;
                case 200:
                    if(client->isSocketIO) {
                        break;
                    }
                case 403: ///< Forbidden
                    // todo handle login
                default:   ///< Server dont unterstand requrst
                    ok = false;
                    DEBUG_WEBSOCKETS("[WS-Client][handleHeader] serverCode is not 101 (%d)\r\n", client->cCode);
                    clientDisconnect(client);
                    break;
            }
        }

        if(ok) {
            if(client->cAccept.length() == 0) {
                ok = false;
            } else {
                // generate Sec-WebSocket-Accept key for check
                String sKey = acceptKey(client->cKey);
                if(sKey != client->cAccept) {
                    DEBUG_WEBSOCKETS("[WS-Client][handleHeader] Sec-WebSocket-Accept is wrong\r\n");
                    ok = false;
                }
            }
        }

        if(ok) {

            DEBUG_WEBSOCKETS("[WS-Client][handleHeader] Websocket connection init done.\r\n");
            headerDone(client);

            runCbEvent(WStype_CONNECTED, (uint8_t *) client->cUrl.c_str(), client->cUrl.length());
        } else if(clientIsConnected(client) && client->isSocketIO && client->cSessionId.length() > 0) {
            sendHeader(client);
        } else {
            DEBUG_WEBSOCKETS("[WS-Client][handleHeader] no Websocket connection close.\r\n");
            client->tcp->write("This is a webSocket client!");
            clientDisconnect(client);
        }
    }
}

