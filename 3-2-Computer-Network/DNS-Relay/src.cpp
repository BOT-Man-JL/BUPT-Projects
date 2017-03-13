
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
#include <iomanip>
#include <string>

#include <tuple>
#include <vector>
#include <list>
#include <unordered_map>
#include <memory>

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

#define HOSTS_FILENAME "dnsrelay.txt"
#define NAMESERVER_IP "202.106.0.20"

namespace DnsRelay
{
	constexpr int PACKET_BUF_SIZE = 4096;
	constexpr int PORT = 53;

	class Logger
	{
	public:
		// Print timestamp at construction
		Logger (std::ostream &os) : _os (os)
		{
			std::time_t t = std::time (nullptr);
			std::tm tm = *std::localtime (&t);
			_os << std::put_time (&tm, "%c") << " - ";
		}

		// Flush at destruction
		~Logger () { _os << std::flush; }

		template<typename T>
		Logger &operator << (const T &val) { _os << val; return *this; }
	private:
		std::ostream &_os;
	};

	struct Config
	{
		Config (int argc, char *argv[])
		{
			if (--argc % 2)
				throw std::runtime_error ("Bad arguments");

			std::unordered_map<std::string, std::string &> mapper
			{
				{ "-o", logFilename },
				{ "-n", nameserverIp },
				{ "-h", hostsFilename }
			};

			for (int i = 0; i < argc / 2; i++)
			{
				try { mapper.at (argv[i * 2 + 1]) = argv[i * 2 + 2]; }
				catch (const std::exception &) {}
			}
		}

		static const char *GetPrompt ()
		{
			return "Args: [-o <log-file-name> | "
				"-n <name-server-ip> | -h <hosts-file-name>]\n"
				"Default Config: -o <stdout> -n " NAMESERVER_IP
				" -h " HOSTS_FILENAME;
		}

		std::string logFilename;
		std::string nameserverIp = NAMESERVER_IP;
		std::string hostsFilename = HOSTS_FILENAME;
	};

	std::ostream &operator << (std::ostream &os, const Config &config)
	{
		return os << "-o " << (config.logFilename.empty () ?
				"<stdout>" : config.logFilename.c_str ())
			<< " -n " << config.nameserverIp
			<< " -h " << config.hostsFilename;
	}

	struct Packet
	{
		struct Header
		{
			uint16_t id;     // 16 bit
			// [flags - 16 bit]
			bool qr;         // 1 bit
			uint8_t opCode;  // 4 bit
			bool aa;         // 1 bit
			bool tc;         // 1 bit
			bool rd;         // 1 bit
			bool ra;         // 1 bit
			uint8_t zero;    // 3 bit
			uint8_t rCode;   // 4 bit
			// qdCount - 16 bit
			// anCount - 16 bit
			// nsCount - 16 bit
			// arCount - 16 bit
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
				if (bufLen < 0) throw std::runtime_error ("Bad packet len");
			};

			// Validation
			checkLen (6 * sizeof (uint16_t));

			// Retrieve Header
			uint16_t *pwBuf = (uint16_t *) buf;

			header.id = ntohs (pwBuf[0]);
			uint16_t flags = ntohs (pwBuf[1]);

			uint16_t qdCount = ntohs (pwBuf[2]);
			uint16_t anCount = ntohs (pwBuf[3]);
			uint16_t nsCount = ntohs (pwBuf[4]);
			uint16_t arCount = ntohs (pwBuf[5]);

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
			for (; qdCount > 0; --qdCount)
			{
				Question question;

				// Retrieve domain name
				auto domainNameLen = getDomainName (question.name);

				// Retrieve fields
				getField (question.type, ntohs);
				getField (question.clas, ntohs);

				// Validation
				checkLen (domainNameLen + 2 * sizeof (uint16_t));

				// Set Question
				qd.emplace_back (std::move (question));
			}

			// Retrieve Resources
			auto getRes = [&] (std::list<Resource> &resources,
							   uint16_t rcCount)
			{
				for (; rcCount > 0; --rcCount)
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
					checkLen (domainNameLen + 3 * sizeof (uint16_t) +
							  sizeof (uint32_t) + rdLen);

					// Set Resource
					resources.emplace_back (std::move (resource));
				}
			};

			getRes (an, anCount);
			getRes (ns, nsCount);
			getRes (ar, arCount);

			// Validation
			if (bufLen != 0) throw std::runtime_error ("Bad packet len");
		}

		void ToBuffer (uint8_t *buf, int &bufLen) const
		{
			auto oldLen = bufLen;
			bufLen = 0;

			auto checkLen = [&] (size_t size)
			{
				oldLen -= size;
				if (oldLen < 0) throw std::runtime_error ("Bad buffer size");
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
			pwBuf[2] = htons ((uint16_t) qd.size ());
			pwBuf[3] = htons ((uint16_t) an.size ());
			pwBuf[4] = htons ((uint16_t) ns.size ());
			pwBuf[5] = htons ((uint16_t) ar.size ());

			// Buffer Pointer
			uint8_t *pbBuf = (uint8_t *) &pwBuf[6];

			auto setField = [&pbBuf] (auto val, auto fn)
			{
				decltype (val) *pBuf = (decltype (val) *) pbBuf;
				*pBuf = fn (val);
				pbBuf += sizeof (val);
			};

			// Set Questions
			for (const auto &question : qd)
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
			auto setRes = [&] (const std::list<Resource> &resources)
			{
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
			};

			setRes (an);
			setRes (ns);
			setRes (ar);
		}

		static std::string ParseDomainName (const std::string &rawName)
		{
			std::string domainName;
			const char *pbBuf = rawName.c_str ();

			while (*pbBuf)
			{
				domainName.append ((pbBuf + 1), *pbBuf);
				pbBuf += 1 + *pbBuf;
				if (*pbBuf) domainName.append (1, '.');
			}

			std::for_each (domainName.begin (), domainName.end (),
						   [] (auto &ch) { ch = tolower (ch); });
			return domainName;
		}

		sockaddr_in addr;
		Header header;
		std::list<Question> qd;
		std::list<Resource> an;
		std::list<Resource> ns;
		std::list<Resource> ar;
	};

	std::ostream &operator << (std::ostream &os, const Packet &packet)
	{
		auto printAddr = [] (std::ostream &os,
							 const sockaddr_in &addr)
		{
			constexpr int BUF_SIZE = 64;
			char szIp[BUF_SIZE];

			if (!inet_ntop (
				AF_INET, (const void *) &addr.sin_addr,
				szIp, BUF_SIZE))
				strcpy (szIp, "Invalid IP");
			os << "[" << szIp << ":"
				<< ntohs (addr.sin_port) << "]\n";
		};

		auto printHeader = [] (std::ostream &os,
							   const Packet::Header &header)
		{
			os << "ID: " << std::setw (4) << std::setfill ('0')
				<< std::right << std::hex << header.id << std::dec
				<< " QR: " << header.qr
				<< " OPCODE: " << (uint16_t) header.opCode
				<< " AA: " << header.aa
				<< " TC: " << header.tc
				<< " RD: " << header.rd
				<< " RA: " << header.ra
				<< " ZERO: " << (uint16_t) header.zero
				<< " RCODE: " << (uint16_t) header.rCode;
		};

		auto printQuestion = [] (std::ostream &os,
								 const Packet::Question &ques)
		{
			os << " - Domain Name: [" <<
				Packet::ParseDomainName (ques.name)
				<< "] Type: " << ques.type
				<< " Class: " << ques.clas << std::endl;
		};

		auto printResource = [] (std::ostream &os,
								 const Packet::Resource &res)
		{
			os << " - Domain Name: [" <<
				Packet::ParseDomainName (res.name)
				<< "] Type: " << res.type
				<< " Class: " << res.clas
				<< " TTL: " << res.ttl
				<< " rdLen: " << res.rdData.size ()
				<< " rdData: [ " << std::hex;
			for (const auto &byteVal : res.rdData)
				os << std::setw (2) << std::setfill ('0')
				<< std::right << (uint16_t) byteVal << " ";
			os << std::dec << "]" << std::endl;
		};

		printAddr (os, packet.addr);
		printHeader (os, packet.header);
		os << std::endl;

		os << packet.qd.size () << " RD fields:" << std::endl;
		for (const auto &ques : packet.qd)
			printQuestion (os, ques);

		os << packet.an.size () << " AN fields:" << std::endl;
		for (const auto &res : packet.an)
			printResource (os, res);

		os << packet.ns.size () << " NS fields:" << std::endl;
		for (const auto &res : packet.ns)
			printResource (os, res);

		os << packet.ar.size () << " AR fields:" << std::endl;
		for (const auto &res : packet.ar)
			printResource (os, res);

		return os << std::endl;
	}

	class Connector
	{
	public:
		Connector ()
		{
			// Init sock
			_sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (_sock < 0)
				throw std::runtime_error ("socket() Error");

			// Bind the 'sock at 'PORT
			sockaddr_in saBind;
			saBind.sin_family = AF_INET;
			saBind.sin_addr.s_addr = INADDR_ANY;
			saBind.sin_port = htons (PORT);

			if (bind (_sock, (sockaddr *) &saBind,
				sizeof (sockaddr_in)) < 0)
			{
				std::ostringstream ostrs;
				ostrs << "bind() Error #" << WSAGetLastError ();
				throw std::runtime_error (ostrs.str ());
			}
		}

		~Connector () { closesocket (_sock); }

		void Recv (std::function<void (const Packet &)> callback)
		{
			uint8_t buf[PACKET_BUF_SIZE];
			sockaddr_in saFrom;
			socklen_t saLen = sizeof (sockaddr_in);

			int bufLen = recvfrom (_sock, (char *) buf, PACKET_BUF_SIZE,
								   0, (sockaddr *) &saFrom, &saLen);
			if (bufLen <= 0)
			{
				std::ostringstream ostrs;
				ostrs << "recvfrom() Error #" << WSAGetLastError ();
				throw std::runtime_error (ostrs.str ());
			}

			callback (Packet (saFrom, buf, bufLen));
		}

		void Send (const Packet &packet)
		{
			uint8_t buf[PACKET_BUF_SIZE];
			int bufLen = sizeof (buf);
			packet.ToBuffer (buf, bufLen);

			if (sendto (_sock, (char *) buf, bufLen,
				0, (const sockaddr *) &packet.addr,
				sizeof (sockaddr_in)) < 0)
			{
				std::ostringstream ostrs;
				ostrs << "sendto Error #" << WSAGetLastError ();
				throw std::runtime_error (ostrs.str ());
			}
		}

	private:
		SOCKET _sock;
	};
}

int main (int argc, char *argv[])
{
	using namespace DnsRelay;
	std::cout << "DNS Relay by BOT-Man-JL, 2017\n";
	std::cout << Config::GetPrompt () << std::endl;

	try
	{
		// Init WSA on Windows
#ifdef WIN32
		WSAData WSAData;
		if (WSAStartup (MAKEWORD (2, 2), &WSAData))
			throw std::runtime_error ("WSAStartup() Error");
#endif
		// Load Config
		Config config (argc, argv);
		std::cout << "Current Config: " << config << std::endl;

		// Init Name Server
		sockaddr_in saNameServer;
		saNameServer.sin_family = AF_INET;
		saNameServer.sin_port = htons (PORT);

		if (!inet_pton (AF_INET, config.nameserverIp.c_str (),
			(void *) &saNameServer.sin_addr))
		{
			std::ostringstream ostrs;
			ostrs << "Bad name server IP address: "
				<< config.nameserverIp;
			throw std::runtime_error (ostrs.str ());
		}

		// Init hosts table
		using HostsTable = std::unordered_map<std::string, uint32_t>;
		HostsTable hostsTable;
		std::ifstream ifs (config.hostsFilename);
		if (ifs.is_open ())
		{
			std::string ip, domainName;
			while (ifs >> ip >> domainName)
			{
				std::for_each (domainName.begin (), domainName.end (),
							   [] (auto &ch) { ch = tolower (ch); });

				in_addr addrIn;
				if (inet_pton (AF_INET, ip.c_str (), (void *) &addrIn))
					hostsTable.emplace (std::move (domainName), addrIn.s_addr);
			}
		}

		// Init Logger
		std::ofstream ofs (config.logFilename);
		std::ostream &logFile = ofs.is_open () ? ofs : std::cout;

		// Records
		using TimePoint = std::chrono::system_clock::time_point;
		using Record = std::tuple<uint16_t, sockaddr_in, TimePoint>;
		std::unordered_map<uint16_t, Record> records;

		auto clearTimeout = [] (
			std::unordered_map<uint16_t, Record> &records)
		{
			for (auto p = records.begin (); p != records.end ();)
			{
				using namespace std::chrono_literals;
				TimePoint timePoint;
				std::tie (std::ignore, std::ignore, timePoint) = p->second;
				if (std::chrono::system_clock::now () - timePoint > 8s)
					p = records.erase (p);
				else
					p = ++p;
			}
		};

		// Predicates
		auto isResponseMsg = [] (const Packet &packet)
		{
			return packet.header.qr == 1;
		};
		auto isSingleQuery = [] (const Packet &packet)
		{
			return packet.qd.size () == 1;
		};
		auto isStandardQuery = [] (const Packet &packet)
		{
			return packet.header.opCode == 0;
		};
		auto hasv4Query = [] (const Packet &packet)
		{
			return !packet.qd.empty () &&
				packet.qd.front ().type == 1 &&
				packet.qd.front ().clas == 1;
		};

		// Helper
		auto setAnswer = [] (const Packet &packet,
							 const HostsTable &hostsTable)
		{
			// Throwable
			auto dwordIp = hostsTable.at (
				Packet::ParseDomainName (packet.qd.front ().name));

			// Throw here if not found

			auto newPacket = packet;
			newPacket.header.qr = newPacket.header.aa =
				newPacket.header.ra = true;

			if (dwordIp != INADDR_ANY)
			{
				Packet::Resource answer;
				answer.name = packet.qd.front ().name;
				answer.type = 1;
				answer.clas = 1;
				answer.ttl = 120;
				answer.rdData.assign ((uint8_t *) &dwordIp,
					(uint8_t *) (&dwordIp + 1));
				newPacket.an.emplace_front (std::move (answer));
			}
			else
				newPacket.header.rCode = 3;

			return newPacket;
		};

		// Functions
		Connector connector;
		auto fromNameServer = [&] (const Packet &packet)
		{
			try
			{
				const Record &record = records.at (packet.header.id);

				auto newPacket = packet;
				std::tie (newPacket.header.id, newPacket.addr,
						  std::ignore) = record;
				connector.Send (newPacket);

				Logger (logFile) << "Send to " << newPacket;
			}
			// Ignore if timeout...
			catch (const std::exception &) {}
		};

		auto toNameServer = [&] (const Packet &packet)
		{
			clearTimeout (records);

			static uint16_t curId;
			records.emplace (++curId, Record {
				packet.header.id, packet.addr,
				std::chrono::system_clock::now () });

			auto newPacket = packet;
			newPacket.header.id = curId;
			newPacket.addr = saNameServer;
			connector.Send (newPacket);

			Logger (logFile) << "Send to " << newPacket;
		};

		// Callback Delegate
		auto onRecv = [&] (const Packet &packet)
		{
			Logger (logFile) << "Recv from " << packet;

			// Response Message
			if (isResponseMsg (packet))
				return fromNameServer (packet);

			// Non-normal Query
			if (!isSingleQuery (packet) ||
				!isStandardQuery (packet) ||
				!hasv4Query (packet))
				return toNameServer (packet);

			// Search hostsTable for question domain name
			// - Set Anwser and Send back if Found
			// - Forward to name server if NOT Found
			try
			{
				auto newPacket = setAnswer (packet, hostsTable);
				connector.Send (newPacket);

				Logger (logFile) << "Send to " << newPacket;
			}
			catch (const std::exception &) { toNameServer (packet); }
		};

		// Run here
		Logger (logFile) << "Server Started :-)\n\n";
		while (true)
		{
			try { connector.Recv (onRecv); }
			catch (const std::exception &ex)
			{
				Logger (logFile) << "Exception: " << ex.what () << "\n";
			}
		}
	}
	catch (const std::exception &ex)
	{
		std::cerr << ex.what () << std::endl;
	}
	return 1;  // Fatal Error
}
