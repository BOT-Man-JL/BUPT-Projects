﻿
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

    class Packet
    {
        using StrMap = std::unordered_map<uint16_t, std::string>;
        static const StrMap &TypeStrs ()
        {
            static const StrMap typeStrs
            {
                { 1, "A" },
                { 2, "NS" },
                { 5, "CNAME" },
                { 6, "SOA" },
                { 12, "PTR" },
                { 15, "MX" },
                { 16, "TXT" },
                { 28, "AAAA" }
            };
            return typeStrs;
        }

        static const StrMap &ClasStrs ()
        {
            static const StrMap clasStrs
            {
                { 1, "IN" }
            };
            return clasStrs;
        }

        static const std::string &UnknownStr ()
        {
            static const std::string ret { "???" };
            return ret;
        }

        static auto GetDomainName (const uint8_t *buf,
                                   const uint8_t *pbFrom,
                                   std::string &name)
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
        }

        static auto ParseDomainName (const std::string &rawName)
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

    public:
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

            std::string nameStr () const
            {
                return ParseDomainName (name);
            }
            const std::string &typeStr () const
            {
                try { return TypeStrs ().at (type); }
                catch (...) { return UnknownStr (); }
            }
            const std::string &clasStr () const
            {
                try { return ClasStrs ().at (clas); }
                catch (...) { return UnknownStr (); }
            }
        };

        struct Resource
        {
            std::string name;
            uint16_t type;   // 16 bit
            uint16_t clas;   // 16 bit
            uint32_t ttl;    // 32 bit
            // rdLen - 16 bit
            std::vector<uint8_t> rdData; // rdLen

            std::string nameStr () const
            {
                return ParseDomainName (name);
            }
            const std::string &typeStr () const
            {
                try { return TypeStrs ().at (type); }
                catch (...) { return UnknownStr (); }
            }
            const std::string &clasStr () const
            {
                try { return ClasStrs ().at (clas); }
                catch (...) { return UnknownStr (); }
            }

            void FixData (const uint8_t *buf)
            {
                switch (type)
                {
                case 2:
                case 5:
                case 12:
                {
                    std::string domainName;
                    GetDomainName (buf, rdData.data (), domainName);
                    rdData.assign (domainName.begin (), domainName.end ());
                    break;
                }
                case 6:
                {
                    std::string domainName;
                    auto pData = rdData.data ();
                    pData += GetDomainName (buf, pData, domainName);
                    pData += GetDomainName (buf, pData, domainName);
                    domainName.append (pData, pData + rdData.size ());
                    rdData.assign (domainName.begin (), domainName.end ());
                    break;
                }
                default: break;
                }
            }

            void PrintData (std::ostream &os) const
            {
                switch (type)
                {
                case 1:
                {
                    os << " Addr: [";
                    for (auto p = rdData.begin (); ;)
                    {
                        os << (uint16_t) *p;
                        if (++p == rdData.end ()) break;
                        os << ".";
                    }
                    os << "]";
                    break;
                }
                case 28:
                {
                    auto count = 0;
                    os << " Addr: [" << std::hex;
                    for (const auto byteVal : rdData)
                    {
                        os << std::setw (2) << std::setfill ('0')
                            << std::right << (uint16_t) byteVal;
                        if (count % 2 && count != 15) os << ":";
                        ++count;
                    }
                    os << std::dec << "]";
                    break;
                }
                case 2:
                case 5:
                case 12:
                {
                    os << " Name: [" << Packet::ParseDomainName (
                        std::string { rdData.begin (), rdData.end () })
                        << "]";
                    break;
                }
                case 6:
                {
                    auto p = rdData.begin ();
                    for (; p != rdData.end () && *p != 0; ++p);
                    os << " NName: [" << Packet::ParseDomainName (
                        std::string { rdData.begin (), p });
                    auto q = ++p;
                    for (; p != rdData.end () && *p != 0; ++p);
                    os << "] RName: [" << Packet::ParseDomainName (
                        std::string { q, p });
                    uint32_t *pd = (uint32_t *) &(*++p);  // Dangerous
                    os << "] Serial: " << ntohl (*pd);
                    os << " Refresh: " << ntohl (*++pd);
                    os << " Retry: " << ntohl (*++pd);
                    os << " Expire: " << ntohl (*++pd);
                    os << " Mininum: " << ntohl (*++pd);
                    break;
                }
                default:
                {
                    auto count = rdData.size ();
                    os << " rdData: [" << std::hex;
                    for (const auto byteVal : rdData)
                    {
                        os << std::setw (2) << std::setfill ('0')
                            << std::right << (uint16_t) byteVal;
                        if (--count != 0) os << " ";
                    }
                    os << std::dec << "]";
                    break;
                }
                }
            }
        };

        Packet (const sockaddr_in &_addr, const uint8_t *buf, int bufLen)
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
                auto domainNameLen =
                    GetDomainName (buf, pbBuf, question.name);
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
                    auto domainNameLen =
                        GetDomainName (buf, pbBuf, resource.name);
                    pbBuf += domainNameLen;

                    // Retrieve fields
                    getField (resource.type, ntohs);
                    getField (resource.clas, ntohs);
                    getField (resource.ttl, ntohl);

                    // Retrieve data
                    uint16_t rdLen;
                    getField (rdLen, ntohs);
                    resource.rdData.assign (pbBuf, pbBuf + rdLen);
                    resource.FixData (buf);
                    pbBuf += rdLen;

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

            setRes (an);
            setRes (ns);
            setRes (ar);
        }

        sockaddr_in addr;
        Header header;
        std::list<Question> qd;
        std::list<Resource> an;
        std::list<Resource> ns;
        std::list<Resource> ar;
    };

    class StrictPacket
    {
        Packet _packet;
        std::vector<uint8_t> _buffer;

    public:
        StrictPacket (const sockaddr_in &addr, const uint8_t *buf, int bufLen)
            : _packet (addr, buf, bufLen), _buffer (buf, buf + bufLen)
        {}

        void SetAddr (const sockaddr_in &addr)
        {
            _packet.addr = addr;
        }
        void SetId (const uint16_t id)
        {
            _packet.header.id = id;
            uint16_t *pwId = (uint16_t *) _buffer.data ();
            *pwId = htons (id);
        }

        const Packet &GetPacket () const
        {
            return _packet;
        }
        const std::vector<uint8_t> &GetBuffer () const
        {
            return _buffer;
        }
    };

    std::ostream &operator << (std::ostream &os,
                               const sockaddr_in &addr)
    {
        constexpr int BUF_SIZE = 64;
        char szIp[BUF_SIZE];

        if (!inet_ntop (
            AF_INET, (const void *) &addr.sin_addr,
            szIp, BUF_SIZE))
            strcpy (szIp, "Invalid IP");
        return os << "[" << szIp << ":"
            << ntohs (addr.sin_port) << "]\n";
    }

    std::ostream &operator << (std::ostream &os,
                               const Packet::Header &header)
    {
        return os << "ID: " << std::setw (4) << std::setfill ('0')
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
    }

    std::ostream &operator << (std::ostream &os,
                               const Packet::Question &ques)
    {
        return os << " - Name: [" << ques.nameStr ()
            << "] Type: " << ques.typeStr ()
            << " Class: " << ques.clasStr ()
            << std::endl;
    }

    std::ostream &operator << (std::ostream &os,
                               const Packet::Resource &res)
    {
        os << " - Name: [" << res.nameStr ()
            << "] Type: " << res.typeStr ()
            << " Class: " << res.clasStr ()
            << " TTL: " << res.ttl;
        res.PrintData (os);
        return os << std::endl;
    }

    std::ostream &operator << (std::ostream &os,
                               const Packet &packet)
    {
        auto printFieldList = [] (
            std::ostream &os,
            const char *fieldName,
            const auto &list)
        {
            if (list.empty ()) return;

            os << fieldName << " has " << list.size ()
                << " fields:" << std::endl;
            for (const auto &res : list)
                os << res;
        };

        os << packet.addr << packet.header;
        printFieldList (os, "QD", packet.qd);
        printFieldList (os, "AN", packet.an);
        printFieldList (os, "NS", packet.ns);
        printFieldList (os, "AR", packet.ar);
        return os;
    }

    std::ostream &operator << (std::ostream &os,
                               const StrictPacket &packet)
    {
        os << packet.GetPacket ();
        auto count = packet.GetBuffer ().size ();
        os << "Buffer: [" << std::hex;
        for (const auto byteVal : packet.GetBuffer ())
        {
            os << std::setw (2) << std::setfill ('0')
                << std::right << (uint16_t) byteVal;
            if (--count != 0) os << " ";
        }
        return os << std::dec << "]\n";
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

        void Recv (std::function<void (const StrictPacket &)> callback)
        {
            uint8_t buf[PACKET_BUF_SIZE];
            sockaddr_in saFrom;
            socklen_t saLen = sizeof (sockaddr_in);

            int bufLen = recvfrom (_sock, (char *) buf, PACKET_BUF_SIZE,
                                   0, (sockaddr *) &saFrom, &saLen);
            if (bufLen <= 0)
                ThrowRuntimeError ("recvfrom");

            callback (StrictPacket (saFrom, buf, bufLen));
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

        void Send (const StrictPacket &packet)
        {
            auto bufLen = (int) packet.GetBuffer ().size ();
            auto buf = (const char *) packet.GetBuffer ().data ();
            if (sendto (_sock, buf, bufLen,
                        0, (const sockaddr *) &packet.GetPacket ().addr,
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
        auto isStandardQuery = [] (const Packet &packet)
        {
            return packet.header.opCode == 0;
        };
        auto isAQuery = [] (const Packet::Question &ques)
        {
            return ques.type == 1 && ques.clas == 1;
        };

        // Helpers
        auto setAnswer = [] (Packet &packet,
                             const std::string &name,
                             uint32_t dwordIp)
        {
            packet.header.qr =
                packet.header.aa =
                packet.header.ra = true;

            if (dwordIp != INADDR_ANY)
            {
                Packet::Resource answer;
                answer.name = name;
                answer.type = 1;
                answer.clas = 1;
                answer.ttl = 120;
                answer.rdData.assign ((uint8_t *) &dwordIp,
                    (uint8_t *) (&dwordIp + 1));
                packet.an.emplace_back (std::move (answer));
            }
            else
                packet.header.rCode = 3;
        };

        Connector connector;
        auto sendPacket = [&] (const Packet &packet)
        {
            connector.Send (packet);
            Logger (logFile) << "Send to " << packet << "\n";
        };
        auto sendStrictPacket = [&] (StrictPacket packet,
                                     uint16_t id,
                                     const sockaddr_in &addr)
        {
            packet.SetId (id);
            packet.SetAddr (addr);
            connector.Send (packet);
            Logger (logFile) << "Send to " << packet << "\n";
        };

        // Functions
        auto fromNameServer = [&] (const StrictPacket &packet)
        {
            try
            {
                uint16_t id;
                sockaddr_in addr;
                std::tie (id, addr, std::ignore) =
                    records.at (packet.GetPacket ().header.id);
                sendStrictPacket (packet, id, addr);
            }
            // Ignore if timeout...
            catch (const std::exception &) {}
        };

        auto toNameServer = [&] (const StrictPacket &packet)
        {
            // Discard this packet if table is full
            if (!clearTimeout (records, config.timeOut))
                return;

            static uint16_t curId;  // Auto loop back
            records.emplace (
                ++curId, Record {
                packet.GetPacket ().header.id,
                packet.GetPacket ().addr,
                std::chrono::system_clock::now () });

            sendStrictPacket (packet, curId, saNameServer);
        };

        // Callback Delegates
        auto onRecv = [&] (const StrictPacket &packet)
        {
            Logger (logFile) << "Recv from " << packet << "\n";

            // Response Message
            if (isResponseMsg (packet.GetPacket ()))
                return fromNameServer (packet);

            // Non-standard Query
            if (!isStandardQuery (packet.GetPacket ()))
                return toNameServer (packet);

            // Search hostsTable for question domain name
            // - Set Anwser and Send back if Found
            // - Forward to name server if NOT Found
            try
            {
                auto newPacket = packet.GetPacket ();
                for (const auto &ques : packet.GetPacket ().qd)
                {
                    if (!isAQuery (ques)) throw std::exception {};
                    const auto &dwordIp = hostsTable.at (
                        ques.nameStr ());
                    setAnswer (newPacket, ques.name, dwordIp);
                }
                sendPacket (newPacket);
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
