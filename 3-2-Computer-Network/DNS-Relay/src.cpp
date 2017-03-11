
// Computer Network Practice Project - DNS Relay
//
// Written by BOT-Man-JL, 2017

#ifdef WIN32

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
#define closesocket close

#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <tuple>
#include <vector>
#include <list>
#include <unordered_map>

#include <algorithm>
#include <functional>
#include <type_traits>
#include <chrono>
#include <exception>

#include <cstdint>
#include <cstring>

#ifndef WIN32
// Substitute on *NIX
int WSAGetLastError ()
{
	return errno;
}
#endif

namespace DnsRelay
{
	constexpr int PACKET_BUF_SIZE = 4096;
	constexpr int PORT = 53;
	const std::string HOSTS_FILENAME = "dnsrelay.txt";
	const std::string NAMESERVER_IP = "202.106.0.20";

	struct Exception
	{
		int debugLevel;
		std::string what;
	};

	class Config
	{
	public:
		Config (int argc, char *argv[]) :
			_debugLevel (0),
			_hostsFilename (HOSTS_FILENAME),
			_nameserverIp (NAMESERVER_IP)
		{
			auto readArgs = [&]
			{
				using namespace std::string_literals;
				auto nextArg = 1;
				if (nextArg >= argc) return;

				if (argv[nextArg] == "-d"s)
				{
					_debugLevel = 1;
					++nextArg;
				}
				if (nextArg >= argc) return;

				if (argv[nextArg] == "-dd"s)
				{
					_debugLevel = 2;
					++nextArg;
				}
				if (nextArg >= argc) return;

				_nameserverIp = argv[nextArg];
				++nextArg;
				if (nextArg >= argc) return;

				_hostsFilename = argv[nextArg];
			};

			auto initNameServer = [this]
			{
				_saNameServer.sin_family = AF_INET;
				_saNameServer.sin_port = htons (PORT);

				if (!inet_pton (AF_INET, _nameserverIp.c_str (),
					(void *) &_saNameServer.sin_addr))
				{
					std::ostringstream ostrs;
					ostrs << "Bad name server IP address \"" << _nameserverIp << "\".";
					throw std::runtime_error (ostrs.str ());
				}
			};

			readArgs ();
			initNameServer ();
		}

		const int GetDebugLevel () const { return _debugLevel; }
		const std::string &GetHostsFilename () const { return _hostsFilename; }
		const sockaddr_in &GetNameServerSa () const { return _saNameServer; }

	private:
		int _debugLevel;
		std::string _hostsFilename;
		std::string _nameserverIp;
		sockaddr_in _saNameServer;
	};

	struct Packet
	{
		struct Header
		{
			uint16_t id;     // 16 bit

			// flag - 16 bit
			bool qr;         // 1 bit
			uint8_t opCode;  // 4 bit
			bool aa;         // 1 bit
			bool tc;         // 1 bit
			bool rd;         // 1 bit
			bool ra;         // 1 bit
			uint8_t zero;    // 3 bit
			uint8_t rCode;   // 4 bit

			uint16_t qdCount;  // 16 bit
			uint16_t anCount;  // 16 bit
			uint16_t nsCount;  // 16 bit
			uint16_t arCount;  // 16 bit
		};

		struct Question
		{
			std::string name;
			uint16_t type;  // 16 bit
			uint16_t clas;  // 16 bit
		};

		struct Resource
		{
			std::string name;
			uint16_t type;   // 16 bit
			uint16_t clas;   // 16 bit
			uint32_t ttl;    // 32 bit
			// rdLen - 16 bit
			std::vector<uint8_t> rdData; // rdLen
		};

		Packet (const sockaddr_in &_addr, uint8_t *buf, int bufLen)
			: addr (_addr)
		{
			auto checkLen = [&] (size_t size)
			{
				bufLen -= size;
				if (bufLen < 0)
					throw Exception { 1, "Bad packet" };
			};

			// Validation
			checkLen (6 * sizeof (uint16_t));

			// Retrieve Header
			uint16_t *pwBuf = (uint16_t *) buf;

			header.id = ntohs (pwBuf[0]);
			uint16_t flags = ntohs (pwBuf[1]);
			header.qdCount = ntohs (pwBuf[2]);
			header.anCount = ntohs (pwBuf[3]);
			header.nsCount = ntohs (pwBuf[4]);
			header.arCount = ntohs (pwBuf[5]);

			header.qr = (flags & 0x8000) >> 15;
			header.opCode = (flags & 0x7800) >> 11;
			header.aa = (flags & 0x0400) >> 10;
			header.tc = (flags & 0x0200) >> 9;
			header.rd = (flags & 0x0100) >> 8;
			header.ra = (flags & 0x0080) >> 7;
			header.zero = (flags & 0x0070) >> 4;
			header.rCode = (flags & 0x000F) >> 0;

			// Buffer Pointer
			uint8_t *pbBuf = (uint8_t *) &pwBuf[6];

			auto getDomainName = [&pbBuf, buf] (std::string &name)
			{
				auto pbBufBeg = pbBuf;
				bool isCompressed = false;
				auto szSeg = (const char *) pbBuf;

				while (*szSeg)
				{
					// Handle compressed domain name
					if ((*szSeg & 0x80) && (*szSeg & 0x40))
					{
						uint16_t *pwBuf = (uint16_t *) szSeg;
						uint16_t offset = ntohs (*pwBuf) & 0x3FFF;
						szSeg = (const char *) buf + offset;

						if (!isCompressed) pbBuf += sizeof (uint16_t);
						isCompressed = true;
					}

					name.append (szSeg, *(uint8_t *) szSeg + 1);
					szSeg += *szSeg + 1;
					if (!isCompressed) pbBuf = (uint8_t *) szSeg;
				}

				if (!isCompressed) ++pbBuf;
				return pbBuf - pbBufBeg;
			};

			auto getField = [&pbBuf] (auto &val, auto fn)
			{
				using Type = std::remove_reference_t<decltype (val)>;
				Type *pBuf = (Type *) pbBuf;
				val = fn (*pBuf);
				pbBuf += sizeof (Type);
			};

			// Retrieve Questions
			for (auto qdCount = header.qdCount; qdCount > 0; --qdCount)
			{
				Question question;

				// Retrieve domain name
				auto domainNameLen = getDomainName (question.name);

				// Validation
				checkLen (domainNameLen + 2 * sizeof (uint16_t));

				// Retrieve fields
				getField (question.type, ntohs);
				getField (question.clas, ntohs);

				// Set Question
				questions.emplace_back (std::move (question));
			}

			// Retrieve Resources
			for (auto rcCount = header.anCount + header.nsCount + header.arCount; rcCount > 0; --rcCount)
			{
				Resource resource;

				// Retrieve domain name
				auto domainNameLen = getDomainName (resource.name);

				// Retrieve fields
				getField (resource.type, ntohs);
				getField (resource.clas, ntohs);
				getField (resource.ttl, ntohl);

				// Retrieve data
				uint16_t rdLen;
				getField (rdLen, ntohs);
				resource.rdData.assign (pbBuf, pbBuf + rdLen);
				pbBuf += rdLen;

				// Validation
				checkLen (domainNameLen + 3 * sizeof (uint16_t) + sizeof (uint32_t) + rdLen);

				// Set Question
				resources.emplace_back (std::move (resource));
			}

			// Validation
			if (bufLen != 0 ||
				questions.size () != header.qdCount ||
				resources.size () != header.anCount + header.nsCount + header.arCount)
				throw Exception { 1, "Invalid packet" };
		}

		void ToBuffer (uint8_t *buf, int &bufLen) const
		{
			auto oldLen = bufLen;
			bufLen = 0;

			auto checkLen = [&] (size_t size)
			{
				oldLen -= size;
				if (oldLen < 0) throw Exception { 1, "Bad buffer" };
				bufLen += size;
			};

			// Validation
			checkLen (6 * sizeof (uint16_t));

			// Set Header
			uint16_t *pwBuf = (uint16_t *) buf;
			uint16_t flags = 0;

			if (header.qr) flags |= 0x8000;
			flags |= header.opCode << 11;
			if (header.aa) flags |= 0x0400;
			if (header.tc) flags |= 0x0200;
			if (header.rd) flags |= 0x0100;
			if (header.ra) flags |= 0x0080;
			flags |= header.zero << 4;
			flags |= header.rCode << 0;

			pwBuf[0] = htons (header.id);
			pwBuf[1] = htons (flags);
			pwBuf[2] = htons (header.qdCount);
			pwBuf[3] = htons (header.anCount);
			pwBuf[4] = htons (header.nsCount);
			pwBuf[5] = htons (header.arCount);

			// Buffer Pointer
			uint8_t *pbBuf = (uint8_t *) &pwBuf[6];

			auto setField = [&pbBuf] (auto val, auto fn)
			{
				decltype (val) *pBuf = (decltype (val) *) pbBuf;
				*pBuf = fn (val);
				pbBuf += sizeof (val);
			};

			// Set Questions
			for (const auto &question : questions)
			{
				// Validation
				checkLen (question.name.size () + 1 + 2 * sizeof (uint16_t));

				// Set domain name
				strncpy ((char *) pbBuf, question.name.c_str (),
						 question.name.size () + 1);
				pbBuf += question.name.size () + 1;

				// Set fields
				setField (question.type, htons);
				setField (question.clas, htons);
			}

			// Set Resources
			for (const auto &resource : resources)
			{
				// Validation
				checkLen (resource.name.size () + 1 +
						  3 * sizeof (uint16_t) + sizeof (uint32_t) +
						  resource.rdData.size ());

				// Set domain name
				strncpy ((char *) pbBuf, resource.name.c_str (),
						 resource.name.size () + 1);
				pbBuf += resource.name.size () + 1;

				// Set fields
				setField (resource.type, htons);
				setField (resource.clas, htons);
				setField (resource.ttl, htonl);
				setField ((uint16_t) resource.rdData.size (), htons);

				// Set data
				memcpy (pbBuf, resource.rdData.data (),
						resource.rdData.size ());
				pbBuf += resource.rdData.size ();
			}
		}

		sockaddr_in addr;
		Header header;
		std::list<Question> questions;
		std::list<Resource> resources;
	};

	class HostsTable
	{
	public:
		HostsTable (const Config &config)
		{
			std::ifstream ifs (config.GetHostsFilename ());
			if (!ifs.is_open ()) return;

			std::string ip, domainName;
			while (ifs >> ip >> domainName)
			{
				std::for_each (domainName.begin (), domainName.end (),
							   [] (auto &ch) { ch = isupper (ch) ? ch + 'a' - 'A' : ch; });

				in_addr addrIn;
				inet_pton (AF_INET, ip.c_str (), (void *) &addrIn);
				_hosts.emplace (std::move (domainName), addrIn.s_addr);
			}
		}

		const uint32_t GetIp (const std::string &domainName) const { return _hosts.at (domainName); }

	private:
		std::unordered_map<std::string, uint32_t> _hosts;
	};

	class Connector
	{
	public:
		Connector (const Config &config)
		{
			// Init sock
			_sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (_sock < 0)
				throw std::runtime_error ("Failed to open udp server socket");

			// Bind the 'sock at 'PORT
			sockaddr_in saBind;
			saBind.sin_family = AF_INET;
			saBind.sin_addr.s_addr = INADDR_ANY;
			saBind.sin_port = htons (PORT);

			if (bind (_sock, (sockaddr *) &saBind, sizeof (sockaddr_in)) < 0)
			{
				std::ostringstream ostrs;
				ostrs << "Fail to bind UDP port, Error " << WSAGetLastError ();
				throw std::runtime_error (ostrs.str ());
			}
		}

		~Connector () { closesocket (_sock); }

		void Recv (std::function<void (const Packet &)> callback)
		{
			uint8_t buf[PACKET_BUF_SIZE];
			sockaddr_in saFrom;
			socklen_t saLen = sizeof (sockaddr_in);

			int bufLen = recvfrom (_sock, (char *) buf, PACKET_BUF_SIZE, 0, (sockaddr *) &saFrom, &saLen);
			if (bufLen <= 0)
			{
				std::ostringstream ostrs;
				ostrs << "recvfrom() Error #" << WSAGetLastError ();
				throw Exception { 1, ostrs.str () };
			}

			callback (Packet (saFrom, buf, bufLen));
		}

		void Send (const Packet &packet)
		{
			uint8_t buf[PACKET_BUF_SIZE];
			int bufLen = sizeof (buf);
			packet.ToBuffer (buf, bufLen);

			if (sendto (_sock, (char *) buf, bufLen, 0, (const sockaddr *) &packet.addr, sizeof (sockaddr_in)) < 0)
			{
				std::ostringstream ostrs;
				ostrs << "Failed to send RESPONSE to client, Error " << WSAGetLastError ();
				throw Exception { 1, ostrs.str () };
			}
		}

	private:
		SOCKET _sock;
	};
}

int main (int argc, char *argv[])
{
	using namespace DnsRelay;

	try
	{
		// Init WSA on Windows
#ifdef WIN32
		WSAData WSAData;
		if (WSAStartup (MAKEWORD (2, 2), &WSAData))
			throw std::runtime_error ("WSAStartup() error");
#endif
		// Init
		Config config (argc, argv);
		Connector connector (config);
		HostsTable hostsTable (config);

		// Predicates
		auto isResponseMsg = [] (const Packet &packet)
		{
			return packet.header.qr == 1;
		};
		auto isSingleQuery = [] (const Packet &packet)
		{
			return packet.header.qdCount == 1;
		};
		auto isStandardQuery = [] (const Packet &packet)
		{
			return packet.header.opCode == 0;
		};
		auto hasAuthAns = [] (const Packet &packet)
		{
			return packet.header.aa;
		};
		auto hasv4Query = [] (const Packet &packet)
		{
			return !packet.questions.empty () &&
				packet.questions.front ().type == 1 &&
				packet.questions.front ().clas == 1;
		};
		auto hasAnswer = [] (const Packet::Resource &res)
		{
			return !res.rdData.empty ();
		};

		// Records
		using TimePoint = std::chrono::system_clock::time_point;
		using Record = std::tuple<uint16_t, sockaddr_in, TimePoint>;
		std::unordered_map<uint16_t, Record> records;

		auto clearTimeout = [] (std::unordered_map<uint16_t, Record> &records)
		{
			for (auto p = records.begin (); p != records.end ();)
			{
				using namespace std::chrono_literals;
				TimePoint timePoint;
				std::tie (std::ignore, std::ignore, timePoint) = p->second;
				if (std::chrono::system_clock::now () - timePoint < 2s)
					p = records.erase (p);
				else
					p = ++p;
			}
		};

		// Handler
		auto fromNameServer = [&] (const Packet &packet)
		{
			try
			{
				const Record &record = records.at (packet.header.id);

				auto newPacket = packet;
				std::tie (newPacket.header.id, newPacket.addr, std::ignore) = record;
				connector.Send (newPacket);
			}
			catch (const std::exception &) {}
		};

		auto toNameServer = [&] (const Packet &packet)
		{
			//clearTimeout (records);

			static uint16_t curId;
			records.emplace (++curId, Record {
				packet.header.id, packet.addr,
				std::chrono::system_clock::now () });

			auto newPacket = packet;
			newPacket.header.id = curId;
			newPacket.addr = config.GetNameServerSa ();
			connector.Send (newPacket);
		};

		// On Recv
		auto onRecv = [&] (const Packet &packet)
		{
			// Response
			if (isResponseMsg (packet))
				return fromNameServer (packet);

			// Non-normal Query
			if (!isSingleQuery (packet) ||
				!isStandardQuery (packet) ||
				hasAuthAns (packet))
				return toNameServer (packet);

			// Find answer in hostsTable
			Packet::Resource answer;
			try
			{
				// Search in domain name in lower-case
				std::string domainName;
				const char *pbBuf =
					packet.questions.front ().name.c_str ();

				while (*pbBuf)
				{
					domainName.append ((pbBuf + 1), *pbBuf);
					pbBuf += 1 + *pbBuf;
					if (*pbBuf) domainName.append (1, '.');
				}

				std::for_each (domainName.begin (), domainName.end (),
							   [] (auto &ch) { ch = isupper (ch) ? ch + 'a' - 'A' : ch; });

				auto dwordIp = hostsTable.GetIp (domainName);
				if (dwordIp != INADDR_ANY)
				{
					answer.rdData.assign ((uint8_t *) &dwordIp,
						(uint8_t *) (&dwordIp + 1));
				}
			}
			// Forward if not found
			catch (const std::exception &) { return toNameServer (packet); }

			// Set Anwser
			auto newPacket = packet;
			newPacket.header.qr = true;
			newPacket.header.aa = true;
			newPacket.header.ra = true;

			if (!hasAnswer (answer))
				newPacket.header.rCode = 3;
			else
			{
				newPacket.header.anCount = 1;
				answer.name = packet.questions.front ().name;
				answer.type = 1;
				answer.clas = 1;
				answer.ttl = 120;
				newPacket.resources.emplace_front (std::move (answer));
			}
			connector.Send (newPacket);
		};

		// Run
		while (true)
		{
			try { connector.Recv (onRecv); }
			catch (const Exception &ex)
			{
				if (config.GetDebugLevel () >= ex.debugLevel)
					std::cerr << ex.what << std::endl;
			}
		}
	}
	catch (const std::exception &ex)
	{
		std::cerr << ex.what () << std::endl;
	}

	// Fatal Error
	return 1;
}
