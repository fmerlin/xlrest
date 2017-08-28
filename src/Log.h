/*******************************************************************************
 * This program and the accompanying materials
 * are made available under the terms of the Common Public License v1.0
 * which accompanies this distribution, and is available at 
 * http://www.eclipse.org/legal/cpl-v10.html
 * 
 * Contributors:
 *     Peter Smith
 *******************************************************************************/

#ifndef LOG_H
#define LOG_H

#define MAX_LOG_LENGTH 4096

#include "Declarations.h"
#include <string>

namespace rms {

	enum LoggingLevel { debug = -1, info = 0, warning = 1, error = 2};

	class Log {

		BOOL haveInit = FALSE;
		BOOL canUseConsole = FALSE;
		BOOL haveConsole = FALSE;
		HANDLE g_logfileHandle = NULL;
		HANDLE g_stdHandle = NULL;
		bool g_haveLogFile = false;
		bool g_logFileAndConsole = false;
		double g_logRollSize = 0;
		wchar_t g_logFilename[MAX_PATH];
		bool g_logOverwrite = false;
		volatile bool g_logRolling = false;
		LoggingLevel g_logLevel = error;
		bool g_error = false;
		char g_errorText[MAX_PATH];
		HttpServer* server;

	public:
		virtual ~Log();
		void Init(HINSTANCE hInstance, const std::string & logfile, const std::string & loglevel);
		void SetLevel(LoggingLevel level);
		void SetLogFileAndConsole(bool logAndConsole);
		LoggingLevel GetLevel();
		void Debug(const char* format, ...);
		void Info(const char* format, ...);
		void Warning(const char* format, ...);
		void Error(const char* format, ...);
		void Error(const std::exception& e);
		void Close();
		void LogIt(LoggingLevel loggingLevel, const char* marker, const char* format, va_list args);
		void setServer(HttpServer* server) { this->server = server; }

	private:
		void RollLog();
	};

	extern Log * logger;

}
#endif // LOG_H