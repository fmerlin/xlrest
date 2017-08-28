//
// Created by IBD533 on 27/08/2016.
//

#ifndef XLL_JSONOBJECT_H
#define XLL_JSONOBJECT_H

#include "Declarations.h"
#include "xlcall.h"

#include <sstream>
#include <memory>


#define TOKEN_BUFFER_SZ 1023

namespace rms {
	extern JsonId* NOTHING;
	
	std::string toUpper(const std::string & name);

	class RMS_EXPORT JsonObject {
	public:
		virtual ~JsonObject() {}

		virtual void add(JsonObject *child) = 0;

		virtual void write(std::ostream &out) = 0;

		virtual JsonObject *getParent() { return NULL; }

		virtual bool isEmpty() { return false; }

		virtual std::string toString() = 0;

		virtual int toInt() = 0;

		virtual double toDouble() = 0;

		virtual bool toBool() = 0;

		virtual std::vector<std::string> toStringList() {
			std::vector<std::string> res;
			if(!isEmpty())
				res.push_back(toString());
			return res;
		}

		virtual size_t size() { return 1; }

		virtual JsonObject *get(const std::string &name);

		virtual JsonMap *getMap(const std::string &name) { return get(name)->toMap(); }

		virtual JsonList *getList(const std::string &name) { return get(name)->toList(); }

		virtual std::vector<std::string> getStringList(const std::string &name) { return get(name)->toStringList(); }

		virtual bool has(const std::string &name) { return false; }

		virtual JsonObject *get(int pos) { return this; }

		virtual JsonMap *toMap() { return NULL; }

		virtual JsonList *toList() { return NULL; }

		virtual JsonObject* clone() = 0;

		virtual void toXlOper(LPXLOPER12 data) = 0;

		virtual XlOperTable* toTable() = 0;

		virtual void finish() {}
	};

	class RMS_EXPORT Tokenizer {
		char token[TOKEN_BUFFER_SZ + 1];
		unsigned int sz, pos;
		int mode;
		char quote;
		JsonObject *parent;
		JsonObject *root;

		void nextObject();

		void nextNumber();

		void nextString();

		void nextId();

		void nextText();

	public:

		Tokenizer() : parent(nullptr), root(nullptr), sz(0), pos(0), mode(0), quote(0) {};

		~Tokenizer();

		void next();

		JsonObject *add(JsonObject *obj);

		JsonObject *getRoot() { return root; }

		void setMode(int mode) { this->mode = mode;  }

		size_t max;
		static const int size = TOKEN_BUFFER_SZ;
		char buffer[size + 1];
	};

	class RMS_EXPORT JsonInt : public JsonObject {
		int val;
	public:
		JsonInt(int val) : val(val) {}

		virtual void add(JsonObject *child) { internalError(); }

		virtual void write(std::ostream &out);

		virtual std::string toString();

		virtual int toInt() { return val; }

		virtual double toDouble() { return val; }

		virtual bool toBool() { return val != 0; }

		virtual JsonObject* clone() { return new JsonInt(val); }

		virtual void toXlOper(LPXLOPER12 data);

		virtual XlOperTable* toTable();
	};

	class RMS_EXPORT JsonDouble : public JsonObject {
		double val;
	public:
		JsonDouble(double val) : val(val) {}

		virtual void add(JsonObject *child) { internalError(); }

		virtual void write(std::ostream &out);

		virtual std::string toString();

		virtual int toInt() { return (int) val; }

		virtual double toDouble() { return val; }

		virtual bool toBool() { return val != 0; }

		virtual JsonObject* clone() { return new JsonDouble(val); }

		virtual void toXlOper(LPXLOPER12 data);

		virtual XlOperTable* toTable();
	};

	class RMS_EXPORT JsonString : public JsonObject {
		std::string val;
	public:
		JsonString(const char *val) : val(val) {}
		JsonString(const std::string& val) : val(val) {}

		virtual void add(JsonObject *child) { internalError(); }

		virtual void write(std::ostream &out);

		virtual std::string toString() { return val; }

		virtual int toInt();

		virtual double toDouble();

		virtual bool toBool() { return val != "TRUE"; }

		virtual bool isEmpty() { return val.empty(); }

		virtual JsonObject* clone() { return new JsonString(val); }

		virtual void toXlOper(LPXLOPER12 data);

		virtual XlOperTable* toTable();
	};

	enum JsonIdType {
		valTRUE, valFALSE, valNULL, valERR
	};

	class RMS_EXPORT JsonId : public JsonObject {
		JsonIdType val;
	public:
		JsonId(JsonIdType val) : val(val) {}

		JsonId(const char *val);

		virtual void add(JsonObject *child) { internalError(); }

		virtual void write(std::ostream &out);

		virtual std::string toString();

		virtual int toInt() { internalError(); }

		virtual double toDouble() { internalError(); }

		virtual bool toBool() { return val == valTRUE; }

		virtual bool isEmpty() { return val == valNULL; }

		virtual JsonList *toList() {
			if(val == valNULL) return NULL;
			throw std::exception();
		}

		virtual JsonMap *toMap() {
			if (val == valNULL) return NULL;
			throw std::exception();
		}

		virtual std::vector<std::string> toStringList();

		virtual JsonObject* clone() { return new JsonId(val); }

		virtual void toXlOper(LPXLOPER12 data);

		virtual XlOperTable* toTable();
	};

	class RMS_EXPORT JsonList : public JsonObject {
		JsonObject *parent;
		std::vector<JsonObject *> val;
	public:
		JsonList(JsonObject *parent) : parent(parent) {}

		~JsonList();

		virtual void add(JsonObject *child) { val.push_back(child); }

		virtual void write(std::ostream &out);

		virtual JsonObject *getParent() { return parent; }

		virtual std::string toString() { throw std::exception(); }

		virtual int toInt() { throw std::exception(); }

		virtual double toDouble() { throw std::exception(); }

		virtual bool toBool() { throw std::exception(); }

		virtual size_t size() { return val.size(); }

		virtual JsonObject *get(int pos) { return val[pos]; }

		virtual JsonList *toList() { return this; }

		virtual std::vector<std::string> toStringList();

		virtual bool isEmpty() { return val.empty(); }

		virtual JsonObject* clone();

		virtual void toXlOper(LPXLOPER12 data);

		virtual XlOperTable* toTable();
	};

	typedef std::map<std::string, JsonObject *> JsonObjectByNames;
	typedef std::vector<std::string> InsertOrder;

	class RMS_EXPORT JsonMap : public JsonObject {
		JsonObject *parent;
		JsonString *key;
		JsonObjectByNames val;
		InsertOrder order;
	public:
		JsonMap(JsonObject *parent) : parent(parent), key(NULL) {}

		~JsonMap();

		virtual void add(JsonObject *child);

		virtual void write(std::ostream &out);

		virtual JsonObject *getParent() { return parent; }

		JsonObject *get(const std::string &name);

		void put(const std::string& name, JsonObject* v) { order.push_back(name);  val[name] = v; }

		void remove(const std::string& name) { val.erase(name); }

		std::string getString(const std::string &name) { return get(name)->toString(); }

		bool getBool(const std::string &name) { return get(name)->toBool(); }

		int getInt(const std::string &name) { return get(name)->toInt(); }

		double getDouble(const std::string &name) { return get(name)->toDouble(); }

		int getEnum(const std::string &name, const char** vals);

		JsonObjectByNames::iterator begin() {
			return val.begin();
		}

		JsonObjectByNames::iterator end() {
			return val.end();
		}

		InsertOrder::iterator beginAsInserted() {
			return order.begin();
		}

		InsertOrder::iterator endAsInserted() {
			return order.end();
		}

		virtual std::string toString() { internalError(); }

		virtual int toInt() { internalError(); }

		virtual double toDouble() { internalError(); }

		virtual bool toBool() { internalError(); }

		virtual JsonMap *toMap() { return this; }

		virtual bool isEmpty() { return val.empty(); }

		virtual JsonObject* clone();

		virtual void toXlOper(LPXLOPER12 data);

		virtual XlOperTable* toTable();

		virtual void finish();

		virtual bool has(const std::string& name) { return val.find(name) != val.end(); }
	};
}
#endif //XLL_JSONOBJECT_H
