//
// Created by IBD533 on 27/08/2016.
//
#include "stdafx.h"
#include "Parameter.h"
#include "JsonObject.h"
#include "Server.h"
#include <cstdio>
#include <algorithm>
#include <time.h>
#include <ctime>
#include "Log.h"
#include <numeric>

namespace rms {

	static const char* LOCATIONS[] = { "PATH", "QUERY", "BODY", "FORMDATA", NULL };

	Parameter::Parameter(JsonMap *data) {
		name = data->getString("name");
		description = data->getString("description");
		JsonObject * in = data->get("in");
		if (in)
			location = (Location)data->getEnum(data->getString("in"), LOCATIONS);
		else
			location = (Location)-1;
		required = data->getBool("required");
		defaultObj = data->has("default") ? data->get("default")->clone() : NULL;
		values = data->getStringList("enum");
	}

	Parameter::~Parameter() {
		if(defaultObj)
			delete defaultObj;
	}


	void Parameter::throwRequiredException()
	{
		std::stringstream errMsg;
		errMsg << "Value is required for field '" << name << "'";
		throw std::exception(errMsg.str().c_str());
	}

	bool Parameter::isNull(LPXLOPER12 arg) {
		if (required || defaultObj != NULL)
			return false;
		int type = arg->xltype & ~(xlbitXLFree | xlbitDLLFree);
		return type == xltypeMissing || type == xltypeNil;
	}

	std::string Parameter::toString(LPXLOPER12 arg) {
		int type = arg->xltype & ~(xlbitXLFree | xlbitDLLFree);
		switch (type) {
		case xltypeMissing:
		case xltypeNil:
			if (defaultObj != NULL)
				return defaultObj->toString();
			if (required)
				throwRequiredException();
			return "";
		case xltypeStr:
			return toStringW(std::wstring(&arg->val.str[1], arg->val.str[0]));
		case xltypeErr:
			if (defaultObj != NULL)
				return defaultObj->toString();
			if (required)
				throwRequiredException();
			return "#N/A";
		case xltypeInt:
			return toStringI(arg->val.w);
		case xltypeNum:
			return toStringD(arg->val.num);
		case xltypeBool:
			return toStringB(arg->val.xbool != 0);
		case xltypeMulti:
			return toString((int)arg->val.array.rows, (int)arg->val.array.columns, (LPXLOPER12)&(arg->val.array.lparray[0]));
		default:
			throw std::exception("Unsupported XLOPER type");
		}
	}

	JsonObject *Parameter::toJson(LPXLOPER12 arg) {
		int type = arg->xltype & ~(xlbitXLFree | xlbitDLLFree);
		switch (type) {
		case xltypeMissing:
		case xltypeNil:
			if (defaultObj != NULL)
				return defaultObj->clone();
			if (required)
				throwRequiredException();
			return new JsonId(valNULL);
		case xltypeErr:
			if (defaultObj != NULL)
				return defaultObj;
			if (required)
				throwRequiredException();
			return new JsonId(valERR);
		case xltypeStr:
			return toJsonW(std::wstring(&arg->val.str[1], arg->val.str[0]));
		case xltypeInt:
			return toJsonI(arg->val.w);
		case xltypeNum:
			return toJsonD(arg->val.num);
		case xltypeBool:
			return toJsonB(arg->val.xbool != 0);
		case xltypeMulti:
			return toJson((int)arg->val.array.rows, (int)arg->val.array.columns, (LPXLOPER12)&arg->val.array.lparray[0]);
		default:
			throw std::exception("Unsupported XLOPER type");
		}
	}

	void Parameter::exceptionBadEnum()
	{
		std::stringstream errMsg;
		errMsg << "The authorized value for the parameter '" << name << "' are ";
		errMsg << values[0];
		for (size_t i = 1; i < values.size(); i++)
			errMsg << "," << values[i];
		throw std::exception(errMsg.str().c_str());
	}

	std::string Parameter::toStringW(const std::wstring& arg) {
		return this->toStringS(xToString(arg));
	}

	std::string Parameter::toStringS(const std::string& arg) {
		if (!this->values.empty()) {
			std::vector<std::string>::iterator pos = std::find(values.begin(), values.end(), arg);
			if (pos == values.end()) {
				exceptionBadEnum();
			}
		}
		return arg;
	}

	std::string Parameter::toStringI(int arg) {
		char buf[33];
		snprintf(buf, 32, "%d", arg);
		return std::string(buf);
	}

	std::string Parameter::toStringD(double arg) {
		char buf[33];
		snprintf(buf, 32, "%f", arg);
		return std::string(buf);
	}

	std::string Parameter::toStringB(bool arg) {
		return arg ? "TRUE" : "FALSE";
	}

	JsonObject *Parameter::toJsonS(const std::string& arg) {
		return new JsonString(arg.c_str());
	}

	JsonObject *Parameter::toJsonW(const std::wstring& arg) {
		return toJsonS(toStringW(arg));
	}

	JsonObject *Parameter::toJsonI(int arg) {
		return new JsonInt(arg);
	}

	JsonObject *Parameter::toJsonD(double arg) {
		return new JsonDouble(arg);
	}

	JsonObject *Parameter::toJsonB(bool arg) {
		return new JsonId(arg ? valTRUE : valFALSE);
	}

	std::string Parameter::toString(int rows, int cols, LPXLOPER12 arg) {
		std::ostringstream out;
		int n = rows * cols;
		for (int i = 0; i < n; i++) {
			if (i > 0)
				out << ',';
			out << toString(&arg[i]);
		}
		return out.str();
	}

	std::string StringParameter::toStringS(const std::string& arg) {
		if (!this->values.empty()) {
			std::vector<std::string>::iterator pos = std::find(values.begin(), values.end(), arg);
			if (pos == values.end())
			{
				exceptionBadEnum();
			}
		}
		return arg;
	}

	void strToXlOper(const std::string& str, LPXLOPER12 xl) {
		xl->xltype = xltypeStr;
		int l = str.length();
		if (l > 32267)
			l = 32267;
		std::wstring out = xToWstring(str);
		wchar_t* p = (wchar_t *)malloc(l*2 + 4);
		memcpy(p + 1, out.c_str(), l*2);
		p[0] = l;
		p[l + 1] = 0;
		xl->val.str = (XCHAR*)p;
	}

	void StringParameter::toXlOper(JsonObject *obj, LPXLOPER12 xl) {
		if (obj == NULL || obj->isEmpty()) {
			xl->xltype = xltypeMissing;
			xl->val.str = 0;
			return;
		}
		std::string val = obj->toString();
		strToXlOper(val, xl);
	}

	JsonObject* StringParameter::toJson(int rows, int cols, LPXLOPER12 arg) {
		int n = rows * cols;
		std::ostringstream out;
		for (int i = 0; i < n; i++) {
			if (i > 0)
				out << ',';
			LPXLOPER12 o = &arg[i];
			out << this->Parameter::toString(o);
		}
		return new JsonString(out.str());
	}

#define timegm _mkgmtime

	void dateToXlOper(const std::string& str, LPXLOPER12 xl) {
		tm brokenTime;
		memset(&brokenTime, 0, sizeof(tm));
		brokenTime.tm_year = atoi(str.substr(0, 4).c_str()) - 1900;
		brokenTime.tm_mon = atoi(str.substr(5, 7).c_str()) - 1;
		brokenTime.tm_mday = atoi(str.substr(8, 10).c_str());
		if (str.size() > 12) {
			brokenTime.tm_hour = atoi(str.substr(11, 13).c_str());
			brokenTime.tm_min = atoi(str.substr(14, 16).c_str());
			brokenTime.tm_sec = atoi(str.substr(17, 19).c_str());
		}
		time_t sinceEpoch = timegm(&brokenTime);
		xl->xltype = xltypeNum;
		xl->val.num = double(sinceEpoch) / (24 * 3600) + 70 * 365 + 19;
	}

	void DateParameter::toXlOper(JsonObject *obj, LPXLOPER12 xl) {
		if (obj == NULL || obj->isEmpty()) {
			strToXlOper("",xl);
			return;
		}
		std::string str = obj->toString();
		if (str.size() < 10)
			throw std::exception("Argument is not a date compatible with YYYY-MM-DD");
		dateToXlOper(str,xl);
	}

	void DoubleParameter::toXlOper(JsonObject *obj, LPXLOPER12 xl) {
		if (obj == NULL || obj->isEmpty()) {
			strToXlOper("", xl);
			return;
		}
		xl->xltype = xltypeNum;
		xl->val.num = obj->toDouble();
	}

	void IntParameter::toXlOper(JsonObject *obj, LPXLOPER12 xl) {
		if (obj == NULL || obj->isEmpty()) {
			strToXlOper("", xl);
			return;
		}
		xl->xltype = xltypeInt;
		xl->val.num = obj->toInt();
	}

	void BoolParameter::toXlOper(JsonObject *obj, LPXLOPER12 xl) {
		if (obj == NULL || obj->isEmpty()) {
			strToXlOper("", xl);
			return;
		}
		xl->xltype = xltypeBool;
		xl->val.xbool = obj->toBool();
	}

	void ObjectParameter::toXlOper(JsonObject *obj, LPXLOPER12 xl) {
		if (obj == NULL || obj->isEmpty()) {
			strToXlOper("",xl);
			return;
		}
		if (!def)
			throw std::exception("Object definition not properly interpreted from swagger.");
		std::vector<Parameter*>& fields = def->getFields();
		int n = obj->size();
		int m = fields.size();
		xl->xltype = xltypeMulti;
		xl->val.array.columns = m;
		xl->val.array.rows = n + 1;
		XLOPER12* array = (XLOPER12*)malloc((n + 1) * m * sizeof(XLOPER12));
		xl->val.array.lparray = array;
		int p = 0;
		for (int i = 0; i < m; i++)
			strToXlOper(toUpper(fields[i]->getName()), &array[p++]);
		for (int j = 0; j < n; j++)
			for (int i = 0; i < m; i++)
				fields[i]->toXlOper(obj->get(j)->get(fields[i]->getName()), &array[p++]);
	}

	JsonObject* ObjectParameter::toJson(int rows, int cols, LPXLOPER12 arg) {
		if (rows < 2)
		{
			std::stringstream errMsg;
			errMsg << "There must be at least 2 rows for the parameter '" << name << "'";
			throw std::exception(errMsg.str().c_str());
		}
		JsonList* res = NULL;
		if (rows > 2)
			res = new JsonList(NULL);
		std::vector<Parameter*> fields;
		int p = 0;
		for (int j = 0; j < cols; j++)
			fields.push_back(def->getField(toString(&arg[p++])));
		for (int i = 1; i < rows; i++) {
			JsonMap* res2 = new JsonMap(res);
			for (int j = 0; j < cols; j++)
				res2->put(fields[j]->getName(),fields[j]->toJson(&arg[p++]));
			if (rows == 2)
				return res2;
			if (res != NULL)
				res->add(res2);
		}
		return res;
	}

	std::vector<Parameter*> ObjectDefinition::getFields() {
		std::vector<Parameter*> res;
		for (InsertOrder::const_iterator it = order.begin(); it != order.end(); it++) {
			ParameterByNames::const_iterator pair = fields.find(*it);
			res.push_back(pair->second);
		}
		return res;
	}


	std::string DateParameter::toStringI(int arg) {
		char tm_str[32];
		time_t tm1 = (time_t) ((arg - 25569) * 24. * 3600.);
		struct tm* tm2 = gmtime(&tm1);
		strftime(tm_str, 32, "%Y-%m-%dT0:0:0", tm2);
		return std::string(tm_str);
	}

	std::string DateParameter::toStringD(double arg) {
		char tm_str[32];
		time_t tm1 = (time_t) ((arg - 25569) * 24. * 3600.);
		struct tm* tm2 = gmtime(&tm1);
		if (tm2 == nullptr)
			throw new std::exception("not a valid datetime specified");
		strftime(tm_str, 32, "%Y-%m-%dT%H:%M:%S", tm2);
		//strftime(tm_str, 32, "%Y-%m-%d", tm2);
		return std::string(tm_str);
	}

#define SWAGGER_DEFINITION "#/definitions"

	Parameter *Parameter::create(JsonMap *p,const Server* srv) {
		if (p == NULL)
			internalError();
		std::string type = p->getString("type");
		std::string format = p->getString("format");
		std::string hasExample = p->getString("x-example");
		JsonMap* schema = p->getMap("schema");
		if (schema != NULL) {
			std::string schema_type = schema->getString("type");
			std::string ref;
			if (schema_type == "array") {
				JsonMap* items = schema->getMap("items");
				if (items == NULL)
					throw std::exception("Bad swagger definition");
				ref = items->getString("$ref");
			} else
				ref = schema->getString("$ref");
			ref = (ref.find(SWAGGER_DEFINITION) == std::string::npos) ? ref : ref.substr(sizeof(SWAGGER_DEFINITION));
			if(ref == "")
			{
				return create(schema, srv);
			}
			else {
				ObjectDefinition* def = srv->getDef(ref);
				if (def == nullptr) {
					return create(schema, srv);
				}
				return new ObjectParameter(p, def);
			}
		} 
		if (type == "string")
		{
			if (format == "date" || format == "date-time")
				return new DateParameter(p);
			if(hasExample == "2001-01-01")
				return new DateParameter(p);
			else
				return new StringParameter(p);
		}
		else if (type == "boolean")
			return new BoolParameter(p);
		else if (type == "number")
			return new DoubleParameter(p);
		else if (type == "integer")
			return new IntParameter(p);
		else if (type == "array") {
			JsonMap* items = p->getMap("items");
			if (items == NULL)
				throw std::exception("Bad swagger definition");
			return new ListParameter(p, create(items, srv));
		}
//		else
//			throw new std::exception("swagger.io not well formed");
		return new UnsupportedParameter(p);
	}

	ObjectDefinition::ObjectDefinition(JsonMap* data,Server* srv) {
		JsonMap* p = data->getMap("properties");
		if (p == NULL)
			throw std::exception("Bad swagger definition");
		std::vector<std::string>  required = data->getStringList("required");
		for (InsertOrder::iterator it = p->beginAsInserted(); it != p->endAsInserted(); it++) {
			JsonMap* m = p->get(*it)->toMap();
			m->put("name", new JsonString(*it));
			m->put("in", new JsonString("BODY"));
			if (std::find(required.begin(), required.end(), *it) != required.end())
				m->put("required", new JsonId(valTRUE));
			Parameter* p = Parameter::create(m, srv);
			fields[p->getName()] = p;
			order.push_back(p->getName());
		}
	}


	void ListParameter::toXlOper(JsonObject* obj, LPXLOPER12 xl)
	{
		if (obj == NULL || obj->isEmpty()) {
			strToXlOper("", xl);
			return;
		}
		int n = obj->size();
		xl->xltype = xltypeMulti;
		xl->val.array.columns = 1;
		xl->val.array.rows = n;
		XLOPER12* array = (XLOPER12*)malloc((n) * sizeof(XLOPER12));
		xl->val.array.lparray = array;
		int p = 0;
		for (int j = 0; j < n; j++)
			primitive->toXlOper(obj->get(j), &array[p++]);
	};

	JsonObject* ListParameter:: toJson(int rows, int cols, LPXLOPER12 arg)
	{
		return primitive->toJson(rows, cols, arg);
	};

	void MapParameter::toXlOper(JsonObject* obj, LPXLOPER12 xl)
	{
		if (obj == NULL || obj->isEmpty()) {
			strToXlOper("", xl);
			return;
		}
		int n = obj->size();
		xl->xltype = xltypeMulti;
		xl->val.array.columns = 1;
		xl->val.array.rows = n;
		XLOPER12* array = (XLOPER12*)malloc((n) * sizeof(XLOPER12));
		xl->val.array.lparray = array;
		int p = 0;
		for (int j = 0; j < n; j++)
			value->toXlOper(obj->get(j), &array[p++]);
	};

	JsonObject* MapParameter::toJson(int rows, int cols, LPXLOPER12 arg)
	{
		return value->toJson(rows, cols, arg);
	};
}
