//
// Created by IBD533 on 29/08/2016.
//
#include "stdafx.h"
#include "Config.h"
#include <cstdio>
#include <fstream>
#include <time.h>
#include <windows.h>
#include <WinSock2.h>
#include "JsonObject.h"
#include "HttpServer.h"
#include "Operation.h"
#include "Log.h"
#include <Winhttp.h>
#include "Parameter.h"


namespace rms {
	Config * config;

	void Config::init(LPXLOPER12 xDLL) {
		init2(xDLL, toJson());
	}

	Config::~Config() {
	}
	
	void GetFileDirectory(LPWSTR filename, LPWSTR output)
	{
		int len = wcslen(filename);
		if (len == 0) {
			output[0] = 0;
			return;
		}
		int i = len - 1;
		bool found = false;
		while (true) {
			if (filename[i] == '\\' || filename[i] == '/') {
				found = true;
				break;
			}
			if (i == 0)
				break;
			i--;
		}

		if (found) {
			i++;
			wmemcpy(output, filename, i);
			output[i] = 0;
		}
		else {
			output[0] = 0;
		}
	}


	void Config::init3(HINSTANCE hInstance) {
		try {
			user_agent = APPNAME ":" __DATE__ "T" __TIME__;
			char buffer[MAX_PATH];
			DWORD size = MAX_PATH-1;

			WSADATA wsData;
			int wsaret = WSAStartup(0x101, &wsData);
			if (wsaret)
				throw std::exception("Error during WSAStartup");

			int err = gethostname(buffer, MAX_PATH-1);
			if(err != 0)
				throw std::exception("Error during gethostname");
			buffer[size] = 0;
			hostname = std::string(buffer, size);
			size = MAX_PATH-1;
			wchar_t wbuffer[MAX_PATH-1];
			GetUserName(wbuffer, &size);
			wbuffer[size] = 0;
			username = xToString(std::wstring(wbuffer, size));
			time_t now;
			time(&now);
			struct tm *now2 = gmtime(&now);
			strftime(buffer, MAX_PATH-1, "%y-%m-%dT%H:%M:%S", now2);
			session = std::string(buffer);
			request = 0;
			Tokenizer tk;

			// Add module name to ini
			wchar_t iniFile[MAX_PATH];
			ExpandEnvironmentStrings(L"%USERPROFILE%\\" APPNAME ".json", iniFile, MAX_PATH);
			std::wstring iniFile2(iniFile);
			std::string inif(iniFile2.begin(), iniFile2.end());

			FILE *f = fopen(inif.c_str(), "r");
			if (f == nullptr) {
				//cannot open file
				char* errorMessage = new char[50 + inif.length()];
				sprintf(errorMessage, "Configuration file located in '%s' cannot be opened.", inif.c_str());
				throw std::exception(errorMessage);
			}
			while (1) {
				tk.max = fread(tk.buffer, 1, tk.size, f);
				if (tk.max == 0)
					break;
				tk.next();
			}
			init(tk.getRoot()->toMap());
		}
		catch (std::exception& e) {
			if (logger)
				logger->Error(e);
		}
	}

	void Config::init(JsonMap* config) {
		XLOPER12 xDLL;
		Excel12(xlGetName, &xDLL, 0);
		// init(&xDLL);

		for (JsonObjectByNames::iterator i = config->begin(); i != config->end(); i++) {
			bool is_logger = i->first == "logger";
			HttpServer* srv = new HttpServer(i->second->toMap(), is_logger);
			servers[i->first] = srv;
			if (is_logger && logger) {
				// logger must be setup first
				srv->init(&xDLL);
				logger->setServer(srv);
			}
		}

		for (ServerByNames::iterator i = servers.begin(); i != servers.end(); i++)
			if (i->second->isActive() && i->first != "logger")
				i->second->init(&xDLL);

		Excel12(xlFree, 0, 1, (LPXLOPER)&xDLL);
	}

	void Config::close() {
		try {
/*
			// Fix Me (crash)
			for (ServerByNames::iterator i = servers.begin(); i != servers.end(); i++)
				if (i->first != "logger") {
					HttpServer* srv = i->second;
					delete srv;
				}
*/
			servers.clear();
			ops.clear();
		} catch(std::exception& e) {
			if(logger)
				logger->Error(e);
		}
	}

	void Config::save() {
/*
		JsonMap* doc = new JsonMap(NULL);
		for (ServerByNames::iterator i = servers.begin(); i != servers.end(); i++)
			doc->put(i->first, i->second->toJson());
		std::ofstream out;
		out.open(APPNAME ".json");
		doc->write(out);
		out.close();
*/
	}

//	static const char* CONFIG = "{\"paths\":{\"/config\":{\"get\":{\"tags\":[\"EXCEL\",\"rms_config\"],\"operationId\":\"xlrest_get_config\",\"responses\":{\"200\":{\"schema\":{\"$ref\":\"#/definitions/Server\"}}}},\"post\":{\"tags\":[\"EXCEL\",\"rms_config\"],\"operationId\":\"xlrest_set_config\",\"responses\":{\"200\":{\"schema\":{\"$ref\":\"#/definitions/Server\"}}}}}},\"definitions\":{\"Server\":{\"type\":\"object\",\"properties\":{\"active\":{\"type\":\"boolean\"},\"name\":{\"type\":\"string\"},\"host\":{\"type\":\"string\"},\"port\":{\"type\":\"integer\"},\"context\":{\"type\":\"string\"},\"server_key\":{\"type\":\"string\"},\"use_https\":{\"type\":\"boolean\"},\"use_proxy\":{\"type\":\"boolean\"},\"category\":{\"type\":\"string\"},\"prefix\":{\"type\":\"string\"}}}}}";

	JsonMap* Config::toJson() {
/*
		Tokenizer tk;
		sprintf(tk.buffer, CONFIG);
		tk.max = strlen(CONFIG);
		tk.next();
		return tk.getRoot()->toMap();
*/
		return new JsonMap(NULL);
	}

	JsonObject* Config::send(const std::string& method, const std::string& url, JsonObject* body, bool has_output, bool is_json) {
/*
		if (method == "get") {
			JsonList* doc = new JsonList(NULL);
			for (ServerByNames::iterator i = servers.begin(); i != servers.end(); i++) {
				JsonMap* srv = i->second->toJson();
				srv->put("name",new JsonString(i->first));
				doc->add(srv);
			}
			return doc;
		} else
		if (method == "post") {
			JsonMap* doc = new JsonMap(NULL);
			for (int i = 0; i < body->size();i++) {
				JsonMap * srv = body->get(i)->toMap();
				doc->put(srv->getString("name"),srv);
				srv->remove("name");
			}
			init(doc);
			save();
			delete doc;
			delete body;
			return new JsonString("OK");
		}
*/
		internalError();
	}

	std::string Config::getRequestId() {
		char id[32];
		sprintf(id, "%d", request++);
		return std::string(id);
	}

	std::string Config::getSheetName() {
		XLOPER12 xlRef, xlSheetName;
		char sheetname[256];
		Excel12(xlfCaller, &xlRef, 0);
		Excel12(xlSheetNm, &xlSheetName, 1, &xlRef);

		if ((xlSheetName.xltype & ~(xlbitXLFree | xlbitDLLFree)) == xltypeStr && xlSheetName.val.str) {
			int len = xlSheetName.val.str[0];
			wchar_t *ptr = xlSheetName.val.str + 1;
			ptr[len] = 0;
			snprintf(sheetname, min(len + 1, 256), "%S", ptr);
		}
		else
			sprintf(sheetname, "no-sheet");
		return std::string(sheetname);
	}

	LPXLOPER12 Config::execute(int number, LPXLOPER12 v0, LPXLOPER12 v1, LPXLOPER12 v2 \
		, LPXLOPER12 v3, LPXLOPER12 v4, LPXLOPER12 v5, LPXLOPER12 v6, LPXLOPER12 v7, LPXLOPER12 v8 \
		, LPXLOPER12 v9, LPXLOPER12 v10, LPXLOPER12 v11, LPXLOPER12 v12, LPXLOPER12 v13, LPXLOPER12 v14, LPXLOPER12 v15, LPXLOPER12 v16 \
		, LPXLOPER12 v17, LPXLOPER12 v18, LPXLOPER12 v19) {
		LPXLOPER12 res = NULL;
		if (number < ops.size()) {
			Operation* op = ops[number];
			if (op != NULL) {
				LPXLOPER12 args[20] = { v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19 };
				try {
					res = op->execute(args);
				}
				catch (std::exception& e) {
					logger->Error(e);
					res = new XLOPER12;
					if (strlen(e.what()) == 0) {
						res->xltype = xltypeErr;
						res->val.err = xlerrValue;
					}
					else
					{
						std::string msg(e.what());
						strToXlOper(msg, res);
					}
				}
			}
		}
		if (res == NULL) {
			res = new XLOPER12;
			res->xltype = xltypeErr;
			res->val.err = xlerrName;
		}
		return res;
	}
}
