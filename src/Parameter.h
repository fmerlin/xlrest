//
// Created by IBD533 on 27/08/2016.
//

#ifndef XLL_PARAMETER_H
#define XLL_PARAMETER_H

#include "Declarations.h"

#include "xlcall.h"
#include <string>
#include <exception>

namespace rms {
	enum Location {
		PATH,
		QUERY,
		BODY,
		FORMDATA
	};

	class RMS_EXPORT Parameter {
	protected:
		bool required;
		Location location;
		std::string name;
		JsonObject* defaultObj;
		std::vector<std::string> values;
		std::string description;
	public:
		Parameter(JsonMap* data);
		virtual ~Parameter();
		void throwRequiredException();
		virtual JsonObject* toJson(LPXLOPER12 arg);
		void exceptionBadEnum();
		virtual std::string toString(LPXLOPER12 arg);
		virtual void toXlOper(JsonObject* arg, LPXLOPER12 xl) = 0;
		virtual std::string toStringS(const std::string& arg);
		virtual std::string toStringW(const std::wstring& arg);
		virtual std::string toStringI(int arg);
		virtual std::string toStringD(double arg);
		virtual std::string toStringB(bool arg);
		virtual std::string toString(int rows, int cols, LPXLOPER12 arg);
		virtual JsonObject* toJsonS(const std::string& arg);
		virtual JsonObject* toJsonW(const std::wstring& arg);
		virtual JsonObject* toJsonI(int arg);
		virtual JsonObject* toJsonD(double arg);
		virtual JsonObject* toJsonB(bool arg);
		virtual JsonObject* toJson(int rows, int cols, LPXLOPER12 arg) = 0;
		std::string getName() { return name; }
		std::string getDescription() { return description; }
		Location getLocation() { return location; }
		bool isNull(LPXLOPER12 arg);

		static Parameter* create(JsonMap* data, const Server* srv);
	};

	class RMS_EXPORT DateParameter : public Parameter {
	public:
		DateParameter(JsonMap* data) : Parameter(data) {}
		virtual std::string toStringI(int arg);
		virtual std::string toStringD(double arg);
		virtual void toXlOper(JsonObject* obj, LPXLOPER12 xl);
		virtual JsonObject* toJson(int rows, int cols, LPXLOPER12 arg) { internalError(); }
	};

	typedef std::map<std::string, Parameter*> ParameterByNames;
	typedef std::vector<std::string> InsertOrder;

	class ObjectDefinition {
		ParameterByNames fields;
		InsertOrder order;
	public:
		ObjectDefinition(JsonMap* data,Server* srv);
		Parameter* getField(const std::string& name) { return fields[name]; }
		std::vector<Parameter*> getFields();
	};

	class RMS_EXPORT ObjectParameter : public Parameter {
		ObjectDefinition* def;
	public:
		ObjectParameter(JsonMap* data,ObjectDefinition* def) : Parameter(data), def(def) {}
		virtual void toXlOper(JsonObject* obj, LPXLOPER12 xl);
		virtual JsonObject* toJson(int rows, int cols, LPXLOPER12 arg);
	};

	class RMS_EXPORT ListParameter : public Parameter {
		Parameter* primitive;
	public:
		ListParameter(JsonMap* data, Parameter* primitive) : Parameter(data), primitive(primitive) {}
		virtual void toXlOper(JsonObject* obj, LPXLOPER12 xl);
		virtual JsonObject* toJson(int rows, int cols, LPXLOPER12 arg);
	};

	class RMS_EXPORT MapParameter : public Parameter {
		Parameter* key;
		Parameter* value;
	public:
		MapParameter(JsonMap* data, Parameter* primitive) : Parameter(data), key(key), value(value) {}
		virtual void toXlOper(JsonObject* obj, LPXLOPER12 xl);
		virtual JsonObject* toJson(int rows, int cols, LPXLOPER12 arg);
	};

	class RMS_EXPORT IntParameter : public Parameter {
	public:
		IntParameter(JsonMap* data) : Parameter(data) {}
		virtual void toXlOper(JsonObject* obj, LPXLOPER12 xl);
		virtual JsonObject* toJson(int rows, int cols, LPXLOPER12 arg) { internalError(); }
		virtual std::string toStringD(double val) { return toStringI(val); }
	};

	class RMS_EXPORT DoubleParameter : public Parameter {
	public:
		DoubleParameter(JsonMap* data) : Parameter(data) {}
		virtual void toXlOper(JsonObject* obj, LPXLOPER12 xl);
		virtual JsonObject* toJson(int rows, int cols, LPXLOPER12 arg) { internalError(); }
	};

	class RMS_EXPORT StringParameter : public Parameter {
	public:
		StringParameter(JsonMap* data) : Parameter(data) {}
		virtual void toXlOper(JsonObject* obj, LPXLOPER12 xl);
		virtual std::string toStringS(const std::string& arg);
		virtual JsonObject* toJson(int rows, int cols, LPXLOPER12 arg);
	};

	class RMS_EXPORT BoolParameter : public Parameter {
	public:
		BoolParameter(JsonMap* data) : Parameter(data) {}
		virtual void toXlOper(JsonObject* obj, LPXLOPER12 xl);
		virtual JsonObject* toJson(int rows, int cols, LPXLOPER12 arg) { internalError(); }
	};

	class RMS_EXPORT UnsupportedParameter : public Parameter {
	public:
		UnsupportedParameter(JsonMap* data) : Parameter(data) {}
		virtual void toXlOper(JsonObject* obj, LPXLOPER12 xl) { internalError(); }
		virtual JsonObject* toJson(LPXLOPER arg) { internalError(); }
		virtual std::string toString(LPXLOPER arg) { internalError(); }
		virtual JsonObject* toJson(int rows, int cols, LPXLOPER12 arg) { internalError(); }
	};

	void strToXlOper(const std::string& str, LPXLOPER12 xl);
	void dateToXlOper(const std::string& str, LPXLOPER12 xl);
}

#endif //XLL_PARAMETER_H
