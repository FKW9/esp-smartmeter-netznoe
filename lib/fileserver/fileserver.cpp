#include "fileserver.h"
#include "html.h"

const String default_httpuser = "admin";
const String default_httppassword = "smartmeter";
const int default_webserverporthttp = 80;

// configuration structure
struct Config
{
	String httpuser;	   // username to access web admin
	String httppassword;   // password to access web admin
	int webserverporthttp; // http port number for web admin
};

// variables
Config config;			// configuration
AsyncWebServer *server; // initialise webserver

// function defaults
String listFiles(bool ishtml = true);
void setupFileserver();
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
bool checkUserWebAuth(AsyncWebServerRequest *request);
void notFound(AsyncWebServerRequest *request);
void configureWebServer();
String processor(const String &var);
String humanReadableSize(const size_t bytes);

void setupFileserver()
{
	config.httpuser = default_httpuser;
	config.httppassword = default_httppassword;
	config.webserverporthttp = default_webserverporthttp;
	Serial.println("Configuring Webserver ...");
	server = new AsyncWebServer(config.webserverporthttp);
	configureWebServer();

	// startup web server
	Serial.println("Starting Webserver ...");
	server->begin();
}

// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml)
{
	String returnText = "";
	File root = SD_MMC.open("/");
	File foundfile = root.openNextFile();
	if (ishtml)
	{
		returnText += "<table class='center'><tr><th align='center'>File Name</th><th align='center'>Size</th><th></th><th></th></tr>";
	}
	// This may run into timeout when sd card has too much files on, only show first 100
	int max_count = 0;
	while (foundfile && (max_count++ < 150))
	{
		String fname = String(foundfile.name());
		// Serial.println(fname);
		if (!(fname.startsWith("/.") || fname.startsWith("/Sys")))
		{
			if (ishtml)
			{
				returnText += "<tr align='left'><td>" + fname + "</td><td>" + humanReadableSize(foundfile.size()) + "</td>";
				returnText += "<td><button onclick=\"downloadDeleteButton(\'" + fname + "\', \'download\')\">Download</button>";
				returnText += "<td><button onclick=\"downloadDeleteButton(\'" + fname + "\', \'delete\')\">Delete</button></tr>";
			}
			else
			{
				returnText += "File: " + fname + " Size: " + humanReadableSize(foundfile.size()) + "\n";
			}
		}
		foundfile = root.openNextFile();
	}

	if (ishtml)
	{
		returnText += "</table>";
	}
	if (max_count >= 150)
	{
		returnText = "<p style='color:red; font-weight:bold;'>Only first 150 Files can be shown!</p>" + returnText;
	}

	root.close();
	foundfile.close();
	return returnText;
}

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(const size_t bytes)
{
	if (bytes < 1024)
		return String(bytes) + " B";
	else if (bytes < (1024 * 1024))
		return String(bytes / 1024.0) + " KB";
	else if (bytes < (1024 * 1024 * 1024))
		return String(bytes / 1024.0 / 1024.0) + " MB";
	else
		return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

// parses and processes webpages
// if the webpage has %SOMETHING% or %SOMETHINGELSE% it will replace those strings with the ones defined
String processor(const String &var)
{
	char buf[64];

	if (var == "FREESPIFFS")
	{
		sprintf(buf, "%lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
		return String(buf);
	}

	if (var == "USEDSPIFFS")
	{
		sprintf(buf, "%lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
		return String(buf);
	}

	if (var == "TOTALSPIFFS")
	{
		uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
		sprintf(buf, "%lluMB\n", cardSize);
		return String(buf);
	}
}

void configureWebServer()
{
	// configure web server
	// if url isn't found
	server->onNotFound(notFound);

	// run handleUpload function when any file is uploaded
	server->onFileUpload(handleUpload);

	// visiting this page will cause you to be logged out
	server->on("/logout", HTTP_GET, [](AsyncWebServerRequest *request)
			   {
				   request->requestAuthentication();
				   request->send(401);
			   });

	// presents a "you are now logged out webpage
	server->on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request)
			   {
				   String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
				   Serial.println(logmessage);
				   request->send_P(401, "text/html", logout_html, processor);
			   });

	server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
			   {
				   String logmessage = "Client:" + request->client()->remoteIP().toString() + +" " + request->url();

				   if (checkUserWebAuth(request))
				   {
					   logmessage += " Auth: Success";
					   Serial.println(logmessage);
					   request->send_P(200, "text/html", index_html, processor);
				   }
				   else
				   {
					   logmessage += " Auth: Failed";
					   Serial.println(logmessage);
					   return request->requestAuthentication();
				   }
			   });

	server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
			   {
				   String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();

				   if (checkUserWebAuth(request))
				   {
					   request->send(200, "text/html", reboot_html);
					   logmessage += " Auth: Success";
					   Serial.println(logmessage);
					   ESP.restart();
				   }
				   else
				   {
					   logmessage += " Auth: Failed";
					   Serial.println(logmessage);
					   return request->requestAuthentication();
				   }
			   });

	server->on("/listfiles", HTTP_GET, [](AsyncWebServerRequest *request)
			   {
				   String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
				   if (checkUserWebAuth(request))
				   {
					   logmessage += " Auth: Success";
					   Serial.println(logmessage);
					   request->send(200, "text/plain", listFiles(true));
				   }
				   else
				   {
					   logmessage += " Auth: Failed";
					   Serial.println(logmessage);
					   return request->requestAuthentication();
				   }
			   });

	server->on("/file", HTTP_GET, [](AsyncWebServerRequest *request)
			   {
				   String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
				   if (checkUserWebAuth(request))
				   {
					   logmessage += " Auth: Success";
					   Serial.println(logmessage);

					   if (request->hasParam("name") && request->hasParam("action"))
					   {
						   const char *fileName = request->getParam("name")->value().c_str();
						   const char *fileAction = request->getParam("action")->value().c_str();

						   logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" + String(fileName) + "&action=" + String(fileAction);

						   if (!SD_MMC.exists(fileName))
						   {
							   Serial.println(logmessage + " ERROR: file does not exist");
							   request->send(400, "text/plain", "ERROR: file does not exist");
						   }
						   else
						   {
							   Serial.println(logmessage + " file exists");
							   if (strcmp(fileAction, "download") == 0)
							   {
								   logmessage += " downloaded";
								   request->send(SD_MMC, fileName, "application/octet-stream");
							   }
							   else if (strcmp(fileAction, "delete") == 0)
							   {
								   logmessage += " deleted";
								   SD_MMC.remove(fileName);
								   request->send(200, "text/plain", "Deleted File: " + String(fileName));
							   }
							   else
							   {
								   logmessage += " ERROR: invalid action param supplied";
								   request->send(400, "text/plain", "ERROR: invalid action param supplied");
							   }
							   Serial.println(logmessage);
						   }
					   }
					   else
					   {
						   request->send(400, "text/plain", "ERROR: name and action params required");
					   }
				   }
				   else
				   {
					   logmessage += " Auth: Failed";
					   Serial.println(logmessage);
					   return request->requestAuthentication();
				   }
			   });
}

void notFound(AsyncWebServerRequest *request)
{
	String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
	Serial.println(logmessage);
	request->send(404, "text/plain", "Not found");
}

// used by server.on functions to discern whether a user has the correct httpapitoken OR is authenticated by username and password
bool checkUserWebAuth(AsyncWebServerRequest *request)
{
	bool isAuthenticated = false;

	if (request->authenticate(config.httpuser.c_str(), config.httppassword.c_str()))
	{
		Serial.println("is authenticated via username and password");
		isAuthenticated = true;
	}
	return isAuthenticated;
}

// handles uploads to the filserver
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
	// make sure authenticated before allowing upload
	if (checkUserWebAuth(request))
	{
		String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
		Serial.println(logmessage);

		if (!index)
		{
			logmessage = "Upload Start: " + String(filename);
			// open the file on first call and store the file handle in the request object
			request->_tempFile = SD_MMC.open("/" + filename, "w");
			Serial.println(logmessage);
		}

		if (len)
		{
			// stream the incoming chunk to the opened file
			request->_tempFile.write(data, len);
			logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
			Serial.println(logmessage);
		}

		if (final)
		{
			logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
			// close the file handle as the upload is now done
			request->_tempFile.close();
			Serial.println(logmessage);
			request->redirect("/");
		}
	}
	else
	{
		Serial.println("Auth: Failed");
		return request->requestAuthentication();
	}
}

void stopFileserver()
{
	Serial.println("Stopping Fileserver");
	server->reset();
	server->end();
}