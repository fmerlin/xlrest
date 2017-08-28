//
// Created by IBD533 on 29/08/2016.
//

#ifndef XLL_SERVER_H
#define XLL_SERVER_H

#include "Declarations.h"
#include "xlcall.h"
#include <set>

namespace rms {
	typedef std::map<std::string, ObjectDefinition*> DefinitionByNames;

	class RMS_EXPORT Server {
	protected:
		std::vector<Operation*> ops;
		DefinitionByNames defs;
		bool active;
		std::string key;
		std::string category;
		std::string prefix;
		std::set<std::string> tags;
		std::set<std::string> methods;
		virtual void init2(LPXLOPER12 xDLL, JsonMap* data);
		friend class Operation;
	public:
		Server() :active(false) {};
		virtual ~Server();
		bool isActive() { return active; }
		virtual JsonObject* send(const std::string& method, const std::string& url, JsonObject* body, bool has_output, bool is_json) = 0;
		virtual void init(JsonMap* config) = 0;
		virtual JsonMap* toJson() = 0;
		virtual void init(LPXLOPER12 xDLL) = 0;
		virtual std::string getContext() = 0;
		virtual void close();
		ObjectDefinition* getDef(const std::string& name) const;
	};
}
#endif //XLL_SERVER_H
