
#include "netCap.h"

PcapReader::PcapReader() {
	pcap_if_t* devptr = nullptr;
	char errBuf[PCAP_ERRBUF_SIZE];

	if (pcap_findalldevs(&devptr, errBuf) < 0)
	{
		std::cout << "Error in pcap_findalldevs:" << errBuf << "\n";
		return;
	}

	this->pAllDev = devptr;
	int devCnt = 0;
	for (pcap_if_t* d = devptr; d; d = d->next) {
		printf("%d :  %s\n", devCnt, (d->description) ? (d->description) : (d->name));
		this->mDevice[devCnt] = d;
		devCnt++;
	}
}
PcapReader::~PcapReader() {

}

bool PcapReader::init(int devNum, const std::string& filter) {
	if (this->mDevice[devNum] == nullptr) {
		printf("no devie %d\n", devNum);
		return false;
	}

	pcap_if_t* pDevice = this->mDevice[devNum];
	char errBuf[PCAP_ERRBUF_SIZE];
	if (!(this->pcapHandle = pcap_open_live(pDevice->name, MAXLEN_PCAP, PCAP_OPENFLAG_PROMISCUOUS, MAXTIME_PCAPREAD, errBuf))) {
		printf("pcap_open_live error %s\n", pDevice->name);
		this->free();
		return false;
	}
	if (pcap_datalink(this->pcapHandle) != DLT_EN10MB)
	{
		printf("This program works only on Ethernet networks.\n");
		this->free();
		return false;
	}

	pcap_setbuff(this->pcapHandle, (1024 * 1024 * 10));

	uint32_t netMask = 0xffffff;

	//if (pDevice->addresses != nullptr) {
	//	netMask = (uint32_t)(((struct sockaddr_in*)(pDevice->addresses->netmask))->sin_addr.S_un.S_addr);
	//	netMask = 0xffffff;
	//	u_int unNetwork, unNetmask;
	//	int nResult = pcap_lookupnet(pDevice->name, &unNetwork, &netMask, errBuf);
	//	if (pcap_lookupnet(pDevice->name, &unNetwork, &netMask, errBuf) == -1)
	//	{
	//		printf("pcap_lookupnet adapter[%s], %s\n", pDevice->name, errBuf);
	//		netMask = 0xffffff;
	//	}
	//	else {
	//		struct in_addr net_addr, mask_addr;
	//		net_addr.s_addr = unNetwork;
	//		char* pNet = inet_ntoa(net_addr);
	//		char szNet[16];
	//		memset(szNet, 0, sizeof(szNet));
	//		sprintf(szNet, "%s", pNet);
	//		unNetmask = 0xffffff;
	//		mask_addr.s_addr = unNetmask;
	//		char* pMask = inet_ntoa(mask_addr);
	//		char szMask[16];
	//		memset(szMask, 0, sizeof(szMask));
	//		sprintf(szMask, "%s", pMask);
	//		printf("OpenAdapter [%s]: Netmask [%s]  Network [%s]\n", pDevice->name, szMask, szNet);
	//	}
	//}

	if (filter.length() > 0) {
		struct bpf_program fCode;

		printf("compile the filter (%s)\n", filter.c_str());

		// compile the filter
		if (pcap_compile(this->pcapHandle, &fCode, filter.c_str(), 1, netMask) < 0)
		{
			printf("Unable to compile the packet filter. Check the syntax...,%s\n", filter.c_str());
			return false;
		}
		//set the filter
		if (pcap_setfilter(this->pcapHandle, &fCode) < 0)
		{
			printf("Error setting the filter.\n");
			return false;
		}
	}

	return true;
}

void PcapReader::free() {
	if (this->pAllDev)
	{
		int i;
		pcap_freealldevs(this->pAllDev);
		this->pAllDev = nullptr;
		for (i = 0; i < MAXCNT_NIC; i++) {
			this->mDevice[i] = nullptr;
		}
	}
}

bool PcapReader::start(void (*onCapture)(const pcap_pkthdr*, const uint8_t*)) {
	printf("Thread START\n");
	this->pThread.reset(new boost::thread(boost::bind(&PcapReader::run, this, onCapture)));
}

void PcapReader::run(void (*onCapture)(const pcap_pkthdr*, const uint8_t*)) {
	int res;
	struct pcap_pkthdr* header;
	const u_char* pkt_data;

	if (this->pcapHandle == nullptr) {
		return;
	}

	int errCount = 0;
	this->runThd = true;
	while (this->runThd)
	{
		res = pcap_next_ex(this->pcapHandle, &header, &pkt_data);
		if (res < 0) {
			if (errCount >= 10) {
				printf("Thread STOP. StopCap\n");
				Sleep(2000);
				this->stop();
			}
			errCount++;

			printf("PCAP ERROR. cnt:%d ret:%d\n", errCount, res);
		}
		else if (res == 0) {
			Sleep(0);
		}
		else {
			onCapture(header, pkt_data);
		}		
	}
	printf("Pcap Thread Out\n");
}
void PcapReader::stop() {
	this->runThd = false;

	if (this->pcapHandle) {
		pcap_close(this->pcapHandle);
		this->pcapHandle = nullptr;
		printf("Packet Capture Stopped <<<.\n");
	}
}