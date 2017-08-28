//
// Created by IBD533 on 29/08/2016.
//

#ifndef XLL_DECLARATIONS_H_H
#define XLL_DECLARATIONS_H_H

#define APPNAME "xlrest"
#define _CRT_SECURE_NO_WARNINGS
#ifdef _DEBUG
#define RMS_EXPORT __declspec(dllexport)
#else
#define RMS_EXPORT
#endif
#include <string>

#include <vector>
#include <map>

namespace rms {
	class Parameter;
	class Server;
	class HttpServer;
	class Config;
	class Operation;
	class JsonObject;
	class JsonMap;
	class JsonList;
	class JsonId;
	class JsonString;
	class JsonInt;
	class JsonDouble;
	class ObjectDefinition;
	class XlOperTable;
	class XlOperColumn;

	#define internalError() { throw std::exception("Internal Error"); }

	inline std::wstring xToWstring(const std::string& txt) {
		return std::wstring(txt.begin(), txt.end());
	}

	inline std::string xToString(const std::wstring& txt) {
		return std::string(txt.begin(), txt.end());
	}


	template<typename charT>
	struct my_equal {
		my_equal(const std::locale& loc) : loc_(loc) {}
		bool operator()(charT ch1, charT ch2) {
			return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
		}
	private:
		const std::locale& loc_;
	};

	// find substring (case insensitive)
	template<typename T>
	int ci_find_substr(const T& str1, const T& str2, const std::locale& loc = std::locale())
	{
		typename T::const_iterator it = std::search(str1.begin(), str1.end(),
			str2.begin(), str2.end(), my_equal<typename T::value_type>(loc));
		if (it != str1.end()) return it - str1.begin();
		else return -1; // not found
	}

}

#endif //XLL_DECLARATIONS_H_H
