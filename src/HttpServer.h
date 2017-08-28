//
// Created by IBD533 on 29/08/2016.
//

#ifndef XLL_HTTPSERVER_H
#define XLL_HTTPSERVER_H

#include "Declarations.h"
#include "Server.h"
#include <winhttp.h>
#include <set>

namespace rms {

	class RMS_EXPORT HttpServer : public Server {
		std::string host;
		std::string context;
		int port;
		bool use_proxy;
		bool use_https;
		bool is_logger;
		std::map<std::string, std::string> http_headers;

		HINTERNET hSession;
		int flags;
		int resolveTimeout = 4000;
		int connectTimeout = 4000;
		int sendTimeout = 60000;
		int receiveTimeout = 60000;
	public:
		HttpServer(JsonMap* data, bool is_logger);
		virtual ~HttpServer() {}
		void close();
		virtual void init(JsonMap* data);
		virtual void init(LPXLOPER12 xDLL);
		virtual JsonObject* send(const std::string& method, const std::string& url, JsonObject* body, bool has_output, bool is_json);
		virtual std::string getContext() { return context; };
		void addHeader(HINTERNET hRequest, const std::string& name, const std::string& val);
		void check(DWORD res);
		virtual JsonMap* toJson();
		void put(const std::string& name, const std::string& val) { http_headers[name] = val;  }
		std::string getKey() { return key;  }
		bool isError(int httpCode);
	};
}

#endif //XLL_HTTPSERVER_H
