#include "stdafx.h"
#include "XlOperTable.h"
#include "Parameter.h"
#include "JsonObject.h"

#include <sstream>
#include <assert.h>

namespace rms {

	XlOperTable::XlOperTable() {}

	XlOperTable::~XlOperTable()
	{
		for (XlOperColumns::const_iterator it = data.begin(); it != data.end(); it++)
			if (it->second->parent == this) {
				it->second->parent = NULL;
				delete it->second;
			}
		data.clear();
	}

	XlOperTable::XlOperTable(XlOperColumn* ptr) { data[""] = ptr; ptr->parent = this; }

	void XlOperTable::detach() { data.clear(); }

	XlOperColumns::const_iterator XlOperTable::begin() const { return data.begin(); }
	XlOperColumns::const_iterator XlOperTable::end() const { return data.end(); }


	XlOperColumn::XlOperColumn(XlOperTable* parent) : parent(parent) {}
	XlOperColumn::XlOperColumn(JsonObject* ptr) { data.push_back(ptr); }

	void XlOperColumn::append(XlOperColumn* item) {
		data.insert(data.end(), item->data.begin(), item->data.end());
		after.insert(item->after.begin(), item->after.end());
	}

	JsonId JsonNull(valNULL);

	void XlOperColumn::increase(int n) {
		for (int i = 0; i < n; i++)
			data.push_back(&JsonNull);
	}

	void XlOperColumn::mul(int n) {
		int p = data.size();
		for (int i = 1; i < n; i++)
			data.insert(data.end(), data.begin(), data.begin() + p);
	}

	void XlOperColumn::clean() {
		col = 0;
		if (after.size() > 1) {
			std::set<std::string> indirect;
			for (std::set<std::string>::const_iterator it = after.begin(); it != after.end(); it++) {
				XlOperColumn* col = parent->get(*it);
				if (col != NULL)
					col->collect(indirect);
			}
			for(std::set<std::string>::const_iterator it = indirect.begin(); it != indirect.end(); it++)
				after.erase(*it);
		}
	}

	void XlOperColumn::collect(std::set<std::string> & cols) {
		if(!after.empty())
			for (std::set<std::string>::const_iterator it = after.begin(); it != after.end(); it++)
				if (cols.insert(*it).second) {
					XlOperColumn* col = parent->get(*it);
					if(col != NULL)
						col->collect(cols);
				}
	}

	void XlOperTable::append(XlOperTable* item) {
		int n = item->size();
		int m = size();
		for (XlOperColumns::iterator it = data.begin(); it != data.end(); it++) {
			XlOperColumns::iterator it2 = item->data.find(it->first);
			if (it2 != item->data.end())
				it->second->append(it2->second);
			else
				it->second->increase(n);
		}
		for (XlOperColumns::iterator it = item->data.begin(); it != item->data.end(); it++) {
			XlOperColumns::iterator it2 = data.find(it->first);
			if (it2 == data.end()) {
				XlOperColumn* col = new XlOperColumn(this);
				data[it->first] = col;
				col->increase(m);
				col->append(it->second);
			}
		}
	}

	static std::string toStr(int n) {
		std::ostringstream ot;
		ot << n;
		return ot.str();
	}

	static std::string findNewName(XlOperColumns& cols, const std::string& name, int n) {
		std::string key = n > 1 ? name + toStr(n) : name;
		if (cols.find(key) != cols.end())
			findNewName(cols, name, n + 1);
		else
			return key;
	}

	void XlOperTable::renameRef(const std::string& oldname, const std::string& newname) {
		for (XlOperColumns::iterator it = data.begin(); it != data.end(); it++)
			it->second->rename(oldname,newname);
	}

	void XlOperColumn::rename(const std::string& oldname, const std::string& newname) {
		if (after.find(oldname) != after.end()) {
			after.erase(oldname);
			after.insert(newname);
		}
	}

	void XlOperColumn::renameRef(const std::string& oldname, const std::string& newname) { parent->renameRef(oldname, newname); }

	void XlOperTable::join(XlOperTable* item) {
		int n = item->size();
		int m = size();
		assert(n > 0 && m > 0);
		if(m == 1)
			mul(n);
		else if(n == 1)
			item->mul(m);
		else if(n < m)
			item->increase(m);
		else
			increase(n);
		for (XlOperColumns::iterator it = item->data.begin(); it != item->data.end(); it++)
			if (data.find(it->first) != data.end())
				item->renameRef(it->first, findNewName(data, it->first, 2));
		for (XlOperColumns::iterator it = item->data.begin(); it != item->data.end(); it++) {
			for (XlOperColumns::iterator it2 = data.begin(); it2 != data.end(); it2++)
				it2->second->after.insert(it->first);
			it->second->parent = this;
			data[it->first] = it->second;
		}
		item->detach();
	}

	void XlOperTable::mul(int n) {
		for (XlOperColumns::iterator it = data.begin(); it != data.end(); it++)
			it->second->mul(n);
	}

	int XlOperTable::size() {
		return data.begin()->second->data.size();
	}

	void XlOperColumn::incCol() {
		col++;
		for (std::set<std::string>::iterator it = after.begin(); it != after.end(); it++)
			parent->get(*it)->incCol();
	}

	void XlOperTable::increase(int n) {
		for (XlOperColumns::iterator it = data.begin(); it != data.end(); it++)
			it->second->increase(n);
	}

	void XlOperTable::toXlOper(LPXLOPER12 ptr) {
		int nbrows = size();
		int nbcols = data.size();
		bool headers = nbcols > 1 || data.begin()->first == "";
		clean();
		incCol();
		ptr->xltype = xltypeMulti;
		ptr->val.array.columns = nbcols;
		ptr->val.array.rows = headers ? nbrows + 1: nbrows;
		ptr->val.array.lparray = (XLOPER12*) malloc((nbrows + 1) * nbcols * sizeof(XLOPER12));
		for (XlOperColumns::iterator it = data.begin(); it != data.end(); it++) {
			int col = it->second->col - 1;
			assert(col < nbcols);
			int offset = col;
			if (headers) {
				strToXlOper(it->first, &ptr->val.array.lparray[col]);
				offset += nbcols;
			}
			for (int row = 0; row < nbrows; row++) {
				LPXLOPER12 l = &ptr->val.array.lparray[row * nbcols + offset];
				it->second->data[row]->toXlOper(l);
			}
		}
	}

	void XlOperTable::clean() {
		for (XlOperColumns::iterator it = data.begin(); it != data.end(); it++)
			it->second->clean();
	}

	void XlOperTable::incCol() {
		for (XlOperColumns::iterator it = data.begin(); it != data.end(); it++)
			it->second->incCol();
	}

	void XlOperTable::setName(const std::string& val) {
		if (data.size() == 1 && data.begin()->first == "") {
			XlOperColumn* col = data.begin()->second;
			data.clear();
			data[val] = col;
		}
	}

	XlOperColumn* XlOperTable::get(const std::string& name) {
		XlOperColumns::iterator it = data.find(name);
		return it != data.end() ? it->second : NULL;
	}
}