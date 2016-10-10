// -*- c++ -*-
//
// Copyright 2010 Ovidiu Predescu <ovidiu@gmail.com>
// Date: May 2010

// Modified by Manuel Iglesias <mhanuel.usb@gmail.com>
// Modified version of Ovidiu Library for NXP K82F/WINC1500 Platform
// Date: June 2016

// Date: October 2013
// Updated: 08-JAN-2012 for Arduno IDE 1.0 by <Hardcore@hardcoreforensics.com>
// Updated: 29-MAR-2013 replacing strtoul with parseHexChar by <shin@marcsi.ch>
//
// TinyWebServer for NXP/K82F Arduino Compatible Platform :)
//
// The DEBUG flag will enable serial console logging in this library
// By default Debugging to the Serial console is OFF.
// This ensures that any scripts using the Serial port are not corrupted
// by the tinywebserver libraries debugging messages.
//
// To ENABLE debugging set the following:
// DEBUG 1 and ENSURE that you have configured the serial Debug Console
//
// There is an overall size increase of about 340 bytes in code size
// when the debugging is enabled and debugging lines are preceded by 'TWS:'

#define DEBUG 1

// 10 milliseconds read timeout
#define READ_TIMEOUT 10

#include "Arduino.h"			// it's calling K82F include

extern "C" {

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
}

#include <WiFi101.h>
#include "Serial.h"
#include "Flash.h"
#include <FatFs.h>
#include "fsl_pit.h"
#include "TinyWebServer.h"

// Temporary buffer.
#define BUFFER_SIZE		512
static __attribute__((aligned(32), section(".myRAM"))) char buffer[WEB_SERVER_CLIENT_MAX][BUFFER_SIZE];			// read in 512 bytes so we speed up SD card access
static __attribute__((aligned(32), section(".myRAM"))) char obuffer[BUFFER_SIZE];
static __attribute__((aligned(32), section(".myRAM"))) char linebuf[BUFFER_SIZE];

extern SerialConsole Serial;

FLASH_STRING(mime_types,
	"HTM*text/html|"
	"TXT*text/plain|"
	"CSS*text/css|"
	"XML*text/xml|"
	"JS*text/javascript|"
	"GIF*image/gif|"
	"JPG*image/jpeg|"
	"PNG*image/png|"
	"ICO*image/vnd.microsoft.icon|"
	"MP3*audio/mpeg|"
	"CSV*application/octet-stream|"
);

void *malloc_check(size_t size) {
  void* r = malloc(size);
#if DEBUG
  if (!r) {
    Serial << F("TWS:No space for malloc: " ); Serial.println(size, DEC);
  }
#endif
  return r;
}

// Offset for text/html in `mime_types' above.
static const TinyWebServer::MimeType text_html_content_type = 4;

TinyWebServer::TinyWebServer(PathHandler handlers[],
			     const char** headers,
                             const int port)
  : handlers_(handlers),
    server_(WiFiServer(port)),
    path_(NULL),
    request_type_(UNKNOWN_REQUEST),
	client_ptr_(NULL) {
    //client_(WiFiClient(255)) {
  if (headers) {
    int size = 0;
    for (int i = 0; headers[i]; i++) {
      size++;
    }
    headers_ = (HeaderValue*)malloc_check(sizeof(HeaderValue) * (size + 1));
    if (headers_) {
      for (int i = 0; i < size; i++) {
        headers_[i].header = headers[i];
        headers_[i].value = NULL;
      }
      headers_[size].header = NULL;
    }
  }
}

void TinyWebServer::begin() {
  server_.begin();
}

// Process headers.
boolean TinyWebServer::process_headers() {
  if (headers_) {
    // First clear the header values from the previous HTTP request.
    for (int i = 0; headers_[i].header; i++) {
      if (headers_[i].value) {
        free(headers_[i].value);
        // Ensure the pointer is cleared once the memory is freed.
        headers_[i].value = NULL;
      }
    }
  }

  enum State {
    ERROR,
    START_LINE,
    HEADER_NAME,
    HEADER_VALUE,
    HEADER_VALUE_SKIP_INITIAL_SPACES,
    HEADER_IGNORE_VALUE,
    END_HEADERS,
  };
  State state = START_LINE;

  char ch;
  unsigned int pos;
  const char* header;
  uint32_t start_time = millis();
  while (1) {
    if (should_stop_processing()) {
      return false;
    }
    if (millis() - start_time > READ_TIMEOUT) {
      return false;
    }
    if (!read_next_char_from_buf((uint8_t*)&ch)) {
      continue;
    }
    start_time = millis();
#if DEBUG
    Serial.print(ch);
#endif
    switch (state) {
    case START_LINE:
      if (ch == '\r') {
	break;
      } else if (ch == '\n') {
	state = END_HEADERS;
      } else if (isalnum(ch) || ch == '-') {
	pos = 0;
	linebuf[pos++] = ch;
	state = HEADER_NAME;
      } else {
	state = ERROR;
      }
      break;

    case HEADER_NAME:
      if (pos + 1 >= sizeof(linebuf)) {
	state = ERROR;
	break;
      }
      if (ch == ':') {
	linebuf[pos] = 0;
	header = linebuf;
	if (is_requested_header(&header)) {
	  state = HEADER_VALUE_SKIP_INITIAL_SPACES;
	} else {
	  state = HEADER_IGNORE_VALUE;
	}
	pos = 0;
      } else if (isalnum(ch) || ch == '-') {
	linebuf[pos++] = ch;
      } else {
	state = ERROR;
	break;
      }
      break;

    case HEADER_VALUE_SKIP_INITIAL_SPACES:
      if (pos + 1 >= sizeof(linebuf)) {
	state = ERROR;
	break;
      }
      if (ch != ' ') {
	linebuf[pos++] = ch;
	state = HEADER_VALUE;
      }
      break;

    case HEADER_VALUE:
      if (pos + 1 >= sizeof(linebuf)) {
	state = ERROR;
	break;
      }
      if (ch == '\n') {
	linebuf[pos] = 0;
	if (!assign_header_value(header, linebuf)) {
	  state = ERROR;
	  break;
	}
	state = START_LINE;
      } else {
	if (ch != '\r') {
	  linebuf[pos++] = ch;
	}
      }
      break;

    case HEADER_IGNORE_VALUE:
      if (ch == '\n') {
	state = START_LINE;
      }
      break;

    default:
      break;
    }

    if (state == END_HEADERS) {
      break;
    }
    if (state == ERROR) {
      return false;
    }
  }
  return true;
}

boolean TinyWebServer::read_next_char_from_buf(uint8_t* ch) {
	uint8_t sock;
	char *lptr;

	sock = client_ptr_->getSockNum();
	lptr = line_ptr_[sock];

	if (!client_available_[sock]) {		// not available in buffer
		return false;
	} else {
		if(*lptr != '\0'){
			*ch = *lptr++;
			line_ptr_[sock] = lptr;
			return true;
		}
	}
	return false;
}

boolean TinyWebServer::get_line_from_int_buf(char* obuffer, uint8_t sock) {
	char *lptr = line_ptr_[sock];
	uint16_t maxsize;
	int i = 0;

	maxsize = lptr - &buffer[sock][0];
	maxsize = BUFFER_SIZE - maxsize;
	obuffer[0] = 0;

	for (i=0; i < maxsize; i++) {
		if (*lptr == '\n') {
			lptr++;
			line_ptr_[sock] = lptr;		// save the current pointer
			break;
		}
		obuffer[i] = *lptr++;
	}
	obuffer[i] = 0;
	return i < maxsize;

}

bool TinyWebServer::get_line_from_internal(uint8_t sock){

	memset(linebuf, 0, sizeof(linebuf));
	boolean is_complete = get_line_from_int_buf(linebuf, sock);
	if (!linebuf[0]) {
		client_available_[sock] = false;
		buffer_ = NULL;
		return false;						// we have a new client but it has no more lines available on internal buffer
	}
#if DEBUG
	Serial << F("TWS:New request: ");
	Serial.println(linebuf);
#endif
	if (!is_complete) {
		// The requested path is too long.
		send_error_code(414);
		clientDisconnect();
		return false;
	}

	client_available_[sock] = true;				// this should be already true...
	return true;
}

bool TinyWebServer::get_all_available(WiFiClient* client, uint8_t sock){
	uint16_t length;

	if (!client->connected()){
		clientDisconnect();
		return false;
	}
	buffer_ = &buffer[sock][0];
	memset(buffer_, 0, BUFFER_SIZE);			// this way we ensure correct line reading
	length = client->available();
	if (length > 0) {
		client_available_[sock] = true;			// this client has something internally available
		line_ptr_[sock] = &buffer[sock][0];		// point to the beginning of array
		client->read((uint8_t*)buffer_, length);			// we are checking the received size...
		return true;
	} else {
		client_available_[sock] = false;		// just in f..case
		return true;						// there is a client connected but nothing is available
	}
}

bool TinyWebServer::newClient(WiFiClient* client){

	uint8_t sock;
	if (!client->connected()){
		client->stop();			// client disconnects but was not added to our list of connected clients
		delete client;
		return false;
	}
	sock = client->getSockNum();
	if(clients_[sock] != NULL){
		client->stop();
		delete client;
		return false;
	}else{
		PRINTF("*** added client with sock %d\r\n", sock);
		clients_[sock] = client;
		client_ptr_ = client;
		if(!client->available()){
			client_available_[sock] = false;
			return true;				// we actually might have a new client that has not send anything....
		}
		return get_all_available(client_ptr_, sock);
	}
	return true;
}

bool TinyWebServer::clientDisconnect(){
	uint8_t sock;

	sock = client_ptr_->getSockNum();
	PRINTF("*** disconnect client %d\r\n", sock);
	client_ptr_->stop();
	delete client_ptr_;
	client_ptr_ = (WiFiClient*)NULL;
	clients_[sock] = (WiFiClient*)NULL;
	client_available_[sock] = false;
	buffer_ = NULL;
	return true;
}

bool TinyWebServer::handleNewClient(){
	WiFiClient *tmp;
	tmp = server_.mavailable();		// I was using a modified version of available but after updating win1500 arduino library seems I lost it....server_.mavailable();
	if(tmp != (WiFiClient*)NULL){	// we have a new client
		PRINTF("*** new client with socket# %d wants to connect\r\n", tmp->getSockNum());
		return newClient(tmp);
	}
	return false;			// false means no client
}

void TinyWebServer::process() {
	uint8_t i = 0;
	uint8_t sock;

	if(handleNewClient()){	// we have a new client
		return;		// attend the old connections first
	}else{ // no new connection case
		for(i = 0; i < WEB_SERVER_CLIENT_MAX; i++) {
			client_ptr_ = clients_[i];
			if(client_ptr_ != (WiFiClient*)NULL){
				if (!client_ptr_->connected()){
					clientDisconnect();					// should be at least connected
					continue;							// we test other clients before returning
				}
				sock = client_ptr_->getSockNum();
				if(!client_available_[sock]){
					if(client_ptr_->available())		// if actually is something available, get them all...
						get_all_available(client_ptr_, sock);
					continue;					// client has nothing available and nothing in internally buffer
				}else{
					buffer_ = &buffer[sock][0];
					break;
				}
			}else{
				continue;
			}
		}
	}
	if((i >= WEB_SERVER_CLIENT_MAX) || client_ptr_ == NULL)
		return;
	// at this pint we are working with client_ptr_ and buffer_ pointers pointing to given client in list
	if(!get_line_from_internal(sock)){			// if false, there are no more internal lines
		return;
	}

	char* request_type_str = get_field(linebuf, 0);
	request_type_ = UNKNOWN_REQUEST;
	if (!strcmp("GET", request_type_str)) {
		request_type_ = GET;
	} else if (!strcmp("POST", request_type_str)) {
		request_type_ = POST;
	} else if (!strcmp("PUT", request_type_str)) {
		request_type_ = PUT;
	} else if (!strcmp("DELETE", request_type_str)) {
		request_type_ = DELETE;
	}
  
  path_ = get_field(linebuf, 1);

  // Process the headers.
  if (!process_headers()) {
    // Malformed header line.
    send_error_code(417);
    clientDisconnect();
//    client_.stop();
  }
  // Header processing finished. Identify the handler to call.

  boolean should_close = true;
  boolean found = false;
  for (int i = 0; handlers_[i].path; i++) {
    int len = strlen(handlers_[i].path);
    boolean exact_match = !strcmp(path_, handlers_[i].path);
    boolean regex_match = false;
    if (handlers_[i].path[len - 1] == '*') {
      regex_match = !strncmp(path_, handlers_[i].path, len - 1);
    }
    if ((exact_match || regex_match)
	&& (handlers_[i].type == ANY || handlers_[i].type == request_type_)) {
      found = true;
      should_close = (handlers_[i].handler)(*this);
      break;
    }
  }

  if (!found) {
    send_error_code(404);
    // (*this) << F("URL not found: ");
    // client_->print(path_);
    // client_->println();
  }
  if (should_close) {
	  clientDisconnect();
//    client_.stop();
  }

  free(path_);
  free(request_type_str);
}

boolean TinyWebServer::is_requested_header(const char** header) {
  if (!headers_) {
    return false;
  }
  for (int i = 0; headers_[i].header; i++) {
    if (!strcmp(*header, headers_[i].header)) {
      *header = headers_[i].header;
      return true;
    }
  }
  return false;
}

boolean TinyWebServer::assign_header_value(const char* header, char* value) {
  if (!headers_) {
    return false;
  }
  boolean found = false;
  for (int i = 0; headers_[i].header; i++) {
    // Use pointer equality, since `header' must be the pointer
    // inside headers_.
    if (header == headers_[i].header) {
      headers_[i].value = (char*)malloc_check(strlen(value) + 1);
      if (!headers_[i].value) {
	return false;
      }
      strcpy(headers_[i].value, value);
      found = true;
      break;
    }
  }
  return found;
}

FLASH_STRING(content_type_msg, "Content-Type: ");

void TinyWebServer::send_error_code(Client *client, int code) {
#if DEBUG
  Serial << F("TWS:Returning ");
  Serial.println(code, DEC);
#endif
  (*client) << F("HTTP/1.1 ");
  client->print(code, DEC);
  (*client) << F(" OK\r\n");
  if (code != 200) {
    end_headers(client);
  }
}

void TinyWebServer::send_content_type(MimeType mime_type) {
  (*client_ptr_) << content_type_msg;

  char ch;
  int i = mime_type;
  while ((ch = mime_types[i++]) != '|') {
    client_ptr_->print(ch);
  }

  client_ptr_->println();
}

void TinyWebServer::send_content_type(const char* content_type) {
  (*client_ptr_) << content_type_msg;
  client_ptr_->println(content_type);
}

const char* TinyWebServer::get_path() { return path_; }

const TinyWebServer::HttpRequestType TinyWebServer::get_type() {
  return request_type_;
}

const char* TinyWebServer::get_header_value(const char* name) {
  if (!headers_) {
    return NULL;
  }
  for (int i = 0; headers_[i].header; i++) {
    if (!strcmp(headers_[i].header, name)) {
      return headers_[i].value;
    }
  }
  return NULL;
}

int parseHexChar(char ch) {
  if (isdigit(ch)) {
    return ch - '0';
  }
  ch = tolower(ch);
  if (ch >= 'a' &&  ch <= 'e') {
    return ch - 'a' + 10;
  }
  return 0;
}

char* TinyWebServer::decode_url_encoded(const char* s) {
  if (!s) {
    return NULL;
  }
  char* r = (char*)malloc_check(strlen(s) + 1);
  if (!r){
    return NULL;
  }
  memset(r, 0, strlen(s) + 1);
  char* r2 = r;
  const char* p = s;
  while (*s && (p = strchr(s, '%'))) {
    if (p - s) {
      memcpy(r2, s, p - s);
      r2 += (p - s);
    }
    // If the remaining number of characters is less than 3, we cannot
    // have a complete escape sequence. Break early.
    if (strlen(p) < 3) {
      // Move the new beginning to the value of p.
      s = p;
      break;
    }
    uint8_t r = parseHexChar(*(p + 1)) << 4 | parseHexChar(*(p + 2));
    *r2++ = r;
    p += 3;

    // Move the new beginning to the value of p.
    s = p;
  }
  // Copy whatever is left of the string in the result.
  int len = strlen(s);
  if (len > 0) {
    strncpy(r2, s, len);
  }
  // Append the 0 terminator.
  *(r2 + len) = 0;

  return r;
}

char* TinyWebServer::get_file_from_path(const char* path) {
  // Obtain the last path component.
  const char* encoded_fname = strrchr(path, '/');
  if (!encoded_fname) {
    return NULL;
  } else {
    // Skip past the '/'.
    encoded_fname++;
  }
  char* decoded = decode_url_encoded(encoded_fname);
  if (!decoded) {
    return NULL;
  }
  // TODO: add compatibility with arduino, now I am using FatFs from chaN
  if (!_USE_LFN){
	  for (char* p = decoded; *p; p++) {
		*p = toupper(*p);
	  }
  }
  return decoded;
}


char *TinyWebServer::get_var_from_uri(const char* path, const char* var) {

	int len;
	const char *p = strstr(path, var);
	if(p != NULL){
		const char* r = strchr(p, '=');
		r++;
		const char* r2 = strchr(r, '&');
		if(r2 == NULL)
			len = strlen(r);
		else
			len = (r2 - r);
		char* value = (char*)malloc_check(len + 1);
		if (!value){
			return NULL;
		}
		strncpy(value, r, len);
		*(value+len)=0;
		char* decoded = decode_url_encoded(value);
		// Serial.println("get_var_from_uri method value before decode");
		// Serial.println(value);
		// Serial.println("get_var_from_uri method value after decode");
		// Serial.println(decoded);
		free(value);
		return decoded;
	}else{
		return NULL;
	}
}

TinyWebServer::MimeType TinyWebServer::get_mime_type_from_filename(
    const char* filename) {
  MimeType r = text_html_content_type;
  if (!filename) {
    return r;
  }

  char* ext = strrchr(filename, '.');
  if (ext) {
    // We found an extension. Skip past the '.'
    ext++;

    char ch;
    unsigned int i = 0;
    while (i < mime_types.length()) {
      // Compare the extension.
      char* p = ext;
      ch = mime_types[i];
      while (*p && ch != '*' && toupper(*p) == ch) {
	p++; i++;
	ch = mime_types[i];
      }
      if (!*p && ch == '*') {
	// We reached the end of the extension while checking
	// equality with a MIME type: we have a match. Increment i
	// to reach past the '*' char, and assign it to `mime_type'.
	r = ++i;
	break;
      } else {
	// Skip past the the '|' character indicating the end of a
	// MIME type.
	while (mime_types[i++] != '|')
	  ;
      }
    }
  }
  return r;
}

void TinyWebServer::send_file(FileFs& file, int hlr) {
	size_t size;
	for(;;) {
		PIT_DisableInterrupts(PIT, kPIT_Chnl_2, kPIT_TimerInterruptEnable);
		if((size = file.read(hlr, obuffer, sizeof(obuffer))) <= 0){
			break;
		}
		PIT_EnableInterrupts(PIT, kPIT_Chnl_2, kPIT_TimerInterruptEnable);
		if (!client_ptr_->connected()) {
			Serial << F("Client disconnect during file transfer");
			Serial.println();
			break;
		}
		write((uint8_t*)obuffer, size);
	}
}

size_t TinyWebServer::write(uint8_t c) {
  client_ptr_->write(c);
}

size_t TinyWebServer::write(const char *str) {
  client_ptr_->write(str);
}

size_t TinyWebServer::write(const uint8_t *buffer, size_t size) {
  client_ptr_->write(buffer, size);
}

boolean TinyWebServer::read_next_char(Client *client, uint8_t* ch) {
  if (!client->available()) {
    return false;
  } else {
    *ch = client->read();
    return true;
  }
}

boolean TinyWebServer::get_line(char* buffer, int size) {
  int i = 0;
  char ch;

  buffer[0] = 0;
  for (; i < size - 1; i++) {
    if (!read_next_char(client_ptr_, (uint8_t*)&ch)) {
      continue;
    }
    if (ch == '\n') {
      break;
    }
    buffer[i] = ch;
  }
  buffer[i] = 0;
  return i < size - 1;
}


// Returns a newly allocated string containing the field number `which`.
// The first field's index is 0.
// The caller is responsible for freeing the returned value.
char* TinyWebServer::get_field(const char* buffer, int which) {
  char* field = NULL;
//  boolean prev_is_space = false;
  int i = 0;
  int field_no = 0;
  int size = strlen(buffer);

  // Locate the field we need. A field is defined as an area of
  // non-space characters delimited by one or more space characters.
  for (; field_no < which; field_no++) {
    // Skip over space characters
    while (i < size && isspace(buffer[i])) {
      i++;
    }
    // Skip over non-space characters.
    while (i < size && !isspace(buffer[i])) {
      i++;
    }
  }

  // Now we identify the end of the field that we want.
  // Skip over space characters.
  while (i < size && isspace(buffer[i])) {
    i++;
  }

  if (field_no == which && i < size) {
    // Now identify where the field ends.
    int j = i;
    while (j < size && !isspace(buffer[j])) {
      j++;
    }

    field = (char*) malloc_check(j - i + 1);
    if (!field) {
      return NULL;
    }
    memcpy(field, buffer + i, j - i);
    field[j - i] = 0;
  }
  return field;
}

// The PUT handler.

namespace TinyWebPutHandler {

HandlerFn put_handler_fn = NULL;

// Fills in `buffer' by reading up to `num_bytes'.
// Returns the number of characters read.
int read_chars(TinyWebServer& web_server, Client* client,
               uint8_t* buffer, int size) {
  uint8_t ch;
  int pos;
  for (pos = 0; pos < size && web_server.read_next_char(client, &ch); pos++) {
    buffer[pos] = ch;
  }
  return pos;
}

boolean put_handler(TinyWebServer& web_server) {
	// Workaround for WINC1500 issue
	// https://github.com/arduino-libraries/WiFi101/issues/70
//  web_server.send_error_code(200);
//  web_server.end_headers();

  const char* length_str = web_server.get_header_value("Content-Length");
  unsigned long length = atol(length_str);
  uint32_t start_time = 0;
  boolean watchdog_start = false;

  WiFiClient *client = web_server.get_client();

  if (put_handler_fn) {
    (*put_handler_fn)(web_server, START, NULL, length);
  }

  uint32_t i;
  for (i = 0; i < length && client->connected();) {
    int16_t size = read_chars(web_server, client, (uint8_t*)obuffer, 512);
    if (!size) {
      if (watchdog_start) {
        if (millis() - start_time > 30000) {
          // Exit if there has been zero data from connected client
          // for more than 30 seconds.
#if DEBUG
          Serial << F("TWS:There has been no data for >30 Sec.\n");
#endif
          break;
        }
      } else {
        // We have hit an empty buffer, start the watchdog.
        start_time = millis();
        watchdog_start = true;
      }
      continue;
    }
    i += size;
    // Ensure we re-start the watchdog if we get ANY data input.
    watchdog_start = false;

    if (put_handler_fn) {
      (*put_handler_fn)(web_server, WRITE, obuffer, size);
    }
  }
  if (put_handler_fn) {
    (*put_handler_fn)(web_server, END, NULL, 0);
    web_server.send_error_code(200);
    web_server.end_headers();
  }

  return true;
}

};


// The FORM handler

namespace TinyWebFormHandler {

FormList *form_handler_list_ptr;
cgiList *cgi_handler_list_ptr;


// Fills in `buffer' by reading up to `num_bytes'.
// Returns the number of characters read.
int read_chars(TinyWebServer& web_server, Client *client,
               uint8_t* buffer, int size) {
  uint8_t ch;
  int pos;
  for (pos = 0; pos < size && web_server.read_next_char(client, &ch); pos++) {
    buffer[pos] = ch;
  }
  return pos;
}

char *get_form_name_uri(const char* path, const char* root) {		// root should be like: /submitform or submitform/form1 or submitform

	int len;
	char *p = strstr(path, root);
	const char* r = strchr(p, '/');
	if(r == NULL)
		return((char*)root);
	r++;
	const char* r2 = strchr(r, '?');
	if(r2 == NULL)
		return NULL;
	len = (r2 - r);
	if(!len){
		return NULL;
	}
	char* value = (char*)malloc_check(len + 1);
	if (!value){
		return NULL;
	}
	strncpy(value, r, len);
	*(value+len)=0;
	return value;
}

char *get_var_from_post_data(char* pdata, const char* var) { return TinyWebServer::get_var_from_uri((const char*)pdata, var);}

// FORM POST
boolean form_handler(TinyWebServer& web_server) {

  FormList *s = form_handler_list_ptr;
  char *form_ = NULL;
  const char* length_str = web_server.get_header_value("Content-Length");
  unsigned int length = atol(length_str);
  int16_t size;
  WiFiClient *client = web_server.get_client();

  if(length < 512 && client->connected()) {
	memset(obuffer, 0, sizeof(obuffer));
    size = read_chars(web_server, client, (uint8_t*)obuffer, 512);
  }else{
		do{
			// because the WINC1500 reception issue...
			size = read_chars(web_server, client, (uint8_t*)obuffer, 512);
		}while(size);
		web_server.send_error_code(413);		// Payload Too Large, length not allowed for a POST
		web_server.end_headers();
  }
#if DEBUG
  Serial.println("testing the get_path function");
  Serial.println(web_server.get_path());
  Serial.println("testing the get_form_name function");
  form_ = get_cgi_post_name_uri(web_server.get_path(), "submitform");
  Serial.println(form_);
#endif
  for(;s != NULL; s++){
	if(!strcmp(s->form_name, form_)){
#if DEBUG
	  Serial.print("calling form method for: ");
	  Serial.println(s->form_name);
#endif
	  (*(s->handler))(web_server, obuffer, size);
	  free(form_);
	  return true;
	}
  }
#if DEBUG
  Serial.println("No method available for this FORM");
#endif
  free(form_);
  return false;
}

char *get_cgi_post_name_uri(const char* path, const char* root) {		// root should be like: /submitform or submitform/form1 or submitform

	int len;
	char *p = strstr(path, root);
	const char* r = strchr(p, '/');
	if(r == NULL)
		return((char*)root);
	r++;
	len = strlen(r);
	if(len > 0){
		char* value = (char*)malloc_check(len + 1);
		if (!value){
			return NULL;
		}
		strncpy(value, r, len);
		*(value+len)=0;
		return value;
	}else{
		return NULL;
	}
}

boolean cgi_handler(TinyWebServer& web_server) {
  cgiList *cgi = cgi_handler_list_ptr;
  char *path_ = NULL;
#if DEBUG
  Serial.println("cgi_handler");
  Serial.println("testing the get_path function");
  Serial.println(web_server.get_path());
  Serial.println("testing the get_cgi_name function");
  path_ = get_cgi_post_name_uri(web_server.get_path(), "intercom_cgi");
#endif
   for(;cgi != NULL; cgi++){
	if(!strcmp(cgi->cgi_name, path_)){
#if DEBUG
	  Serial.print("calling form method for: ");
	  Serial.println(cgi->cgi_name);
#endif
	  (*(cgi->cgihandler))(web_server);
	  free(path_);
	  return true;
	}
  }
#if DEBUG
  Serial.println("No method available for this CGI");
#endif
  free(path_);
  return false;
}

};		// end of TinyWebFormHandler namespace


