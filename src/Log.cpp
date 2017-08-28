/*******************************************************************************
 * This program and the accompanying materials
 * are made available under the terms of the Common Public License v1.0
 * which accompanies this distribution, and is available at 
 * http://www.eclipse.org/legal/cpl-v10.html
 * 
 * Contributors:
 *     Peter Smith
 *******************************************************************************/

#include "stdafx.h"
#include "Log.h"
#include <stdio.h>
#include "Declarations.h"
#include "HttpServer.h"
#include "JsonObject.h"
#include "Config.h"
#include <time.h>

namespace rms
{

	Log * logger;
	const char* levels[] = {"debug","info","warning","error"};

	void Log::Init(HINSTANCE hInstance, const std::string & logfile, const std::string & loglevel)
	{
		if (loglevel == "debug") {
			g_logLevel = debug;
		}
		else if (loglevel== "info") {
			g_logLevel = info;
		}
		else if (loglevel== "warning") {
			g_logLevel = warning;
		}
		else if (loglevel=="warn") {
			g_logLevel = warning;
		}
		else if (loglevel== "error") {
			g_logLevel = error;
		}
		else if (loglevel == "err") {
			g_logLevel = error;
		}
		else {
			g_logLevel = info;
			Warning("log.level unrecognized");
		}

		g_logfileHandle = INVALID_HANDLE_VALUE;

		// If there is a log file specified redirect std streams to this file
		if (logfile.length() > 0) {
			wchar_t defWorkingDir[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, defWorkingDir);
			g_logOverwrite = false;
			std::wstring wlogfile = xToWstring(logfile);
			ExpandEnvironmentStrings(wlogfile.c_str(), g_logFilename, MAX_PATH);
			g_logfileHandle = CreateFile(g_logFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
				g_logOverwrite ? CREATE_ALWAYS : OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
			if (g_logfileHandle != INVALID_HANDLE_VALUE) {
				SetFilePointer(g_logfileHandle, 0, NULL, g_logOverwrite ? FILE_BEGIN : FILE_END);
				g_stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
				SetStdHandle(STD_OUTPUT_HANDLE, g_logfileHandle);
				SetStdHandle(STD_ERROR_HANDLE, g_logfileHandle);
				g_haveLogFile = true;
				g_logFileAndConsole = true;
				// Check for log rolling
				g_logRollSize = 1000000;
				if (g_logRollSize > 0) {
					wchar_t fullLog[MAX_PATH];
					GetFullPathName(xToWstring(logfile).c_str(), MAX_PATH, fullLog, 0);
				}
			}
			else {
				g_logfileHandle = GetStdHandle(STD_OUTPUT_HANDLE);
				Error("Could not open log file");
			}
		}
		else {
			g_logfileHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		}
	}

	void Log::RollLog()
	{
		char suffix[MAX_PATH];
		SYSTEMTIME st;
		GetLocalTime(&st);
		sprintf(suffix, ".%4d%02d%02d-%02d%02d%02d", st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond);
		CloseHandle(g_logfileHandle);
		std::wstring logfilename_bak(g_logFilename);
		logfilename_bak += xToWstring(suffix);
		MoveFile(g_logFilename, logfilename_bak.c_str());
		g_logfileHandle = CreateFile(g_logFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
			g_logOverwrite ? CREATE_ALWAYS : OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (g_logfileHandle != INVALID_HANDLE_VALUE) {
			SetFilePointer(g_logfileHandle, 0, NULL, g_logOverwrite ? FILE_BEGIN : FILE_END);
		}
		Info("Rolled log name: %S", g_logFilename);
	}

	/*
	 * Log to console/file/debug monitor.
	 *
	 * Levels are { debug = -1, info = 0, warning = 1, error = 2 }
	 */
	void Log::LogIt(LoggingLevel loggingLevel, const char * marker, const char* format, va_list args)
	{
		if (g_logLevel > loggingLevel || loggingLevel > error) return;
		if (!format) return;

		time_t now;
		time(&now);

		char tmp[MAX_LOG_LENGTH];
		vsnprintf(tmp, MAX_LOG_LENGTH, format, args);

		SYSTEMTIME st;
		GetLocalTime(&st);

		char tmp2[MAX_LOG_LENGTH];
		snprintf(tmp2, MAX_LOG_LENGTH, "%02d:%02d:%02d %s %s\r\n", st.wHour, st.wMinute, st.wSecond, marker ? marker : "", tmp);

#ifdef _DEBUG
		OutputDebugStringA(tmp2);
#endif

		// Check if we need to roll the log
		if (g_logRollSize > 0 && !g_logRolling) {
			g_logRolling = true;
			DWORD size = GetFileSize(g_logfileHandle, 0);
			if (size > g_logRollSize)
				RollLog();
			g_logRolling = false;
		}

		DWORD dwRead;
		WriteFile(g_logfileHandle, tmp2, strlen(tmp2), &dwRead, NULL);
		FlushFileBuffers(g_logfileHandle);

#ifdef NDEBUG
		if (server && loggingLevel >= info) {
			try {

				std::auto_ptr<JsonMap> body(new JsonMap(NULL));
				JsonMap* event = new JsonMap(body.get());
				event->put("message", new JsonString(tmp));
				event->put("severity", new JsonString(levels[loggingLevel + 1]));
				event->put("username", new JsonString(config->getUserName()));
				event->put("sheetname", new JsonString(config->getSheetName()));
				event->put("session", new JsonString(config->getSessionId()));
				body->put("time", new JsonInt((int) now));
				body->put("host", new JsonString(config->getHostName()));
				body->put("index", new JsonString("main"));
				body->put("source", new JsonString("http:xlrest"));
				body->put("sourcetype", new JsonString("httpevent"));
				body->put("event", event);
				std::auto_ptr<JsonObject> obj(server->send("POST", server->getContext(), body.get(), true, true));
			}
			catch (std::exception& e) {
				if(e.what())
					printf(e.what());
				else
					printf(typeid(e).name());
			}
		}
#endif
	}

	void Log::SetLevel(LoggingLevel loggingLevel)
	{
		g_logLevel = loggingLevel;
	}

	LoggingLevel Log::GetLevel()
	{
		return g_logLevel;
	}

	void Log::SetLogFileAndConsole(bool logAndConsole)
	{
		g_logFileAndConsole = logAndConsole;
	}

	void Log::Debug(const char* format, ...)
	{
		if (g_logLevel <= debug) {
			va_list args;
			va_start(args, format);
			LogIt(debug, "[dbug]", format, args);
			va_end(args);
		}
	}

	void Log::Info(const char* format, ...)
	{
		if (g_logLevel <= info) {
			va_list args;
			va_start(args, format);
			LogIt(info, "[info]", format, args);
			va_end(args);
		}
	}

	void Log::Warning(const char* format, ...)
	{
		if (g_logLevel <= warning) {
			va_list args;
			va_start(args, format);
			LogIt(warning, "[warn]", format, args);
			va_end(args);
		}
	}

	void Log::Error(const std::exception& e) {
		if (e.what() != NULL)
			Error("%s", e.what());
		else
			Error("%s", typeid(e).name());
	}

	void Log::Error(const char* format, ...)
	{
		if (g_logLevel <= error) {
			va_list args;
			va_start(args, format);
			LogIt(error, "[err]", format, args);
			va_end(args);
		}
	}

	void Log::Close()
	{
		if (g_logfileHandle) {
			CloseHandle(g_logfileHandle);
			g_logfileHandle = NULL;
		}
	}

	Log::~Log() {
		if (server) {
			delete server;
			server = NULL;
		}
	}
}