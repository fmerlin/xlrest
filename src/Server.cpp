//
// Created by IBD533 on 29/08/2016.
//
#include "stdafx.h"
#include "Server.h"
#include "JsonObject.h"
#include "Operation.h"
#include "Config.h"
#include "Parameter.h"
#include "Log.h"

namespace rms {

	ObjectDefinition* Server::getDef(const std::string& name) const
	{
		DefinitionByNames::const_iterator it = defs.find(name);
		if (it == defs.end())
		{
			return nullptr;
		}
		return it->second;
	}

	void Server::init2(LPXLOPER12 xDLL, JsonMap *swagger) {
		JsonMap* mydefs = swagger->getMap("definitions");
		if(mydefs)
			for (JsonObjectByNames::iterator it = mydefs->begin(); it != mydefs->end(); it++) {
				JsonMap* data = it->second->toMap();
				std::string type = data->getString("type");
				if(type == "object")
					defs[it->first] = new ObjectDefinition(data,this);
			}
		JsonMap *paths = swagger->getMap("paths");
		if (paths)
			for (JsonObjectByNames::iterator i = paths->begin(); i != paths->end(); i++) {
				JsonMap *methods = (JsonMap *)i->second;
				for (JsonObjectByNames::iterator j = methods->begin(); j != methods->end(); j++) {
					try {
						Operation* op = new Operation(this, j->first, getContext().append(i->first), (JsonMap *)j->second);
						ops.push_back(op);
						if (op->isActive()) {
							int number = config->add(op);
							op->registerUDF(xDLL, number);
						}
					}
					catch(std::exception& e)
					{
						logger->Error(e);
						logger->Error("rejecting function %s %s\n", j->first.c_str(), i->first.c_str());
					}
				}
			}
		delete swagger;
	}

	Server::~Server() {
		close();
	}

	void Server::close() {
		for (DefinitionByNames::iterator it = defs.begin(); it != defs.end(); it++)
			delete it->second;
		defs.clear();
		for (size_t i = 0; i < ops.size(); i++)
			delete ops[i];
		ops.clear();
	}
}
