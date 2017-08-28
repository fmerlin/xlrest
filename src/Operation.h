//
// Created by IBD533 on 26/08/2016.
//

#ifndef XLL_OPERATION_H
#define XLL_OPERATION_H

#include "Declarations.h"

#include "xlcall.h"

namespace rms {
	class RMS_EXPORT Operation {
		std::string method;
		std::string name;
		std::string templateUrl;
		std::string summary;
		std::string tag;
		std::string description;
		std::vector<std::string> tags;
		std::vector<std::string> produces;
		std::vector<Parameter*> parameters;
		std::map<int, std::string> responses;
		bool is_json, is_text, is_csv;
		Parameter* output;
		Server* server;
		bool volatil;
		bool active;
		XLOPER12 registrationId;
		int number;
	public:
		Operation(Server * server, const std::string& method, const std::string& templateUrl, JsonMap* data);
		~Operation() { unregisterUDF(); }
		LPXLOPER12 execute(LPXLOPER12* args);
		std::string getName() { return name; }
		void registerUDF(LPXLOPER12 xDLL, int number);
		void unregisterUDF();
		bool isActive() { return active; }
	};
}

#endif //XLL_OPERATION_H
