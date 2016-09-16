// -*- c++ -*-
//
// Copyright 2010 Ovidiu Predescu <ovidiu@gmail.com>
// Date: May, June 2010
//
// Updated: 08-JAN-2012 for Arduno IDE 1.0 by <Hardcore@hardcoreforensics.com>
//
// TinyWebServer for Arduino.

#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__

#include <WiFi101.h>
#include "Server.h"
#include "Client.h"
#include "ff.h"                    /* file system routines */

#include <Print.h>

#define WEB_SERVER_CLIENT_MAX  (7)

class TinyWebServer;

namespace TinyWebPutHandler {
  enum PutAction {
    START,
    WRITE,
    END
  };

  typedef void (*HandlerFn)(TinyWebServer& web_server,
			    PutAction action,
			    char* buffer, int size);

  // An HTTP handler that knows how to handle file uploads using the
  // PUT method. Set the `put_handler_fn' variable below to your own
  // function to handle the characters of the uploaded function.
  boolean put_handler(TinyWebServer& web_server);
  extern HandlerFn put_handler_fn;
};

// added to support forms and cgi
namespace TinyWebFormHandler {

  typedef boolean (*FormHandlerFn)(TinyWebServer& web_server,
		  	  	  char* buffer, int size);

  typedef boolean (*cgiHandlerFn)(TinyWebServer& web_server);

  typedef struct {
    const char* 	form_name;
    FormHandlerFn 	handler;
  } FormList;

  typedef struct {
    const char*  	cgi_name;
    cgiHandlerFn 	cgihandler;
  } cgiList;
  // An HTTP handler that knows how to handle forms using the GET
  // method. Set the `form_handler_fn' variable below to your own
  // function to handle the characters of the form function.
  char *get_cgi_post_name_uri(const char* path, const char* root);
  char *get_var_from_post_data(char* pdata, const char* var);
  boolean form_handler(TinyWebServer& web_server);
  boolean cgi_handler(TinyWebServer& web_server);
  extern FormList *form_handler_list_ptr;
  extern cgiList *cgi_handler_list_ptr;

};


class TinyWebServer : public Print {
public:
  // An HTTP path handler. The handler function takes the path it
  // registered for as argument, and the Client object to handle the
  // response.
  //
  // The function should return true if it finished handling the request
  // and the connection should be closed.
  typedef boolean (*WebHandlerFn)(TinyWebServer& web_server);

  enum HttpRequestType {
    UNKNOWN_REQUEST,
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    ANY,
  };

  // An identifier for a MIME type. The number is opaque to a human,
  // but it's really an offset in the `mime_types' array.
  typedef uint16_t MimeType;

  typedef struct {
    const char* path;
    HttpRequestType type;
    WebHandlerFn handler;
  } PathHandler;

  // Initialize the web server using a NULL terminated array of path
  // handlers, and a NULL terminated array of headers the handlers are
  // interested in.
  //
  // NOTE: Make sure the header names are all lowercase.
  TinyWebServer(PathHandler handlers[], const char** headers,
                const int port=80);

  // Call this method to start the HTTP server
  void begin();

  // Handles a possible HTTP request. It will return immediately if no
  // client has connected. Otherwise the request is handled
  // synchronously.
  //
  // Call this method from the main loop() function to have the Web
  // server handle incoming requests.
  void process();

  bool newClient(WiFiClient* client);
  bool handleNewClient();
  bool clientDisconnect();
  boolean get_line_from_int_buf(char* obuffer, uint8_t sock);
  bool get_line_from_internal(uint8_t sock);
  bool get_all_available(WiFiClient* client, uint8_t sock);
  boolean read_next_char_from_buf(uint8_t* ch);
  // Sends the HTTP status code to the connect HTTP client.
  void send_error_code(int code) {
    send_error_code(client_ptr_, code);
  }
  static void send_error_code(Client* client, int code);

  void send_content_type(MimeType mime_type);
  void send_content_type(const char* content_type);

  // Call this method to indicate the end of the headers.
  inline void end_headers() { client_ptr_->println(); }
  static inline void end_headers(Client *client) { client->println(); }

  // void send_error_code(MimeType mime_type, int code);
  // void send_error_code(const char* content_type, int code);

  const char* get_path();
  const HttpRequestType get_type();
  const char* get_header_value(const char* header);
  WiFiClient* get_client() { return client_ptr_; }

  // Processes the HTTP headers and assigns values to the requested
  // ones in headers_. Returns true when successful, false in case of
  // errors.
  boolean process_headers();

  // Helper methods

  // Assumes `s' is an HTTP encoded URL, replaces all the escape
  // characters in it and returns the unencoded version. For example
  // for "/index%2Ehtm", this method returns "index.htm".
  //
  // The returned string must be free()d by the caller.
  static char* decode_url_encoded(const char* s);

  // Assumes the last component of the URL path is a file
  // name. Returns the file name in upper case, ready to passed to
  // SdFile's open() method.
  //
  // In addition to the file name, it sets `mime_type' to an identifier
  //
  // The returned string must be free()d by the caller.
  static char* get_file_from_path(const char* path);

  // The returned string must be free()d by the caller.
  static char* get_var_from_uri(const char* path, const char* var);

  // Guesses a MIME type based on the extension of `filename'. If none
  // could be guessed, the equivalent of text/html is returned.
  static MimeType get_mime_type_from_filename(const char* filename);

  // Sends the contents of `file' to the currently connected
  // client. The file must be opened in read mode.
  //
  // This is mainly an optimization to reuse the internal static
  // buffer used by this class, which saves us some RAM.
  void send_file(FileFs& file, int hlr);

  // These methods write directly in the response stream of the
  // connected client
  virtual size_t write(uint8_t c);
  virtual size_t write(const char *str);
  virtual size_t write(const uint8_t *buffer, size_t size);

  // Some methods used for testing purposes

  // Returns true if the HTTP request processing should be stopped.
  virtual boolean should_stop_processing() { return !client_ptr_->connected();}

  // Reads a character from the request's input stream. Returns true
  // if the character could be read, false otherwise.
  virtual boolean read_next_char(Client *client, uint8_t* ch);


 protected:
  // Returns the field number `which' from buffer. Fields are
  // separated by spaces. Should be a private method, but made public
  // so it can be tested.
  static char* get_field(const char* buffer, int which);

private:
  // The path handlers
  PathHandler* handlers_;

  typedef struct {
    const char* header;
    char* value;
  } HeaderValue;

  // The headers
  HeaderValue* headers_;

  // The TCP/IP server we use.
  WiFiServer server_;

  char* path_;
  HttpRequestType request_type_;
  WiFiClient *client_ptr_;
  WiFiClient *clients_[WEB_SERVER_CLIENT_MAX] = { NULL };
  bool client_available_[WEB_SERVER_CLIENT_MAX] = { false };
  char *buffer_;
  char *line_ptr_[WEB_SERVER_CLIENT_MAX];				// client line pointer

  // Reads a line from the HTTP request sent by an HTTP client. The
  // line is put in `buffer' and up to `size' characters are written
  // in it.
  boolean get_line(char* buffer, int size);

  // Returns true if the header is marked as requested in the headers_
  // array. As a side effect, the pointer to the actual header is made
  // to point to the one in the headers_ array.
  boolean is_requested_header(const char** header);

  boolean assign_header_value(const char* header, char* value);
};

#endif /* __WEB_SERVER_H__ */
