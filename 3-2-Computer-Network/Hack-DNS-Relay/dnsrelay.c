
// Hacking Computer Network Practice Project - DNS Relay
//
// Hacked by BOT-Man-JL, 2017
// Original Written by gaozc@bupt.edu.cn
//
// Special thanks: thiefuniverse - helping debug on Linux

#ifdef WIN32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

#define s_addr S_un.S_addr

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define SOCKET int
#define Sleep sleep

#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define BYTE uint8_t
#define WORD uint16_t
#define DWORD uint32_t

#define TABLE_BLOCK_SIZE 4096u
#define SZIP_BUF_SIZE 128u
#define SZDOMAINNAME_BUF_SIZE 512u

#define SZIP_BUF_SIZE_2 256u
#define PACKET_BUF_SIZE 4096u
#define RECORD_COUNT 4096u
#define TIMEOUT 8u
#define SLEEP_MS 500u

#define PORT 53u

typedef struct
{
	WORD from;
	WORD to;
	struct sockaddr_in sa;

	// Different from binary version, time_t in modern C is 64-bit
	time_t timestamp;
} Record;

WORD curId;
int debugLevel;
struct sockaddr_in saNameServer;
SOCKET sock;
BYTE *pbTable;

void InitWSA ()
{
#ifdef WIN32
	// Init WSA on Windows
	struct WSAData WSAData;
	if (WSAStartup (MAKEWORD (2, 2), &WSAData))
	{
		printf ("WSAStartup() error\n");
		exit (1);
	}
#endif
}

#ifndef WIN32
// Substitute on *NIX
int WSAGetLastError ()
{
	return errno;
}
#endif

// Convert 'szStr to lower-case
void ToLower (char *szStr) { for (; *szStr; ++szStr) if (*szStr >= 'A' && *szStr <= 'Z') *szStr = *szStr + ('a' - 'A'); }

// Print traffic info
// type: 1 => Simplified, 2 => Comprehensive
void PrintTraffic (const char *szPrefix, struct sockaddr_in *sa, BYTE *pbBuf, int bufLen, DWORD mode)
{
	if (debugLevel >= 2)
	{
		// Print prefix, ip addr, port and buffer length
		printf ("%s %s:%d (%d bytes) ", szPrefix, inet_ntoa (sa->sin_addr), ntohs (sa->sin_port), bufLen);

		// mode 1 is simplified logging
		if (mode == 2) return;

		// Print raw buffer
		for (int i = 0; i < bufLen; ++i) printf (" %02x", pbBuf[i]);

		// Print packet content
		WORD *pwBuf = (WORD *) pbBuf;
		DWORD flags = ntohs (pwBuf[1]);

		printf ("\n\tID %04x, QR %d, OPCODE %d, AA %d, TC %d, RD %d, RA %d, Z %d, RCODE %d\n"
				"\tQDCOUNT %u, ANCOUNT %u, NSCOUNT %u, ARCOUNT %u\n",
				ntohs (pwBuf[0]),
				(int) ((flags & 0x8000) >> 15),
				(int) ((flags & 0x7800) >> 11),
				(int) ((flags & 0x0400) >> 10),
				(int) ((flags & 0x0200) >> 9),
				(int) ((flags & 0x0100) >> 8),
				(int) ((flags & 0x0080) >> 7),
				(int) ((flags & 0x0070) >> 4),
				(int) ((flags & 0x000F) >> 0),
				ntohs (pwBuf[2]),
				ntohs (pwBuf[3]),
				ntohs (pwBuf[4]),
				ntohs (pwBuf[5]));
	}
}

// Load Table from 'szFileName
void LoadTable (const char *szFileName)
{
	char szIp[SZIP_BUF_SIZE];
	char szDomainName[SZDOMAINNAME_BUF_SIZE];

	int recordCount = 0;
	pbTable = (BYTE *) malloc (TABLE_BLOCK_SIZE);

	char *szTableCur = (char *) pbTable;
	int bufSize = TABLE_BLOCK_SIZE;
	printf ("Try to load table \"%s\" ... ", szFileName);

	FILE *pFile = fopen (szFileName, "r");
	if (pFile)
	{
		printf ("OK\n");

		for (DWORD *pdIp; fscanf (pFile, "%s%s", szIp, szDomainName) == 2; szTableCur = (char *) &pdIp[1])
		{
			ToLower (szDomainName);

			// Check if there is sufficent space
			if (&szTableCur[strlen (szDomainName) + 1 + sizeof (DWORD) + 1] > (char *) &pbTable[bufSize])
			{
				bufSize += TABLE_BLOCK_SIZE;
				pbTable = (BYTE *) realloc (pbTable, bufSize);
				if (!pbTable)
				{
					printf ("Failed to allocate %d bytes memory\n", bufSize);
					exit (1);
				}
			}

			// Count Record
			++recordCount;

			// Write to pbTable
			pdIp = (DWORD *) &szTableCur[sprintf (szTableCur, "%s", szDomainName) + 1];
			struct in_addr ipAddr;
			ipAddr.s_addr = inet_addr (szIp);
			pdIp[0] = ipAddr.s_addr;

			if (debugLevel >= 2)
				printf ("\t%3d: %s\t%s\n", recordCount, inet_ntoa (ipAddr), szDomainName);
		}
		fclose (pFile);
	}
	else
		printf ("Ignored!\n");

	// Truncate 'pbTable
	szTableCur[0] = 0;

	// Calc ocupied memory sized, and re-allocate it
	int memSize = szTableCur - (char *) pbTable + 1;
	pbTable = (BYTE *) realloc (pbTable, memSize);

	printf ("%d names, occupy %d bytes memory\n", recordCount, memSize);
}

// Set anwser to buffer
DWORD SetAnswer (DWORD *pdIp, BYTE *pbBuf, DWORD bufLen)
{
	WORD *pwBuf = (WORD *) pbBuf;

	// Set QR (Response) & AA (Authoritative answer) & RA (Recursion Available) 
	pwBuf[1] = pwBuf[1] | htons (0x8480);

	// Case: Valid IP
	if (pdIp[0])
	{
		pwBuf[3] = htons (1u);  // anwser count: 1

		char *szBufEnd = (char *) pbBuf + bufLen;
		strcpy (szBufEnd, (char *) &pbBuf[6 * sizeof (WORD)]);  // name: (same as query)
		szBufEnd = &szBufEnd[strlen (szBufEnd) + 1];

		*(WORD *) szBufEnd = htons (1u);  // type: 1
		szBufEnd += sizeof (WORD);
		*(WORD *) szBufEnd = htons (1u);  // class: 1
		szBufEnd += sizeof (WORD);
		*(DWORD *) szBufEnd = htonl (120u);  // ttl: 120
		szBufEnd += sizeof (DWORD);
		*(WORD *) szBufEnd = htons (4u);  // rdlength: 4
		szBufEnd += sizeof (WORD);
		*(DWORD *) szBufEnd = pdIp[0];  // rddata: IP DWORD
		szBufEnd += sizeof (DWORD);

		return szBufEnd - (char *) pbBuf;
	}

	// Case: 0.0.0.0
	// Set error response (RCODE = 3)
	pwBuf[1] = pwBuf[1] | htons (0x0003);

	return bufLen;
}

// Handle bussiness logic
void Run ()
{
	BYTE buf[PACKET_BUF_SIZE];
	char szIp[SZIP_BUF_SIZE_2];

	static Record records[RECORD_COUNT];
	static unsigned acceptCount;

	// Recieve a packet
	struct sockaddr_in saFrom;
	socklen_t saLen = sizeof (struct sockaddr_in);

	// Note that bufLen must be signed type
	int bufLen = recvfrom (sock, buf, PACKET_BUF_SIZE, 0, (struct sockaddr *) &saFrom, &saLen);

	// Case: Failed to recv
	if (bufLen <= 0)
	{
		if (debugLevel >= 1)
			printf ("recvfrom() Error #%d\n", WSAGetLastError ());
		Sleep (SLEEP_MS);
		return;
	}

	PrintTraffic ("RECV from", &saFrom, buf, bufLen, 1);

	// Get current time
	time_t curTime;
	time (&curTime);

	WORD *pwBuf = (WORD *) buf;

	// Case: Response Message from Name Server (QR == 1)
	if (pwBuf[1] & htons (0x8000))
	{
		// Find an entry to response to client without timeout
		Record *pRecord = records;
		while (curTime - pRecord->timestamp > TIMEOUT || pRecord->from != ntohs (pwBuf[0]))
		{
			// Next entry
			pRecord += sizeof (Record);

			// No such entry, discard the response
			if (pRecord >= &records[RECORD_COUNT]) return;
		}

		// Free this entry
		pRecord->timestamp = 0;

		// Restore the id
		pwBuf[0] = htons (pRecord->to);

		// Send this entry to Client
		PrintTraffic ("SEND to", &(pRecord->sa), buf, bufLen, 2);
		if (debugLevel >= 2)
			printf ("[ID %04x -> %04x]\n", pRecord->from, pRecord->to);
		if (sendto (sock, buf, bufLen, 0, (struct sockaddr *) &(pRecord->sa), sizeof (struct sockaddr_in)) < 0 && debugLevel >= 1)
			printf ("Failed to send RESPONSE to client, error %d\n", WSAGetLastError ());
		return;
	}

	// Case: Forward this packet
	//   - Not a Query
	//   - Non-standard Query (OPCODE != 0) || has-Authoritative-Answer (AA == 1)
	//   - Anwser not found in Table
	if (ntohs (pwBuf[2]) != 1 || pwBuf[1] & htons (0x7800))
	{
	labelForward:;
		// Clear timeout records, and find an avaliable entry
		Record *pRecord = records;
		while (1)
		{
			// Clear timeout records
			if (pRecord->timestamp && curTime - pRecord->timestamp > TIMEOUT)
				pRecord->timestamp = 0;

			// Find an avaliable entry
			if (!pRecord->timestamp)
				break;

			// Next entry
			pRecord += sizeof (Record);

			// No such entry, denie the response
			if (pRecord >= &records[RECORD_COUNT]) return;
		}

		// Save current packet's info to the entry
		pRecord->timestamp = curTime;
		memcpy (&(pRecord->sa), &saFrom, sizeof (struct sockaddr_in));
		pRecord->to = ntohs (pwBuf[0]);
		pRecord->from = curId++;

		// Replace the id
		pwBuf[0] = htons (pRecord->from);

		// Forward this packet to Name Server
		PrintTraffic ("SEND to", &saNameServer, buf, bufLen, 2);
		if (debugLevel >= 2)
			printf ("[ID %04x->%04x]\n", pRecord->from, pRecord->to);
		if (sendto (sock, buf, bufLen, 0, (struct sockaddr *) &saNameServer, sizeof (struct sockaddr_in)) < 0 && debugLevel >= 1)
			printf ("Failed to send QUERY to nameserver, Error %d\n", WSAGetLastError ());
		return;
	}

	// Case: It's time to play a man-in-middle :-)

	// Retrieve IP address
	char *szIpCur = szIp;
	BYTE *pbSegBeg = buf + 6 * sizeof (WORD) + 1;
	BYTE segSize = buf[6 * sizeof (WORD)];

	while (segSize)
	{
		// Copy seg
		memcpy (szIpCur, pbSegBeg, segSize);

		// Move to next seg
		szIpCur += segSize;
		pbSegBeg += segSize;

		// Check next seg
		segSize = pbSegBeg[0];
		++pbSegBeg;
		if (!segSize) break;

		// Copy dot
		szIpCur[0] = '.';
		++szIpCur;
	}

	// Truncate and modify szIp
	szIpCur[0] = 0;
	ToLower (szIp);

	// Retrieve type and class
	WORD *pwTypeClass = (WORD *) pbSegBeg;

	WORD type = ntohs (pwTypeClass[0]);
	WORD clas = ntohs (pwTypeClass[1]);

	// Check if Addr found in Table, and Get the corresponding position
	DWORD isAddrFound = 0;
	char *szEntryBeg = (char *) pbTable;

	// Only handle IPv4 message when table is not empty
	if (type == 1 && clas == 1 && pbTable[0])
	{
		while (strcmp (szEntryBeg, szIp) != 0)
		{
			DWORD addrSize = strlen (szEntryBeg) + 1;
			if (!szEntryBeg[addrSize + 4])
				goto labelNotFound;
			szEntryBeg += addrSize + 4;
		}
		isAddrFound = 1u;
	}

labelNotFound:
	if (debugLevel >= 1)
		printf ("\t%6.3f  %2u:%c %s", clock () * 0.001, acceptCount++, isAddrFound ? '*' : ' ', szIp);
	if (debugLevel >= 2 - (type != 1))
		printf (", TYPE %u, CLASS %u", type, clas);
	if (debugLevel >= 1)
		printf ("\n");

	// Case: Not found
	if (!isAddrFound)
		goto labelForward;

	// Case: Find the anwser on hosts

	// Set Answer
	DWORD newBufLen = SetAnswer ((DWORD *) &szEntryBeg[strlen (szEntryBeg) + 1], buf, bufLen);

	// Response to Client
	PrintTraffic ("SEND to", &saFrom, buf, newBufLen, 1);
	if (sendto (sock, buf, newBufLen, 0, (struct sockaddr *) &saFrom, sizeof (struct sockaddr_in)) < 0 && debugLevel >= 1)
		printf ("Failed to send RESPONSE to client, Error %d\n", WSAGetLastError ());
}

// Main Function
int main (int argc, const char **argv, const char **envp)
{
	// Init Prompt
	printf ("DNSRELAY, Version 1.10, Build: Jul 12 2010 02:01:21\n"
			"Usage: dnsrelay [-d | -dd] [<dns-server>] [<db-file>]\n\n");

	// Seed 'curId for filtering traffic
	curId = (WORD) time (0);

	// Default Config
	const char *szIp = "202.106.0.20";
	const char *szFileName = "dnsrelay.txt";

	// Check Args (Original Version)
	int iNextArg = 1;
	if (iNextArg < argc)
	{
		if (strcmp (argv[1], "-d") == 0)
		{
			debugLevel = 1;
			++iNextArg;
		}

		if (iNextArg < argc)
		{
			if (strcmp (argv[iNextArg], "-dd") == 0)
			{
				debugLevel = 2;
				++iNextArg;
			}

			if (iNextArg < argc)
			{
				szIp = argv[iNextArg];
				++iNextArg;

				if (iNextArg < argc)
					szFileName = argv[iNextArg];
			}
		}
	}

	// Init WSA
	InitWSA ();

	// Config Name Server
	saNameServer.sin_family = AF_INET;
	saNameServer.sin_port = htons (PORT);
	saNameServer.sin_addr.s_addr = inet_addr (szIp);
	if (saNameServer.sin_addr.s_addr == INADDR_NONE)
	{
		printf ("Bad name server IP address \"%s\".\n", szIp);
		exit (1);
	}

	// Prompt config
	printf ("Name server %s:%d.\n", inet_ntoa (saNameServer.sin_addr), ntohs (saNameServer.sin_port));
	printf ("Debug level %d.\n", debugLevel);

	// Init sock
	sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0)
	{
		perror ("opening udp server socket");
		exit (1);
	}

	// Bind the 'sock at 'PORT
	struct sockaddr_in saBind;
	saBind.sin_family = AF_INET;
	saBind.sin_addr.s_addr = INADDR_ANY;
	saBind.sin_port = htons (PORT);
	printf ("Bind UDP port %d ...", PORT);
	if (bind (sock, (struct sockaddr *) &saBind, sizeof (struct sockaddr_in)) < 0)
	{
		printf ("**** Fail to bind UDP port, Error %d\n", WSAGetLastError ());
		exit (1);
	}
	printf ("OK!\n");

	// Load Table
	LoadTable (szFileName);

	// Run
	while (1) Run ();
}