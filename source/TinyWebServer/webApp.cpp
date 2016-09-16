
#include "sdCardSPI.h"
#include "ff.h"
#include "FatFs.h"
#include "Serial.h"
#include "TinyWebServer.h"
#include "Flash.h"
#include "fsl_pit.h"
#include "webApp.h"


void send_file_name(TinyWebServer& web_server, const char* filename);
boolean file_handler(TinyWebServer& web_server);
boolean index_handler(TinyWebServer& web_server);
boolean removefilehandler(TinyWebServer& web_server);
// FORM handlers
boolean dateform_handler(TinyWebServer& web_server, char* buffer, int size);
// CGI handlers
boolean rtc_cgi(TinyWebServer& web_server);


extern FileFs file;
extern SerialConsole Serial;
extern boolean has_filesystem;

const char* headers[] = {
  "Content-Length",
  NULL
};

TinyWebServer::PathHandler handlers[] = {
	{"/upload/" "*", TinyWebServer::PUT, &TinyWebPutHandler::put_handler },
	{"/remove/" "*", TinyWebServer::GET, &removefilehandler },
	{"/submitform" "*", TinyWebServer::POST, &TinyWebFormHandler::form_handler },
	{"/intercom_cgi" "*", TinyWebServer::GET, &TinyWebFormHandler::cgi_handler },
	{"/", TinyWebServer::GET, &index_handler },
	{"/" "*", TinyWebServer::GET, &file_handler },
	{NULL},
};

TinyWebFormHandler::FormList form_handler_list[] = {
  {"available", NULL},
  {"dateform", &dateform_handler},
  {NULL},
};

TinyWebFormHandler::cgiList cgi_handler_list[] = {
  {"available", NULL},
  {"localtime", &rtc_cgi},
  {NULL},
};


TinyWebServer web = TinyWebServer(handlers, headers);

boolean file_handler(TinyWebServer& web_server) {

  if(!has_filesystem) {
    web_server.send_error_code(500);
    web_server << F("Internal Server Error");
    return true;
  }

  char* filename = TinyWebServer::get_file_from_path(web_server.get_path());

  if(!filename) {
  	web_server.send_error_code(400);
  	web_server << F("Bad Request");
  	return true;
  }

  send_file_name(web_server, filename);
  free(filename);
  return true;
}

void send_file_name(TinyWebServer& web_server, const char* filename) {


	TinyWebServer::MimeType mime_type
		= TinyWebServer::get_mime_type_from_filename(filename);
	PIT_DisableInterrupts(PIT, kPIT_Chnl_2, kPIT_TimerInterruptEnable);
	int fh = file.open(filename, FA_READ);
	PIT_EnableInterrupts(PIT, kPIT_Chnl_2, kPIT_TimerInterruptEnable);
	if (fh >= 0) {
		web_server.send_error_code(200);
		web_server.send_content_type(mime_type);
		web_server.end_headers();

		Serial << F("Read file "); Serial.println(filename);
		web_server.send_file(file, fh);
		PIT_DisableInterrupts(PIT, kPIT_Chnl_2, kPIT_TimerInterruptEnable);
		file.close(fh);
		PIT_EnableInterrupts(PIT, kPIT_Chnl_2, kPIT_TimerInterruptEnable);
	} else {
		web_server.send_error_code(404);
		web_server.send_content_type("text/plain");
		web_server.end_headers();

		Serial << F("Could not find file: "); Serial.println(filename);
		web_server << F("404 - File not found") << filename << "\n";
	}

}

boolean index_handler(TinyWebServer& web_server) {
  web_server.send_error_code(200);
  web_server.end_headers();
//  send_file_name(web_server, "index.html");
  web_server << F("<html><body><h1>Hello World NXP!</h1></body></html>\n");
  return true;
}

void file_uploader_handler(TinyWebServer& web_server,
			   TinyWebPutHandler::PutAction action,
			   char* buffer, int size) {
  static uint32_t start_time;
  static uint32_t total_size;
  static int fh = -1;

  switch (action) {
  case TinyWebPutHandler::START:
    start_time = millis();
    total_size = 0;
    if (!file.isOpen(fh)) {
      // File is not opened, create it. First obtain the desired name
      // from the request path.
      char* fname = web_server.get_file_from_path(web_server.get_path());
      if (fname) {
		Serial << F("Creating ") << fname << "\r\n";
		fh = file.open(fname, FA_WRITE | FA_CREATE_ALWAYS );
		free(fname);
      }
    }
    break;

  case TinyWebPutHandler::WRITE:
    if (file.isOpen(fh)) {
      file.write(fh, buffer, size);
      total_size += size;
    }
    break;

  case TinyWebPutHandler::END:
    Serial << F("Wrote ") << file.fileSize(fh) << F(" bytes in ")
	   << millis() - start_time << F(" millis (received ")
           << total_size << F(" bytes)\r\n");
    file.close(fh);
    fh = -1;
  }
}

boolean removefilehandler(TinyWebServer& web_server){
  char* filename = TinyWebServer::get_file_from_path(web_server.get_path());
  Serial.println(filename);
  if (filename) {
	web_server.send_error_code(200);
	web_server.send_content_type("text/html");
	web_server.end_headers();
    if(file.remove(filename)){
      Serial.println("file removed");
      web_server << F("<html><body><h1>Delete file OK</h1></body></html>\n");
    }else{
      Serial.println("file remove fail!");
      web_server << F("<html><body><h1>Delete file FAIL</h1></body></html>\n");
      //web_server << Fmod("<html><body><h1>Could not delete file: %s</h1></body></html>\n", filename);
    }
  }

  free(filename);
  return true;
}

// *********************
// FORM HANDLERS
//**********************

boolean dateform_handler(TinyWebServer& web_server, char* buffer, int size){
  Serial.println("FORM datetime_handler");
  Serial.println(web_server.get_path());
  char *datetime = TinyWebFormHandler::get_var_from_post_data(buffer, "datetime");
  Serial.print("datetime= ");
  Serial.println(datetime);
  free(datetime);
  web_server.send_error_code(200);
  web_server.end_headers();
  return true;
}

// *********************
// FORM HANDLERS
//**********************

boolean rtc_cgi(TinyWebServer& web_server) {
  Serial.println("show local time method");
  web_server.send_error_code(200);
  web_server.end_headers();
//  String t = getSysDateTime();
  String t = String("06/12/2016 12:00:00");
  char time[20];
  web_server << F("<b>");
  t.toCharArray(time, t.length()+1);
  web_server << F(time);
  web_server << F("</b>\n");
}


void setAppHandlers(void){
	//register the upload file handler
	TinyWebPutHandler::put_handler_fn = file_uploader_handler;
    // Assign the form functions to "form_handler_list"
    TinyWebFormHandler::form_handler_list_ptr = form_handler_list;
    // Assign the cgi functions to "cgi_handler_list"
    TinyWebFormHandler::cgi_handler_list_ptr = cgi_handler_list;
}
