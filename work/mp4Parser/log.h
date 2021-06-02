#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <stdarg.h>
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>

typedef struct _stLOG {
	char buf[5000];
	char file[50];
}stLOG;

enum logCode {
	LOG_I = 0,
	LOG_W,
	LOG_E,
	LOG_D,
	LOG_P,
};

namespace LOG
{
	class logManager
	{
	private:
		boost::scoped_ptr<boost::thread> pThread;
		boost::mutex mutex;

		std::string logFile_;

		stLOG* arrLog[10000];
		int logindex;

		int logSave(stLOG* log);
		bool addLog(stLOG* log);
		stLOG* getLog();

		void start();
		void stop();
		void run();
	public:
		logManager(const std::string& logFile);
		~logManager();

		void log(int nCod, const char* szFuntion, int nLine, const char* fmt, ...);
		void logDump(int nCod, const char* pszFuncName, int nLine, char* dump, int dumpSize, const char* pszFormat, ...);
		
		void hexView(const unsigned char* data, int length, char* log);
	};
}