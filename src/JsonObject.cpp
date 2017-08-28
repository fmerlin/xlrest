//
// Created by IBD533 on 27/08/2016.
//
#include "stdafx.h"
#include <cstdlib>
#include <cstdarg>
#include "JsonObject.h"
#include <cassert>
#include <algorithm>
#include "xlcall.h"
#include "Parameter.h"
#include "XlOperTable.h"

namespace rms {

#define parseError(A) { throw std::exception("Not json data, " A); }

	std::string toUpper(const std::string & name)
	{
		std::string name_upper(name);
		std::transform(name.begin(), name.end(), name_upper.begin(), ::toupper);
		return name_upper;
	}


	void Tokenizer::nextNumber() {
		while (pos < max) {
			unsigned char c = buffer[pos];
			if (c == '.' || c == 'e' || c == 'E' || c == '-')
			{
				mode = 2;
				pos++;
			}
			else if (c >= '0' && c <= '9')
				pos++;
			else {
				token[sz] = 0;
				if (mode == 1)
					add(new JsonInt(atoi(token)));
				else
					add(new JsonDouble(atof(token)));
				mode = 0;
				return;
			}
			if (sz < TOKEN_BUFFER_SZ)
				token[sz++] = c;
		}
	}

	int fromHexa(unsigned char c) {
		if (c >= '0' && c <= '9')
			return c - '0';
		if (c >= 'A' && c <= 'F')
			return c - 'A';
		if (c >= 'a' && c <= 'f')
			return c - 'a';
		parseError("hexacode convertion error")
	}

	void Tokenizer::nextString() {
		while (pos < max) {
			unsigned char c = buffer[pos++];
			if (mode == 4) {
				mode = 3;
				if (sz < TOKEN_BUFFER_SZ) {
					switch (c) {
					case '\\':
					case '\'':
					case '/':
					case '"':
						token[sz++] = c;
						break;
					case 'b':
						token[sz++] = '\b';
						break;
					case 't':
						token[sz++] = '\t';
						break;
					case 'n':
						token[sz++] = '\n';
						break;
					case 'f':
						token[sz++] = '\f';
						break;
					case 'r':
						token[sz++] = '\r';
						break;
					case 'u':
						{
							if (pos + 4 >= max)
								parseError("buffer too small");
							int c1 = fromHexa(buffer[pos++]);
							int c2 = fromHexa(buffer[pos++]);
							if (c1 != 0 || c2 != 0)
								parseError("unsupported unicode text")
								int c3 = fromHexa(buffer[pos++]);
							int c4 = fromHexa(buffer[pos++]);
							token[sz++] = c4 | c3 << 8;
						}
						break;
					default:
						parseError("unknown escape character")
					}
				}
			}
			else if (c == '\\')
				mode = 4;
			else if (c == quote)
			{
				token[sz] = 0;
				mode = 0;
				add(new JsonString(token));
				return;
			}
			else if(sz < TOKEN_BUFFER_SZ)
				token[sz++] = c;
		}
	}

	void Tokenizer::nextId() {
		while (pos < max) {
			unsigned char c = buffer[pos];
			if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9' || c == '_') {
				if(sz < TOKEN_BUFFER_SZ)
					token[sz++] = (char)tolower(c);
			} else {
				token[sz] = 0;
				mode = 0;
				add(new JsonId(token));
				return;
			}
			pos++;
		}
	}

	void Tokenizer::nextText() {
		if (buffer[0] == 0) {
			token[sz++] = 0;
			pos++;
			add(new JsonString(token));
		} else
			while (pos < max && sz < TOKEN_BUFFER_SZ)
				token[sz++] = buffer[pos++];
	}

	void Tokenizer::nextObject() {
		sz = 0;
		while (pos < max) {
			unsigned char c = buffer[pos++];
			switch (c) {
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			case '\f':
			case '\0':
				break;
			case '-':
				mode = 1;
				token[sz++] = c;
				return;
			case '.':
				mode = 2;
				token[sz++] = c;
				return;
			case ':':
				assert(mode == 0);
				break;
			case ',':
				if(parent != NULL)
					parent->finish();
				assert(mode == 0);
				break;
			case '"':
			case '\'':
				quote = c;
				mode = 3;
				return;
			case '{':
				parent = add(new JsonMap(parent));
				break;
			case '}':
				if (parent != NULL) {
					parent->finish();
					parent = parent->getParent();
				}
				break;
			case '[':
				parent = add(new JsonList(parent));
				break;
			case ']':
				if (parent != NULL) {
					parent->finish();
					parent = parent->getParent();
				}
				break;
			case '_':
				mode = 5;
				token[sz++] = c;
				return;
			default:
				if (c >= '0' && c <= '9')
					mode = 1;
				if (c >= 'a' && c <= 'z')
					mode = 5;
				if (c >= 'A' && c <= 'Z')
					mode = 5;
				if (mode != 0) {
					token[sz++] = (char)tolower(c);
					return;
				}
				parseError("not json text")
			}
		}
	}

	Tokenizer::~Tokenizer() {
//		if (root != NULL)
//			delete root;
	}

	JsonObject* Tokenizer::add(JsonObject* obj) {
		if (root == NULL)
			root = obj;
		if (parent == NULL)
			parent = obj;
		else
			parent->add(obj);
		return obj;
	}

	void Tokenizer::next() {
		pos = 0;
		while (pos < max) {
			switch (mode) {
			case -1:
				nextText();
				break;
			case 0:
				nextObject();
				break;
			case 1:
			case 2:
				nextNumber();
				break;
			case 3:
			case 4:
				nextString();
				break;
			case 5:
				nextId();
				break;
			}
		}
	}

	JsonObject* JsonObject::get(const std::string &name) {
		return NOTHING;
	}

	void JsonInt::write(std::ostream &out) {
		out << val;
	}

	std::string JsonInt::toString() {
		char buf[32];
		snprintf(buf, 31, "%d", val);
		return std::string(buf);
	}

	void JsonInt::toXlOper(LPXLOPER12 data)
	{
		data->xltype = xltypeInt;
		data->val.w = val;
	}

	std::string JsonDouble::toString() {
		char buf[32];
		snprintf(buf,31,"%f",val);
		return std::string(buf);
	}

	void JsonDouble::write(std::ostream &out) {
		out << val;
	}

	void JsonDouble::toXlOper(LPXLOPER12 data)
	{
		data->xltype = xltypeNum;
		data->val.num = val;
	}

	void JsonString::write(std::ostream &out) {
		out << '"';
		for (const char* c = val.c_str(); *c; c++) {
			switch (*c) {
			case '"':
				out << "\\\"";
				break;
			default:
				out << *c;
				break;
			}
		}
		out << '"';
	}

	int JsonString::toInt() {
		return atoi(val.c_str());
	}

	double JsonString::toDouble() {
		return atof(val.c_str());
	}

	static bool check(const std::string& val, const std::string& pattern) {
		if (val.size() < pattern.size())
			return false;
		for (int i = 0; i < pattern.size(); i++) {
			char c = val[i];
			if (pattern[i] == 'D') {
				if (c < '0' || c > '9')
					return false;
			}
			else if (c != pattern[i])
				return false;
		}
		return true;
	}

	void JsonString::toXlOper(LPXLOPER12 data)
	{
		if (check(val, "DDDD-DD-DDTDD:DD:DD"))
			dateToXlOper(val, data);
		else if (check(val, "DDDD-DD-DD"))
			dateToXlOper(val, data);
		else if (val.size() > 0)
			strToXlOper(val, data);
		else
			data->xltype = xltypeMissing;
	}

	JsonId::JsonId(const char *v) {
		std::string id(v);
		std::string name_upper(id);
		std::transform(id.begin(), id.end(), name_upper.begin(), ::tolower);
		if (name_upper == "true")
			val = valTRUE;
		else if (name_upper == "false")
			val = valFALSE;
		else if (name_upper == "null")
			val = valNULL;
		else if (name_upper == "nan")
			val = valNULL;
		else
			parseError("not json text")
	}

	void JsonId::write(std::ostream &out) {
		switch (val) {
		case valTRUE:
			out << "true";
			break;
		case valFALSE:
			out << "false";
			break;
		case valNULL:
			out << "null";
			break;
		}
	}

	std::vector<std::string> JsonId::toStringList() {
		std::vector<std::string> res;
		return res;
	}

	std::string JsonId::toString() {
		switch (val) {
		case valNULL:
			return "";
		case valTRUE:
			return "true";
		case valFALSE:
			return "false";
		case valERR:
			return "#N/A";
		}
		return "";
	}

	void JsonId::toXlOper(LPXLOPER12 data) {
		switch (val) {
		case valNULL:
			strToXlOper("", data);
			break;
		case valTRUE:
			data->xltype = xltypeBool;
			data->val.xbool = true;
			break;
		case valFALSE:
			data->xltype = xltypeBool;
			data->val.xbool = false;
			break;
		case valERR:
			data->xltype = xltypeErr;
			break;
		}
	}

	void JsonList::write(std::ostream &out) {
		out << '[';
		for (size_t i = 0; i < val.size(); i++) {
			if (i > 0)
				out << ',';
			val[i]->write(out);
		}
		out << ']';
	}

	JsonList::~JsonList() {
		for (size_t i = 0; i < val.size(); i++)
			delete val[i];
	}

	void JsonMap::add(JsonObject *child) {
		if (key == NULL)
			key = (JsonString *)child;
		else {
			val[key->toString()] = child;
			order.push_back(key->toString());
			delete key;
			key = NULL;
		}
	}

	void JsonMap::write(std::ostream &out) {
		out << '{';
		bool first = true;
		for (InsertOrder::const_iterator i = order.begin(); i != order.end(); i++) {
			const std::string & key = *i;
			JsonObjectByNames::const_iterator pair = val.find(key);
			if (first)
				first = false;
			else
				out << ',';
			out << '"' << pair->first << '"' << ':';
			pair->second->write(out);
		}
		out << '}';
	}

	JsonMap::~JsonMap() {
		if (key)
			delete key;
		for (std::map<std::string, JsonObject*>::iterator i = val.begin(); i != val.end(); i++)
			delete i->second;
	}

	extern JsonId* NOTHING = new JsonId(valNULL);

	JsonObject *JsonMap::get(const std::string &name) {
		JsonObjectByNames::iterator it = val.find(name);
		if (it == val.end())
			return NOTHING;
		return it->second;
	}

	int JsonMap::getEnum(const std::string &name, const char** vals) {
		int n = 0;
		std::string name_upper = toUpper(name);
		while (vals[n] != NULL) {
			if (name_upper == vals[n])
				return n;
			n++;
		}
		return -1;
	}

	std::vector<std::string> JsonList::toStringList() {
		std::vector<std::string> res;
		for (size_t i = 0; i < val.size(); i++)
			res.push_back(val[i]->toString());
		return res;
	}


	JsonObject* JsonList::clone() {
		JsonList* res = new JsonList(NULL);
		for (size_t i = 0; i < val.size(); i++)
			res->add(val[i]->clone());
		return res;
	}

	JsonObject* JsonMap::clone() {
		JsonMap* res = new JsonMap(NULL);
		for (std::map<std::string, JsonObject*>::iterator i = val.begin(); i != val.end(); i++)
			res->put(i->first,i->second->clone());
		return res;
	}

	void JsonList::toXlOper(LPXLOPER12 data) {
		XlOperTable* t = toTable();
		if (t != NULL) {
			t->toXlOper(data);
			delete t;
		}
	}

	void JsonMap::finish() {
		if (key != NULL) {
			val[key->toString()] = new JsonId(valNULL);
			order.push_back(key->toString());
			delete key;
			key = NULL;
		}
	}

	void JsonMap::toXlOper(LPXLOPER12 data) {
		XlOperTable* t = toTable();
		if (t != NULL) {
			t->toXlOper(data);
			delete t;
		}
	}

	XlOperTable* JsonMap::toTable() {
		XlOperTable* res = NULL;
		for (InsertOrder::iterator i = order.begin(); i != order.end(); i++) {
			XlOperTable* t = val[*i]->toTable();
			if (t != NULL) {
				t->setName(*i);
				if (res == NULL)
					res = t;
				else {
					res->join(t);
					delete t;
				}
			}
		}
		return res;
	}

	XlOperTable* JsonList::toTable() {
		XlOperTable* res = NULL;
		for (size_t i = 0; i < val.size(); i++) {
			XlOperTable* t = val[i]->toTable();
			if (t != NULL) {
				if (res == NULL)
					res = t;
				else {
					res->append(t);
					delete t;
				}
			}
		}
		return res;
	}

	XlOperTable* JsonId::toTable() {
		return new XlOperTable(new XlOperColumn(this));
	}

	XlOperTable* JsonInt::toTable() {
		return new XlOperTable(new XlOperColumn(this));
	}

	XlOperTable* JsonDouble::toTable() {
		return new XlOperTable(new XlOperColumn(this));
	}

	XlOperTable* JsonString::toTable() {
		return new XlOperTable(new XlOperColumn(this));
	}
}
