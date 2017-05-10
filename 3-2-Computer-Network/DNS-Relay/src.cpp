
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
#include <errno.h>

#ifndef WIN32
// Substitute on *NIX
int WSAGetLastError ()
{
    return errno;
}
#endif

#define HOSTS_FILENAME "dnsrelay.txt"
#define NAMESERVER_IP "202.106.0.20"
#define TIMEOUTSTR "5"
#define TIMEOUT 5

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
            _os << std::put_time (std::localtime (&t), "%c") << " - ";
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

            std::string timeOutStr;
            const std::unordered_map<std::string, std::string &> mapper
            {
                { "-o", logFilename },
                { "-n", nameserverIp },
                { "-h", hostsFilename },
                { "-t", timeOutStr }
            };

            for (int i = 0; i < argc / 2; i++)
            {
                try { mapper.at (argv[i * 2 + 1]) = argv[i * 2 + 2]; }
                catch (const std::exception &) {}
            }

            if (timeOutStr.empty ()) return;
            try { timeOut = std::chrono::seconds (std::stol (timeOutStr)); }
            catch (...) { throw std::runtime_error ("Bad timeout input"); }
        }

        static const char *GetPrompt ()
        {
            return "Args: [-o <log-file-name> | -n <name-server-ip> | "
                "-h <hosts-file-name> | -t <timeout-seconds>]\n"
                "Default Config: -o <stdout> -n " NAMESERVER_IP
                " -h " HOSTS_FILENAME " -t " TIMEOUTSTR;
        }

        std::string logFilename;
        std::string nameserverIp = NAMESERVER_IP;
        std::string hostsFilename = HOSTS_FILENAME;
        std::chrono::seconds timeOut = std::chrono::seconds (TIMEOUT);
    };

    std::ostream &operator << (std::ostream &os, const Config &config)
    {
        return os << "-o " << (config.logFilename.empty () ?
                               "<stdout>" : config.logFilename.c_str ())
            << " -n " << config.nameserverIp
            << " -h " << config.hostsFilename
            << " -t " << config.timeOut.count ();
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
            auto checkLen = [&bufLen] (size_t size)
            {
                bufLen -= (int) size;
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

            auto getDomainName = [buf] (uint8_t *pbFrom, std::string &name)
            {
                auto pbBuf = pbFrom;
                auto isCompressed = false;
                auto szSeg = (const char *) pbBuf;

                while (*szSeg)
                {
                    // Handle compressed domain name
                    if (*szSeg & 0xC0)
                    {
                        uint16_t *pwBuf = (uint16_t *) szSeg;
                        uint16_t offset = ntohs (*pwBuf) & 0x3FFF;
                        szSeg = (const char *) buf + offset;

                        if (!isCompressed) pbBuf += sizeof (uint16_t);
                        isCompressed = true;
                    }

                    uint8_t segSize = *(uint8_t *) szSeg + 1;
                    name.append (szSeg, segSize);
                    szSeg += segSize;
                    if (!isCompressed) pbBuf = (uint8_t *) szSeg;
                }
                name.append (1u, '\0');

                if (!isCompressed) ++pbBuf;
                return pbBuf - pbFrom;
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
                auto domainNameLen = getDomainName (pbBuf, question.name);
                pbBuf += domainNameLen;

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
                    auto domainNameLen = getDomainName (pbBuf, resource.name);
                    pbBuf += domainNameLen;

                    // Retrieve fields
                    getField (resource.type, ntohs);
                    getField (resource.clas, ntohs);
                    getField (resource.ttl, ntohl);

                    // Retrieve data
                    uint16_t rdLen;
                    getField (rdLen, ntohs);
                    resource.rdData.assign (pbBuf, pbBuf + rdLen);
                    pbBuf += rdLen;

                    // TODO: support more protocols
                    // Handle compression in rdData
                    if (resource.type == 2 || resource.type == 5)
                    {
                        auto pFrom = resource.rdData.data ();
                        std::string domainName;
                        getDomainName (pFrom, domainName);
                        resource.rdData.assign (domainName.begin (), domainName.end ());
                    }

                    // Validation
                    checkLen (domainNameLen + sizeof (uint32_t) +
                              3 * sizeof (uint16_t) + rdLen);

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
            auto pbBuf = buf;
            auto oldLen = bufLen;
            bufLen = 0;

            auto checkLen = [&oldLen, &bufLen] (size_t size)
            {
                oldLen -= (int) size;
                if (oldLen < 0) throw std::runtime_error ("Bad buffer size");
                bufLen += (int) size;
            };

            auto setField = [&pbBuf, &checkLen] (auto val, auto fn)
            {
                decltype (val) *pBuf = (decltype (val) *) pbBuf;
                *pBuf = decltype (val) (fn (val));
                pbBuf += sizeof (val);
                checkLen (sizeof (val));
            };

            auto setBytes = [&pbBuf, &checkLen] (auto *pSrc, size_t size)
            {
                memcpy ((void *) pbBuf, (void *) pSrc, size);
                pbBuf += size;
                checkLen (size);
            };

            // Get flags
            uint16_t flags = 0;
            if (header.qr) flags |= 0x8000;
            flags |= header.opCode << 11;
            if (header.aa) flags |= 0x0400;
            if (header.tc) flags |= 0x0200;
            if (header.rd) flags |= 0x0100;
            if (header.ra) flags |= 0x0080;
            flags |= header.zero << 4;
            flags |= header.rCode << 0;

            // Set Header
            setField (header.id, htons);
            setField (flags, htons);
            setField ((uint16_t) qd.size (), htons);
            setField ((uint16_t) an.size (), htons);
            setField ((uint16_t) ns.size (), htons);
            setField ((uint16_t) ar.size (), htons);

            // Set Questions
            for (const auto &question : qd)
            {
                setBytes (question.name.c_str (), question.name.size ());
                setField (question.type, htons);
                setField (question.clas, htons);
            }

            // Set Resources
            auto setRes = [&] (const std::list<Resource> &resources)
            {
                for (const auto &resource : resources)
                {
                    setBytes (resource.name.c_str (),
                              resource.name.size ());
                    setField (resource.type, htons);
                    setField (resource.clas, htons);
                    setField (resource.ttl, htonl);
                    setField ((uint16_t) resource.rdData.size (), htons);
                    setBytes (resource.rdData.data (),
                              resource.rdData.size ());
                }
            };

            auto sizean = an.size ();
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
                << " RCODE: " << (uint16_t) header.rCode
                << std::endl;
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
                << " rdData: [ ";
            if (res.type == 1)  // A
            {
                auto p = res.rdData.begin ();
                while (true)
                {
                    os << (uint16_t) *p;
                    if (++p == res.rdData.end ()) break;
                    os << ".";
                }
                os << " ";
            }
            else if (res.type == 28)  // AAAA
            {
                auto count = 0;
                os << std::hex;
                for (const auto byteVal : res.rdData)
                {
                    os << std::setw (2) << std::setfill ('0')
                        << std::right << (uint16_t) byteVal;
                    if (count % 2)
                        if (count != 15) os << ":";
                        else os << " ";
                    ++count;
                }
                os << std::dec;
            }
            else if (res.type == 2 || res.type == 5)  // NS, CNAME
            {
                os << Packet::ParseDomainName (std::string {
                    res.rdData.begin (), res.rdData.end () }) << " ";
            }
            else  // Unknown
            {
                os << std::hex;
                for (const auto byteVal : res.rdData)
                    os << std::setw (2) << std::setfill ('0')
                    << std::right << (uint16_t) byteVal << " ";
                os << std::dec;
            }
            os << "]" << std::endl;
        };

        auto printFieldList = [] (
            std::ostream &os,
            const char *fieldName,
            const auto &list,
            const auto &printField)
        {
            if (list.empty ()) return;

            os << fieldName << " has " << list.size ()
                << " fields:" << std::endl;
            for (const auto &res : list)
                printField (os, res);
        };

        printAddr (os, packet.addr);
        printHeader (os, packet.header);
        printFieldList (os, "QD", packet.qd, printQuestion);
        printFieldList (os, "AN", packet.an, printResource);
        printFieldList (os, "NS", packet.ns, printResource);
        printFieldList (os, "AR", packet.ar, printResource);

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
                throw std::runtime_error ("socket() failed");

            // Bind the 'sock at 'PORT
            sockaddr_in saBind;
            saBind.sin_family = AF_INET;
            saBind.sin_addr.s_addr = INADDR_ANY;
            saBind.sin_port = htons (PORT);

            if (bind (_sock, (sockaddr *) &saBind,
                      sizeof (sockaddr_in)) < 0)
                ThrowRuntimeError ("bind");
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
                ThrowRuntimeError ("recvfrom");

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
                ThrowRuntimeError ("send");
        }

    private:
        SOCKET _sock;

        void ThrowRuntimeError (const char *type)
        {
            std::ostringstream ostrs;
            ostrs << type << "() failed (" << WSAGetLastError ()
                << "): " << strerror (errno);
            throw std::runtime_error (ostrs.str ());
        }
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
        // TODO: Support multi-host (multimap) (not in requirement)
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
            std::unordered_map<uint16_t, Record> &records,
            const std::chrono::seconds &timeOut)
        {
            for (auto p = records.begin (); p != records.end ();)
            {
                TimePoint timePoint;
                std::tie (std::ignore, std::ignore, timePoint) = p->second;
                if (std::chrono::system_clock::now () - timePoint > timeOut)
                    p = records.erase (p);
                else
                    p = ++p;
            }

            return records.size () < 0x10000;
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

        // Helpers
        auto fromRecord = [] (Packet packet,
                              const Record &record)
        {
            std::tie (packet.header.id, packet.addr,
                      std::ignore) = record;
            return packet;
        };
        auto toRecord = [] (const Packet &packet)
        {
            return Record { packet.header.id, packet.addr,
                std::chrono::system_clock::now () };
        };
        auto setTarget = [] (Packet packet, uint16_t id,
                             const sockaddr_in &sa)
        {
            packet.header.id = id;
            packet.addr = sa;
            return packet;
        };
        auto setAnswer = [] (Packet packet,
                             uint32_t dwordIp)
        {
            packet.header.qr =
                packet.header.aa =
                packet.header.ra = true;

            if (dwordIp != INADDR_ANY)
            {
                Packet::Resource answer;
                answer.name = packet.qd.front ().name;
                answer.type = 1;
                answer.clas = 1;
                answer.ttl = 120;
                answer.rdData.assign ((uint8_t *) &dwordIp,
                    (uint8_t *) (&dwordIp + 1));
                packet.an.emplace_front (std::move (answer));
            }
            else
                packet.header.rCode = 3;

            return packet;
        };

        Connector connector;
        auto sendPacket = [&] (const Packet &packet)
        {
            connector.Send (packet);
            Logger (logFile) << "Send to " << packet;
        };

        // Functions
        auto fromNameServer = [&] (const Packet &packet)
        {
            try
            {
                sendPacket (fromRecord (packet,
                                        records.at (packet.header.id)));
            }
            // Ignore if timeout...
            catch (const std::exception &) {}
        };

        auto toNameServer = [&] (const Packet &packet)
        {
            // Discard this packet if table is full
            if (!clearTimeout (records, config.timeOut))
                return;

            static uint16_t curId;  // Auto loop back
            records.emplace (++curId,
                             toRecord (packet));
            sendPacket (setTarget (
                packet, curId, saNameServer));
        };

        // Callback Delegates
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
                sendPacket (setAnswer (packet,
                                       hostsTable.at (
                                           Packet::ParseDomainName (
                                               packet.qd.front ().name))));
            }
            catch (const std::exception &) { toNameServer (packet); }
        };

        auto onException = [&] (const std::exception &ex)
        {
            Logger (logFile) << "Exception: " << ex.what () << "\n";
        };

        // Run here
        Logger (logFile) << "Server Started :-)\n\n";
        while (true)
        {
            try { connector.Recv (onRecv); }
            catch (const std::exception &ex) { onException (ex); }
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what () << std::endl;
    }
    return 1;  // Fatal Error
}
