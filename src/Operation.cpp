//
// Created by IBD533 on 26/08/2016.
//
#include "stdafx.h"
#include "Operation.h"
#include "JsonObject.h"
#include "Config.h"
#include "Parameter.h"
#include "Log.h"
#include <time.h>

namespace rms {
	const char* hexa = "0123456789ABCDEF";

	std::string encodeURL(std::string& s) {
		std::ostringstream out;
		for (std::string::iterator it = s.begin(); it != s.end(); it++) {
			char c = *it;
			if(c >= '0' && c<= '9' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_' || c == '~' || c == '-' || c == '.')
				out << c;
			else {
				int c1 = c & 15;
				int c2 = (c / 16) & 15;
				out << '%' << hexa[c2] << hexa[c1];
			}
		}
		return out.str();
	}

	LPXLOPER12 Operation::execute(LPXLOPER12 *args) {
		clock_t top = clock();
		std::string url = templateUrl;
		std::string query;
		int pos;
		std::auto_ptr<JsonObject> body;
		std::ostringstream str;
		for (size_t i = 0; i < parameters.size(); i++) {
			Parameter &p = *parameters[i];
			std::string name = p.getName();
			if(i > 0)
				str << ",";
			str << name << "=" << p.toString(args[i]);
			switch (p.getLocation()) {
			case PATH:
				pos = url.find("{" + name + "}", 0);
				if (pos >= 0)
					url = url.replace(pos, name.size() + 2, encodeURL(p.toString(args[i])));
				else
					logger->Error("cannot replace '%s' in parameter %s", name.c_str(), url.c_str());
				break;
			case QUERY:
			case FORMDATA: {
				int type = args[i]->xltype & ~(xlbitXLFree | xlbitDLLFree);
				if (type == xltypeMulti)
				{
					int c = args[i]->val.array.columns;
					int r = args[i]->val.array.rows;
					for (int k = 0; k < r; k++) {
						for (int j = 0; j < c; j++) {
							LPXLOPER12 arg = (LPXLOPER12)&(args[i]->val.array.lparray[k*c + j]);
							if (!p.isNull(arg)) {
								if (!query.empty())
									query += '&';
								query += name + '=' + encodeURL(p.toString(arg));
							}
						}
					}
				}
				else if (!p.isNull(args[i]))
				{
					if (!query.empty())
						query += '&';
					query += name + '=' + encodeURL(p.toString(args[i]));
				}
				break;
			}
			case BODY:
				body.reset(p.toJson(args[i]));
				break;
			}
		}

		if (!query.empty())
			url += '?' + query;

		std::auto_ptr<JsonObject> obj(server->send(method,  url, body.get(), output != NULL, is_json));

		LPXLOPER12 res = new XLOPER12;

		if (obj.get() == NULL)
			obj.reset(new JsonString(""));

		obj->toXlOper(res);

		if (logger) {
			double dt = ((double)clock() - top) / CLOCKS_PER_SEC;
			int rows = res->xltype == xltypeMulti ? res->val.array.rows : 1;
			int cols = res->xltype == xltypeMulti ? res->val.array.columns : 1;
			logger->Info("Processing [function=%s] params[%s] for [rows=%d]x[cols=%d] in [time=%f] secs", name.c_str(), str.str().c_str(), rows, cols, dt);
		}

		return res;
	}

	LPXLOPER12 strToXlOper(const std::string& str) {
		LPXLOPER12 xl = new XLOPER12;
		xl->xltype = xltypeStr;
		int l = str.length();
		if (l > 32267)
			l = 32267;
		std::wstring out = xToWstring(str);
		wchar_t* p = (wchar_t *)malloc(l*2 + 4);
		memcpy(p+1, out.c_str(), 2*l);
		p[0] = (SHORT) l;
		p[l + 1] = 0;
		xl->val.str = (XCHAR*)p;
		return xl;
	}

	void Operation::registerUDF(LPXLOPER12 xDLL, int number) {
		if (logger)
			logger->Debug("Registering operation %s", name.c_str());
		this->number = number;
		LPXLOPER12 input[30];
		input[0] = xDLL;
		char numstr[21]; // enough to hold all numbers up to 64-bits
		sprintf(numstr, "FS%d", number);
		input[1] = strToXlOper(std::string(numstr));
		input[2] = strToXlOper(volatil ? "UQQQQQQQQQQQQQQQQQQQQ!" : "UQQQQQQQQQQQQQQQQQQQQ");
		input[3] = strToXlOper(server->prefix + name);
		std::string arg_csv;
		for (size_t j = 0; j < parameters.size(); j++) {
			arg_csv.append(parameters[j]->getName());
			arg_csv.append(",");
		}
		input[4] = strToXlOper(arg_csv.substr(0, arg_csv.size() - 1));
		input[5] = strToXlOper("1");
		input[6] = strToXlOper(server->category);
		input[7] = strToXlOper("");
		input[8] = strToXlOper("");
		input[9] = strToXlOper(description);
		int size = 10;
		for (size_t j = 0; j < parameters.size(); j++)
			input[size++] = strToXlOper(parameters[j]->getDescription());
		int res = Excel12v(xlfRegister, &registrationId, size, input);
		for (int j = 1; j < size; j++)
			free(input[j]);
	}

	void Operation::unregisterUDF() {
		if (logger)
			logger->Debug("Unregistering operation %s", name.c_str());
		int res = Excel12v(xlfUnregister, 0, 1, (LPXLOPER12 *) &registrationId);
		config->remove(number);
	}

	Operation::Operation(Server * server, const std::string &method, const std::string &templateUrl, JsonMap *data) : server(server), method(method),
		templateUrl(templateUrl), volatil(false) {
		this->name = data->getString("operationId");
		this->summary = data->getString("summary");
		this->description = data->getString("description");
		this->produces = data->getStringList("produces");
		this->tags = data->getStringList("tags");
		JsonList *lst = data->getList("parameters");
		JsonMap *rep = data->getMap("responses");

		if (logger)
			logger->Debug("Creating operation %s", name.c_str());

		is_json = std::find(produces.begin(),produces.end(),"application/json") != produces.end();
		is_text = std::find(produces.begin(), produces.end(), "text/plain") != produces.end();
		is_csv = std::find(produces.begin(), produces.end(), "text/csv") != produces.end();

		active = !name.empty();
		active &= is_text || is_json || produces.empty();
		active &= server->methods.empty() || server->methods.find(toUpper(method)) != server->methods.end();
		if (!server->tags.empty()) {
			for (std::vector<std::string>::iterator it = tags.begin(); it != tags.end(); it++)
				if (server->tags.find(*it) != server->tags.end())
					tag = *it;
			active &= tag != "";
		}
		if (lst != NULL) {
			for (size_t i = 0; i < lst->size(); i++) {
				JsonMap *p = lst->get(i)->toMap();
				parameters.push_back(Parameter::create(p, server));
			}
		}
		if(rep != NULL)
		{
            JsonMap* rep200 = rep->getMap("200");
			if (rep200 != NULL) {
                output = Parameter::create(rep200, server);
				for (JsonObjectByNames::iterator it2 = rep->begin(); it2 != rep->end(); it2++)
					responses[atoi(it2->first.c_str())] = it2->second->get("description")->toString();
				return;
			}
            JsonMap* rep204 = rep->getMap("204");
            if (rep204 != NULL) {
                logger->Debug("method %s return void\n", name.c_str());
                return;
            }
		}
		if (logger && active)
			logger->Debug("no response 200 described for method %s\n", name.c_str());
		active = false;
	}
}
