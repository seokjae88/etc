#pragma once

#include <map>


#include "log.h"
extern LOG::logManager* logger;

enum SampleType {
	NONE = 0,
	AVC1,
	MP4A
};

class chunkInfo {
private:
	int sampleCnt = 0;
	int offset = 0;
	int index = 0;
public:
	std::map<int, int> sampleData;

	chunkInfo() {}
	void add(int num, int size) {
		if (this->sampleData[num]) {
			logger->log(logCode::LOG_E, __FUNCTION__, __LINE__, "aleady exist sample num[%d] size[%d->%d]", num, this->sampleData[num], size);
		}
		this->sampleData[num] = size;
	}
	int getSampleCnt() {
		return this->sampleCnt;
	}
	void setSampleCnt(int cnt) {
		this->sampleCnt = cnt;
	}
	int getOffset() {
		return this->offset;
	}
	void setOffset(int o) {
		this->offset = o;
	}
	int getIndex() {
		return this->index;
	}
	void setIndex(int idx) {
		this->index = idx;
	}

	void debugPrint() {
		logger->log(logCode::LOG_I, __FUNCTION__, __LINE__, "offset[%d], size[%d], index[%d], sampleCnt[%d]",
			this->offset, this->sampleData.size(), this->index, this-sampleCnt);
	}
};

class trak {
private:
	int sampleType = SampleType::NONE;
	int sampleCntTotal = 0;
	int sampleSizeTotal = 0;
	int chunkCnt = 0;
public:
	char sps[1024];
	int spsLength;
	char pps[1024];
	int ppsLength;
	std::map<int, chunkInfo*> chunkData;
	
	trak() {
		memset(sps, 0x0, sizeof(1024));
		memset(pps, 0x0, sizeof(1024));
	}
	void add(int num, chunkInfo* info) {
		if (this->chunkData[num]) {
			logger->log(logCode::LOG_E, __FUNCTION__, __LINE__, "aleady exist chunk num[%d]", num);
		}
		this->chunkData[num] = info;
	}
	chunkInfo* get(int num) {
		return this->chunkData[num];
	}
	int getChunkCnt() {
		if (this->chunkCnt != this->chunkData.size()) {
			logger->log(logCode::LOG_E, __FUNCTION__, __LINE__, "size[%d] / [%d]", this->chunkCnt, this->chunkData.size());
		}
		return this->chunkCnt;
	}
	void setChunkCnt(int cnt) {
		this->chunkCnt = cnt;
	}
	int getSampleCntTotal() {
		return this->sampleCntTotal;
	}
	void addSampleCnt(int cnt) {
		this->sampleCntTotal += cnt;
	}
	int getSampleSizeTotal() {
		return this->sampleSizeTotal;
	}
	void addSampleSize(int size) {
		this->sampleSizeTotal += size;
	}
	int getSampleType() {
		return this->sampleType;
	}
	void setSampleType(const std::string& type) {
		if (type == "avc1") {
			this->sampleType = SampleType::AVC1;
		}
		else if (type == "mp4a") {
			this->sampleType = SampleType::MP4A;
		}
		else {
			this->sampleType = SampleType::NONE;
		}
	}
};
