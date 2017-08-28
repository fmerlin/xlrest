//
// Created by IBD533 on 29/08/2016.
//

#ifndef XLL_CONFIG_H
#define XLL_CONFIG_H

#include "Declarations.h"
#include "Server.h"

namespace rms {
	typedef std::map<std::string, HttpServer*> ServerByNames;

	class RMS_EXPORT Config : public Server {
		std::string user_agent;
		std::string username;
		std::string hostname;
		std::string session;
		int request;
		ServerByNames servers;
	public:
		virtual ~Config();
		void init3(HINSTANCE hInstance);
		size_t add(Operation* op) { ops.push_back(op); return ops.size() - 1; }
		void remove(int i) { if(i >= 0 && i < ops.size()) ops[i] = NULL; }
		void close();
		virtual void init(LPXLOPER12 xDLL);
		virtual void init(JsonMap* config);
		virtual JsonMap* toJson();
		virtual JsonObject* send(const std::string& method, const std::string& url, JsonObject* body, bool has_output, bool is_json);
		void save();

		LPXLOPER12 execute(int number, LPXLOPER12 v0, LPXLOPER12 v1, LPXLOPER12 v2 \
			, LPXLOPER12 v3, LPXLOPER12 v4, LPXLOPER12 v5, LPXLOPER12 v6, LPXLOPER12 v7, LPXLOPER12 v8 \
			, LPXLOPER12 v9, LPXLOPER12 v10, LPXLOPER12 v11, LPXLOPER12 v12, LPXLOPER12 v13, LPXLOPER12 v14, LPXLOPER12 v15, LPXLOPER12 v16 \
			, LPXLOPER12 v17, LPXLOPER12 v18, LPXLOPER12 v19);

		std::string getUserName() { return username; }
		std::string getHostName() { return hostname; }
		std::string getSheetName();
		virtual std::string getContext() { return ""; };
		std::string getSessionId() { return session; }
		std::string getUserAgent() { return user_agent; }
		std::string getRequestId();
	};

	extern Config * config;
}

#endif //XLL_CONFIG_H
