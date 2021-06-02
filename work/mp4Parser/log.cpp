#include "log.h"

using namespace LOG;

logManager::logManager(const std::string& logFile) : pThread()
{
	this->logFile_ = logFile;
	this->start();
}
logManager::~logManager() {
	this->stop();
}
void logManager::log(int nCod, const char* szFuntion, int nLine, const char* fmt, ...)
{
	char szBuff[2000 * 8];
	memset(szBuff, 0x00, sizeof(szBuff));

	char szBuff2[2000 * 8];
	memset(szBuff2, 0x00, sizeof(szBuff2));

	char ListTemp[500];
	memset(ListTemp, 0x00, sizeof(ListTemp));

	va_list ArgList;
	va_start(ArgList, fmt);
	vsnprintf(szBuff, sizeof(szBuff) - 1, fmt, ArgList);
	va_end(ArgList);

	switch (nCod)
	{
	case logCode::LOG_I:
		sprintf_s(ListTemp, "INFO");
		break;
	case logCode::LOG_W:
		sprintf_s(ListTemp, "WARNING");
		break;
	case logCode::LOG_E:
		sprintf_s(ListTemp, "ERROR");
		//printf("[%-8s][CH:%04d]-%s \n", ListTemp, nCh, szBuff);
		break;
	case logCode::LOG_D:
		sprintf_s(ListTemp, "DEBUG");

		break;
	case logCode::LOG_P:
		sprintf_s(ListTemp, "INFO");
		printf("[%-8s]-%s \n", ListTemp, szBuff);
		break;
	}

	sprintf_s(szBuff2, "[%-8s]-%s [%s():%d]", ListTemp, szBuff, szFuntion, nLine);

	stLOG* log = new stLOG;
	memset(log, 0, sizeof(stLOG));
	sprintf_s(log->buf, "%s", szBuff2);

	if (addLog(log) == false)
		delete log;
}

void logManager::logDump(int nCod, const char* pszFuncName, int nLine, char* dump, int dumpSize, const char* pszFormat, ...)
{
	char szBuff[2000 * 8];
	memset(szBuff, 0x00, sizeof(szBuff));
	char szBuff2[2000 * 8];
	memset(szBuff2, 0x00, sizeof(szBuff2));
	char ListTemp[500];
	memset(ListTemp, 0x00, sizeof(ListTemp));

	va_list ArgList;
	va_start(ArgList, pszFormat);
	vsnprintf(szBuff, sizeof(szBuff) - 1, pszFormat, ArgList);
	va_end(ArgList);

	switch (nCod)
	{
	case logCode::LOG_I:
		sprintf_s(ListTemp, "INFO");
		break;
	case logCode::LOG_W:
		sprintf_s(ListTemp, "WARNING");
		break;
	case logCode::LOG_E:
		sprintf_s(ListTemp, "ERROR");
		break;
	case logCode::LOG_D:
		sprintf_s(ListTemp, "DEBUG");
		break;
	case logCode::LOG_P:
		break;
	}

	char log[4096];
	memset(log, 0x00, sizeof(log));
	this->hexView((const unsigned char*)dump, dumpSize, log);
	log[sizeof(log) - 1] = 0;

	sprintf_s(szBuff2, "[%-8s]-%s %s", ListTemp, szBuff, log);

	stLOG* log2 = new stLOG;
	memset(log2, 0, sizeof(stLOG));
	sprintf_s(log2->buf, "%s", szBuff2);

	if (addLog(log2) == false)
		delete log2;
}
void logManager::start() {
	if (this->pThread) {
		return;
	}
	this->pThread.reset(new boost::thread(boost::bind(&logManager::run, this)));
}
void logManager::stop() {
	if (!this->pThread) {
		return;
	}
	this->pThread->join();
	this->pThread.reset();
}
void logManager::run()
{
	while (true)
	{
		stLOG* log = getLog();
		if (log == NULL)
		{
			Sleep(10);
		}
		else
		{
			this->logSave(log);
			delete log;
		}
		Sleep(0);
	}
}
bool logManager::addLog(stLOG* log)
{
	this->mutex.lock();

	if (this->logindex >= 10000)
	{
		stLOG* log2 = new stLOG;
		memset(log2, 0, sizeof(stLOG));
		sprintf_s(log2->buf, "%s", "LOG QUEUE FULL");
		this->arrLog[9999] = log2;
		mutex.unlock();
		return false;
	}

	this->arrLog[this->logindex++] = log;

	this->mutex.unlock();
	return true;
}
stLOG* logManager::getLog()
{
	this->mutex.lock();

	stLOG* ret;
	if (logindex < 1)
		ret = NULL;
	else
	{
		ret = arrLog[0];

		for (int i = 0; i < logindex; i++)
		{
			arrLog[i] = arrLog[i + 1];
		}

		logindex--;
	}

	this->mutex.unlock();

	return ret;
}

int logManager::logSave(stLOG* log)
{
	char Dbuffer[2000 * 8]; memset(Dbuffer, 0, sizeof(Dbuffer));
	char Mbuffer[260]; memset(Mbuffer, 0, sizeof(Mbuffer));

	boost::posix_time::ptime timeLocal = boost::posix_time::second_clock::local_time();
	
	int year = timeLocal.date().year();
	int month = timeLocal.date().month();
	int day = timeLocal.date().day();
	int hour = timeLocal.time_of_day().hours();
	int min = timeLocal.time_of_day().minutes();
	int sec = timeLocal.time_of_day().seconds();
	int ms = timeLocal.time_of_day().total_milliseconds();

	sprintf_s(Mbuffer, "%04d/%02d/%02d/%02d-%02d-%02d-%04d:", year, month, day, hour, min, sec, ms);
	sprintf_s(Dbuffer, "%s%s\n", Mbuffer, log->buf);


	std::ofstream stream;
	stream.open(this->logFile_, std::ios::out | std::ios::app);
	if (stream.is_open() == false) {
		stream.open(this->logFile_, std::ios::out);
		if (stream.is_open() == false) {
			return 0;
		}
	}

	stream.write(Dbuffer, strlen(Dbuffer));
	stream.flush();
	stream.close();

	return  1;
}
void logManager::hexView(const unsigned char* data, int length, char* log)
{
	char text[1024];

	const int hexCount = 16;  //한라인 16개씩 출력
	int index = 0;
	unsigned char hexToChar[17];
	unsigned char* hex = (unsigned char*)data;
	int i;
	for (i = 0; i < length; i++)
	{
		if ((i % hexCount) == 0)
		{
			if (i != 0)
			{
				memset(text, 0, sizeof(text));
				sprintf(text, "  %s\n", hexToChar);
				if (index + strlen(text) > 4000) return;
				sprintf(log + index, "%s", text);
				index += strlen(text);

			}

			memset(text, 0, sizeof(text));
			sprintf(text, "%04x", i);

			sprintf(log + index, "%s", text);
			index += strlen(text);

		}
		if ((i % 8) == 0)
		{
			memset(text, 0, sizeof(text));
			sprintf(text, "  %02x", hex[i]);
			if (index + strlen(text) > 4000) return;
			sprintf(log + index, "%s", text);
			index += strlen(text);

		}
		else
		{
			memset(text, 0, sizeof(text));
			sprintf(text, " %02x", hex[i]);
			if (index + strlen(text) > 4000) return;
			sprintf(log + index, "%s", text);
			index += strlen(text);

		}

		if ((hex[i] < 0x20) || (hex[i] > 0x7F)) //hex -> char 변환
		{
			hexToChar[i % hexCount] = '.';
		}
		else
		{
			hexToChar[i % hexCount] = hex[i];
		}
		hexToChar[(i % hexCount) + 1] = '\0';
	}


	while ((i % hexCount) != 0)
	{
		memset(text, 0, sizeof(text));
		sprintf(text, "   ");
		if (index + strlen(text) > 4000) return;
		sprintf(log + index, "%s", text);
		index += strlen(text);
		i++;
	}

	memset(text, 0, sizeof(text));
	sprintf(text, "    %s\n", hexToChar);
	if (index + strlen(text) > 4000) return;
	sprintf(log + index, "%s", text);
	index += strlen(text);
}