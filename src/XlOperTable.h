#pragma once

#include <set>
#include <vector>
#include <map>
#include "xlcall.h"
#include "Declarations.h"

namespace rms {
	class RMS_EXPORT XlOperColumn {
		friend class XlOperTable;
		XlOperTable* parent;
		std::set<std::string> after;
		std::vector<JsonObject*> data;
		int col;
	public:
		XlOperColumn(XlOperTable* parent);
		XlOperColumn(JsonObject* ptr);
		void append(XlOperColumn* item);
		void increase(int n);
		void mul(int n);
		void clean();
		void collect(std::set<std::string>& cols);
		void incCol();
		int getCol() { return col; }
		void rename(const std::string& oldname, const std::string& newname);
		void renameRef(const std::string& oldname, const std::string& newname);
	};

	typedef std::map<std::string, XlOperColumn*> XlOperColumns;

	class RMS_EXPORT XlOperTable {
		XlOperColumns data;
	public:
		XlOperTable();
		XlOperTable(XlOperColumn* ptr);
		~XlOperTable();

		void join(XlOperTable* item);
		void append(XlOperTable* item);
		void toXlOper(LPXLOPER12 data);
		int size();
		void mul(int n);
		void setName(const std::string& val);
		void increase(int n);
		void clean();
		void incCol();
		void detach();

		XlOperColumn* get(const std::string& name);
		XlOperColumns::const_iterator begin() const;
		XlOperColumns::const_iterator end() const;
		void renameRef(const std::string& oldname, const std::string& newname);
	};
}