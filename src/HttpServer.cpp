//
// Created by IBD533 on 29/08/2016.
//
#include "stdafx.h"
#include "HttpServer.h"
#include <winhttp.h>

#include "JsonObject.h"
#include "Config.h"
#include "Log.h"
#include <time.h>

namespace rms {
	void HttpServer::addHeader(HINTERNET hRequest, const std::string &name, const std::string &val) {
		if (val != "") {
			std::wstring header = xToWstring(name + ":" + val);
			check(WinHttpAddRequestHeaders(hRequest, header.c_str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD));
		}
	}

	JsonObject *HttpServer::send(const std::string &method, const std::string &url, JsonObject *body, bool has_response, bool is_json) {
		Tokenizer tk;
		std::ostringstream out;
		JsonObject* res = NULL;
		if (body != NULL) {
			body->write(out);
			put("Content-Type", "application/json;charset=UTF-8");
		}
		std::string out_str = out.str();

		clock_t top = clock();
		std::wstring whost(host.begin(),host.end());
		std::wstring wmethod(method.begin(), method.end());
		std::wstring wurl(url.begin(), url.end());

		HINTERNET hConnect = WinHttpConnect(hSession, whost.c_str(), port, 0);
		HINTERNET hRequest = WinHttpOpenRequest(hConnect, wmethod.c_str(), wurl.c_str(), 0, 0, 0, flags);
		try {
			if(is_logger)
				WinHttpSetTimeouts(hRequest, 100, 100, 100, 100);
			else {
				WinHttpSetTimeouts(hRequest, resolveTimeout, connectTimeout, sendTimeout, receiveTimeout);
				put("X-CLIENT-REQUEST", config->getRequestId());
				put("X-CLIENT-SHEETNAME", config->getSheetName());
			}
			for(std::map<std::string,std::string>::iterator it = http_headers.begin(); it != http_headers.end(); it++)
				addHeader(hRequest, it->first, it->second);
			check(WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)out_str.c_str(), out_str.size(),
				out_str.size(), (DWORD_PTR) this));
			check(WinHttpReceiveResponse(hRequest, 0));
			DWORD status;
			DWORD statusLength = 4;
			check(WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				NULL, &status, &statusLength, 0));
			bool err = isError(status);
			if (has_response || err) {
				tk.setMode(is_json || err ? 0 : -1);
				while (1) {
					DWORD max;
					check(WinHttpReadData(hRequest, tk.buffer, tk.size, &max));
					if (max == 0) {
						tk.max = 1;
						tk.buffer[0] = 0;
						tk.next();
						break;
					}
					tk.max = max;
					tk.next();
				}
				res = tk.getRoot();
			}
			if (err) {
				if (res != NULL) {
					JsonMap* m = res->getMap("error");
					if(m != NULL)
						throw std::exception(m->getString("message").c_str());
				}
				throw std::exception("Error");
			}
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);

			if (logger && !is_logger) {
				double dt = ((double) clock() - top) / CLOCKS_PER_SEC;
				logger->Debug("Requesting Http %s %s in %f", method.c_str(), url.c_str(), dt);
			}
		}
		catch (std::exception &e) {
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			throw e;
		}
		return res;
	}

	void HttpServer::check(DWORD res) {
		if (res != true)
		{
			char serr[32];
			DWORD err = GetLastError();
			sprintf(serr,"Error WinHttp: %d", err);
			throw std::exception(serr);
		}
	}

	bool HttpServer::isError(int httpCode) {
		int major = httpCode/100;
		return major > 2;
	}


	static const std::string GET = "GET";

	void HttpServer::init(LPXLOPER12 xDLL) {
		flags = WINHTTP_FLAG_BYPASS_PROXY_CACHE;
		if (use_https)
			flags |= WINHTTP_FLAG_SECURE;
		WINHTTP_CURRENT_USER_IE_PROXY_CONFIG proxy;
		WinHttpGetIEProxyConfigForCurrentUser(&proxy);
		int proxyType = use_proxy ? WINHTTP_ACCESS_TYPE_NAMED_PROXY : WINHTTP_ACCESS_TYPE_NO_PROXY;
		std::wstring userAgent = xToWstring(config->getUserAgent());
		hSession = WinHttpOpen(userAgent.c_str(), proxyType,
			proxy.lpszProxy, proxy.lpszProxyBypass, 0);
		if (!is_logger) {
			try {
				init2(xDLL, send(GET, context + "/swagger.json", NULL, true, true)->toMap());
			}
			catch (std::exception &e) {
				active = false;
				logger->Error(e);
			}
		}
		key = config->getUserName();
	}

	void HttpServer::close() {
		Server::close();
		WinHttpCloseHandle(hSession);
	}

	HttpServer::HttpServer(JsonMap *data, bool is_logger) : is_logger(is_logger) {
		init(data);
		if (logger) {
			if(is_logger)
				logger->setServer(this);
			else
				logger->Debug("Creating http client to %s://%s:%d/%s", use_https ? "https" : "http", host.c_str(), port, context.c_str());
		}
	}

	void HttpServer::init(JsonMap* data) {
		host = data->getString("host");
		port = data->getInt("port");
		context = data->getString("context");
		active = data->getBool("active");
		use_proxy = data->getBool("use_proxy");
		use_https = data->getBool("use_https");
		key = data->getString("key");
		prefix = data->getString("prefix");
		std::vector<std::string> lst = data->getStringList("methods");
		methods.clear();
		methods.insert(lst.begin(), lst.end());
		category = data->getString("category");
		lst = data->getStringList("tags");
		tags.clear();
		tags.insert(lst.begin(),lst.end());
		if(is_logger)
			http_headers["Authorization"] = data->getString("authorization");
		else if(config) {
			if (key.empty())
				key = config->getUserName();
			http_headers["X-CLIENT-HOSTNAME"] = config->getHostName();
			http_headers["X-CLIENT-USERNAME"] = config->getUserName();
			http_headers["X-CLIENT-SHEETNAME"] = config->getSheetName();
			http_headers["X-CLIENT-SESSION"] = config->getSessionId();
			http_headers["X-CLIENT-KEY"] = key;
		}
	}

	JsonMap* HttpServer::toJson() {
		JsonMap* res = new JsonMap(NULL);
		res->put("host", new JsonString(host));
		res->put("port", new JsonInt(port));
		res->put("context", new JsonString(context));
		res->put("key", new JsonString(key));
		res->put("prefix", new JsonString(prefix));
		res->put("category", new JsonString(category));
		res->put("active", new JsonId(active ? valTRUE : valFALSE));
		res->put("use_proxy", new JsonId(use_proxy ? valTRUE : valFALSE));
		res->put("use_https", new JsonId(use_https ? valTRUE : valFALSE));
		JsonList* lst = new JsonList(res);
		for(std::set<std::string>::iterator it = tags.begin(); it != tags.end(); it++)
			lst->add(new JsonString(*it));
		res->put("tags", lst);
		lst = new JsonList(res);
		for (std::set<std::string>::iterator it = methods.begin(); it != methods.end(); it++)
			lst->add(new JsonString(*it));
		res->put("methods", lst);
		return res;
	}
}
