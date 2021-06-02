#pragma once
#include <pcap.h>
#include <iostream>
#include <string>
#include <map>

#include "math.h"
#include "stdint.h"

#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

#ifndef PCAP_OPENFLAG_PROMISCUOUS
#define PCAP_OPENFLAG_PROMISCUOUS		1
#endif

#define	DEFAULT_CAPTURE_FILE_SIZE	100 * 1024


#define	PCAP_MAGIC					0xa1b2c3d4
#define	PCAP_SWAPPED_MAGIC			0xd4c3b2a1
#define	PCAP_MODIFIED_MAGIC			0xa1b2cd34
#define	PCAP_SWAPPED_MODIFIED_MAGIC	0x34cdb2a1
#define	PCAP_NSEC_MAGIC				0xa1b23c4d
#define	PCAP_SWAPPED_NSEC_MAGIC		0x4d3cb2a1

/*
static pcap_file_header pcapGlobalHdr =
{
  PCAP_MAGIC,
  2,
  4,
  0,
  0,
  10000,
  1
};

static pcap_file_header pcapGlobalHdrNano =
{
  PCAP_NSEC_MAGIC,
  2,
  4,
  0,
  0,
  10000,
  1
};

*/

/* 4 bytes IP address */
typedef struct _IP_ADDRESS_V4
{
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
} IP_ADDRESS_V4;

/* IPv4 header */
typedef struct _IP_HEADER_V4 {
	u_char			ver_ihl;		// Version (4 bits) + Internet header length (4 bits)
	u_char			TypeOfService;	// Type of service 
	u_short			TotalLength;	// Total length 
	u_short			Identifier;		// Identification
	u_short			Flag_Offset;	// Flags (3 bits) + Fragment offset (13 bits)
	u_char			TimeToLive;		// Time to live
	u_char			Protocol;		// Protocol
	u_short			HeaderCRC;		// Header checksum
	IP_ADDRESS_V4	AddrSrc;		// Source address
	IP_ADDRESS_V4	AddrDest;		// Destination address
	u_int			Option_Pad;		// Option + Padding
} IP_HEADER_V4;

/* UDP header*/
typedef struct _UDP_HEADER {
	u_short			PortSrc;		// Source port
	u_short			PortDest;		// Destination port
	u_short			Length;			// Datagram length
	u_short			CRC;			// Checksum
}UDP_HEADER;

typedef struct {
	unsigned char	CRCCount : 4;
	unsigned char	ExtBit : 1;
	unsigned char	Padding : 1;
	unsigned char	Ver : 2;
	unsigned char	PayloadType : 7;
	unsigned char	Mark : 1;
	unsigned short	SeqNo;
	unsigned int	TimeStamp;
	unsigned int	SynchSrc;
} RTP_HEADER;

typedef enum _IP_HEADER_PROTOCOL_KIND
{
	enIP_HEADER_PROTOCOL_TCP = 0x06,
	enIP_HEADER_PROTOCOL_UDP = 0x11
} IP_HEADER_PROTOCOL_KIND;

typedef enum _RTP_PAYLOAD_TYPE
{
	enPAYLOAD_TYPE_NONE = -1,
	enPAYLOAD_TYPE_G711_MLAW = 0,
	enPAYLOAD_TYPE_CELP = 1,
	enPAYLOAD_TYPE_G726 = 2,
	enPAYLOAD_TYPE_GSM = 3,
	enPAYLOAD_TYPE_G723 = 4,
	enPAYLOAD_TYPE_G711_ALAW = 8,
	enPAYLOAD_TYPE_G722 = 9,
	enPAYLOAD_TYPE_G728 = 15,
	enPAYLOAD_TYPE_G729 = 18,

	enPAYLOAD_TYPE_H261 = 31,
	enPAYLOAD_TYPE_MPEG = 32,
	enPAYLOAD_TYPE_H263 = 34,
	enPAYLOAD_TYPE_MAX
} enRTP_PAYLOAD_TYPE;

#define MAXLEN_LINE				(1024)
#define MAXCNT_NIC				(16)			// Maximum NIC Card
#define MAXLEN_PCAP				(64*1024)		// 4 KB Capture
#define MAXTIME_PCAPREAD		(10)			// pCap read timeout
#define MAXLEN_IPADDR			(16)
#define MAXLEN_PATH				(260)


#define DEFLEN_ETHHDR			(14)
#define DEFLEN_IPHDR			(20)
#define DEFLEN_UDPHDR			(8)
#define DEFLEN_RTPHDR			(12)
#define DEFLEN_HEADERS			(DEFLEN_ETHHDR+DEFLEN_IPHDR+DEFLEN_UDPHDR+DEFLEN_RTPHDR)
#define DEFLEN_UNI_DISP_HDR		(5)

#define TCP_PROTOCOL ( 6 )
#define UDP_PROTOCOL ( 17 )
#define TPK_VER (3)

enum
{
	RTP_PAYLOAD_TYPE_G711_PCMU = 0,
	RTP_PAYLOAD_TYPE_G711_PCMA = 8,
	RTP_PAYLOAD_TYPE_G729 = 18,
	RTP_PAYLOAD_TYPE_DYNAMIC101 = 101    //RTP EVENT(dtmf)
};

#define UMAKEINT32(byte1,byte2,byte3,byte4) \
	( \
	(UINT)( \
	( (DWORD)(byte4) << 24) | ((DWORD)(byte3) << 16) | ((DWORD)(byte2) << 8) | (DWORD)(byte1) \
	) \
	) 
#define MAKETEST(a, b) ((ULONGLONG)(((UINT)(a)) | ((ULONGLONG)((UINT)(b))) << 32))

/// caydies! winpcap struct
// 6 byte MAC Address
typedef struct tsMac_address
{
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
	unsigned char byte4;
	unsigned char byte5;
	unsigned char byte6;
}NMac, * PNMac;

// 4 bytes IP address
typedef union NIPADDR
{
	uint32_t		nIpAddr;
	struct
	{
		uint8_t	nIp1;
		uint8_t	nIp2;
		uint8_t	nIp3;
		uint8_t	nIp4;
	};
} NIPADDR, * PIPADDR;

// IPv4 header
typedef struct NIPHEADER
{
	uint8_t		nVerIhl;			// Version (4 bits) + Internet header length (4 bits)
	uint8_t		nTos;				// Type of service 
	uint16_t	nTotlen;			// Total length 
	uint16_t	nId;				// Identification
	uint16_t	nFlagsFragOffset;	// Flags (3 bits) + Fragment offset (13 bits)
	uint8_t		nTtl;				// Time to live
	uint8_t		nProto;				// Protocol
	uint16_t	nCrc;				// Header checksum
	NIPADDR		stSrcAddr;			// Source address
	NIPADDR		stDstAddr;			// Destination address
	uint32_t	nOptPad;			// Option + Padding
} NIPHEADER, * PIPHEADER;

// UDP header
typedef struct NUDPHEADER
{
	uint16_t	nSrcPort;			// Source port
	uint16_t	nDstPort;			// Destination port
	uint16_t	nLength;			// Datagram length
	uint16_t	nCrc;				// Checksum
} NUDPHEADER, * PUDPHEADER;


typedef struct NRTPHEADER
{
	uint8_t		nCSrcCnt : 4;
	uint8_t		bExtensoin : 1;
	uint8_t		bPad : 1;
	uint8_t		nVer : 2;
	uint8_t		nPayType : 7;
	uint8_t		bMarker : 1;
	uint16_t	nSeqNum;
	uint32_t	nTimestamp;
	uint32_t	nSSrc;
} RTPHEADER, * PRTPHEADER;
// TCP header
typedef struct NTCPHEADER
{
	uint16_t	nSrcPort;			// Source port
	uint16_t	nDstPort;			// Destination port
	uint32_t    nSeqNum;            // Sequence Number 
	uint32_t    nAcknowNum;         // Acknowledgement Number
	uint8_t     nHeadLen;           // Header length   
	uint8_t     nFlags;             // Flags
	uint16_t	nWndSize;			// Window Size
	uint16_t	nCrc;				// Checksum
	uint8_t		nSeqAck[2];         // SEQ/ACK analysis
} NTCPHEADER, * PTCPHEADER;

typedef struct NIPPORTINFO
{
	uint32_t	nIp;
	uint16_t	nPort;
	uint16_t	nPadding;			// for Padding (Structure Alignments)
} NIPPORTINFO, * PIPPORTINFO;

typedef struct NicName
{
	pcap_if_t* pPcapIf;
	char    szDeviceName[250];
} NICNAME;

typedef struct NicIpAddr
{
	int		nNicIdx;
	char    szDeviceName[250];
	char	szNicIp[MAXLEN_IPADDR];
} NICIPADDR;

class PcapReader {
private:
	pcap_if_t* pAllDev = nullptr;
	std::map<int, pcap_if*> mDevice;
	pcap* pcapHandle;
	boost::scoped_ptr<boost::thread> pThread;
	bool runThd = false;
		
	void run(void (*onCapture)(const pcap_pkthdr*, const uint8_t*));
public:
	PcapReader();
	~PcapReader();
	bool init(int devNum, const std::string& filter);
	void free();
	bool start(void (*onCapture)(const pcap_pkthdr*, const uint8_t*));
	void stop();
};
