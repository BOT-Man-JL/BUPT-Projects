
//
// Air Conditioner - Server MVC Service
// BOT Man, 2017
//

#ifndef AC_SERVER_SERVICE_H
#define AC_SERVER_SERVICE_H

#include <exception>
#include <mutex>

#define MAXCLIENT 3
#define THRESHOLD 1.0
#define DEADTIME 3
#define DBNAME "ac.db"

#include "ormlite/ormlite.h"
#include "server-model.h"
#include "time-helper.h"

namespace Air_Conditioner
{
    class LogManager
    {
        struct OnOffEntity
        {
            int id; RoomId room;
            time_t timeBeg, timeEnd;

            ORMAP ("OnOffLog", id, room, timeBeg, timeEnd);
        };

        struct RequestEntity
        {
            int id; RoomId room;
            time_t timeBeg, timeEnd;
            Temperature tempBeg, tempEnd;
            Cost costBeg, costEnd;
            Wind wind;

            ORMAP ("RequestLog", id, room, timeBeg, timeEnd,
                   tempBeg, tempEnd, wind, costBeg, costEnd);
        };

        // In Database
        static BOT_ORM::ORMapper &_mapper ()
        {
            static BOT_ORM::ORMapper mapper (DBNAME);
            static auto hasInit = false;

            if (!hasInit)
            {
                try { mapper.CreateTbl (OnOffEntity {}); }
                catch (...) {}
                try { mapper.CreateTbl (RequestEntity {}); }
                catch (...) {}
                hasInit = true;
            }
            return mapper;
        }

        static const TimePoint &GetStartTime ()
        {
            static const auto startTime = std::chrono::system_clock::now ();
            return startTime;
        }

        static TimePoint GetFakeTime (const TimePoint &realTime)
        {
            const auto &startTime = GetStartTime ();
            std::chrono::duration<double> deltaTime = realTime - startTime;
            return startTime + std::chrono::hours { int (deltaTime.count ()) };
        }

    public:
        static void WriteOnOff (const RoomId &room,
                                const LogOnOff &entry)
        {
            auto begTime = std::chrono::system_clock::to_time_t (
                GetFakeTime (entry.timeBeg));
            auto endTime = std::chrono::system_clock::to_time_t (
                GetFakeTime (entry.timeEnd));

            try
            {
                auto &mapper = _mapper ();
                mapper.Insert (OnOffEntity {
                    0, room, begTime, endTime
                }, false);
            }
            catch (...)
            {
                throw std::runtime_error ("Database is busy");
            }
        }

        static void WriteRequest (const RoomId &room,
                                  const LogRequest &entry)
        {
            auto begTime = std::chrono::system_clock::to_time_t (
                GetFakeTime (entry.timeBeg));
            auto endTime = std::chrono::system_clock::to_time_t (
                GetFakeTime (entry.timeEnd));

            try
            {
                auto &mapper = _mapper ();
                mapper.Insert (RequestEntity {
                    0, room, begTime, endTime,
                    entry.tempBeg, entry.tempEnd,
                    entry.costBeg, entry.costEnd,
                    entry.wind
                }, false);
            }
            catch (...)
            {
                throw std::runtime_error ("Database is busy");
            }
        }

        static std::pair<TimePoint, TimePoint> GetTimeRange ()
        {
            static OnOffEntity onOffEntity;
            static RequestEntity requestEntity;
            static auto field = BOT_ORM::FieldExtractor {
                onOffEntity, requestEntity };

            try
            {
                auto &mapper = _mapper ();
                auto minTime = mapper.Query (onOffEntity)
                    .Aggregate (BOT_ORM::Expression::Min (
                        field (onOffEntity.timeBeg)));
                auto maxTime = mapper.Query (onOffEntity)
                    .Aggregate (BOT_ORM::Expression::Max (
                        field (onOffEntity.timeEnd)));

                auto timeBeg = (minTime == nullptr) ?
                    std::chrono::system_clock::now () :
                    std::chrono::system_clock::from_time_t (minTime.Value ());
                auto timeEnd = (maxTime == nullptr) ?
                    std::chrono::system_clock::now () :
                    std::chrono::system_clock::from_time_t (maxTime.Value ());

                return std::make_pair (
                    std::move (timeBeg) - std::chrono::hours { 24 },
                    std::move (timeEnd) + std::chrono::hours { 24 });
            }
            catch (...)
            {
                throw std::runtime_error ("Database is busy");
            }
        }

        static LogOnOffList GetOnOff (const TimePoint &from,
                                      const TimePoint &to)
        {
            static OnOffEntity entity;
            static auto field = BOT_ORM::FieldExtractor { entity };

            auto begTime = std::chrono::system_clock::to_time_t (
                GetFakeTime (from));
            auto endTime = std::chrono::system_clock::to_time_t (
                GetFakeTime (to));

            try
            {
                auto &mapper = _mapper ();
                auto result = mapper.Query (entity)
                    .Where (
                        field (entity.timeBeg) >= begTime &&
                        field (entity.timeEnd) < endTime
                    )
                    .ToList ();

                LogOnOffList ret;
                for (const auto &entry : result)
                {
                    ret[entry.room].emplace_back (LogOnOff {
                        std::chrono::system_clock::from_time_t (entry.timeBeg),
                        std::chrono::system_clock::from_time_t (entry.timeEnd)
                    });
                }
                return ret;
            }
            catch (...)
            {
                throw std::runtime_error ("Database is busy");
            }
        }

        static LogRequestList GetRequest (const TimePoint &from,
                                          const TimePoint &to)
        {
            static RequestEntity entity;
            static auto field = BOT_ORM::FieldExtractor { entity };

            auto begTime = std::chrono::system_clock::to_time_t (
                GetFakeTime (from));
            auto endTime = std::chrono::system_clock::to_time_t (
                GetFakeTime (to));

            try
            {
                auto &mapper = _mapper ();
                auto result = mapper.Query (entity)
                    .Where (
                        field (entity.timeBeg) >= begTime &&
                        field (entity.timeEnd) < endTime
                    )
                    .ToList ();

                LogRequestList ret;
                for (const auto &entry : result)
                {
                    ret[entry.room].emplace_back (LogRequest {
                        std::chrono::system_clock::from_time_t (entry.timeBeg),
                        std::chrono::system_clock::from_time_t (entry.timeEnd),
                        entry.tempBeg, entry.tempEnd,
                        entry.costBeg, entry.costEnd,
                        entry.wind
                    });
                }
                return ret;
            }
            catch (...)
            {
                throw std::runtime_error ("Database is busy");
            }
        }
    };

    class ConfigManager
    {
        // In Memory
        static ServerInfo &_config ()
        {
            static ServerInfo config;
            return config;
        }

    public:
        static void SetConfig (const ServerInfo &config)
        {
            _config () = config;
        }
        static ServerInfo GetConfig ()
        {
            return _config ();
        }
    };

    class GuestManager
    {
        struct GuestEntity
        {
            RoomId room;
            GuestId guest;
            Energy lastEnergy;
            Cost lastCost;

            ORMAP ("Guest", room, guest, lastEnergy, lastCost);
        };

        // In Database
        static BOT_ORM::ORMapper &_mapper ()
        {
            static BOT_ORM::ORMapper mapper (DBNAME);
            static auto hasInit = false;

            if (!hasInit)
            {
                try { mapper.CreateTbl (GuestEntity {}); }
                catch (...) {}
                hasInit = true;
            }
            return mapper;
        }

    public:
        static void AddGuest (const GuestInfo &guest)
        {
            try
            {
                auto &mapper = _mapper ();
                mapper.Insert (GuestEntity {
                    guest.room, guest.guest,
                    Energy { 0 }, Cost { 0 }
                });
            }
            catch (...)
            {
                throw std::runtime_error (
                    "The room has already been registered");
            }
        }

        static void RemoveGuest (const RoomId &room)
        {
            static GuestEntity entity;
            auto &mapper = _mapper ();

            entity.room = room;
            mapper.Delete (entity);
        }

        static void AuthGuest (const GuestInfo &guest)
        {
            static GuestEntity entity;
            static auto field = BOT_ORM::FieldExtractor { entity };

            auto notFound = true;
            try
            {
                auto &mapper = _mapper ();
                notFound = mapper.Query (entity)
                    .Where (
                        field (entity.room) == guest.room &&
                        field (entity.guest) == guest.guest)
                    .ToList ().empty ();
            }
            catch (...)
            {
                throw std::runtime_error ("Database is busy");
            }

            if (notFound)
                throw std::runtime_error ("Invalid Room ID or Guest ID");
        }

        static std::pair<Energy, Cost> ReadLastState (const RoomId &room)
        {
            static GuestEntity entity;
            static auto field = BOT_ORM::FieldExtractor { entity };

            auto &mapper = _mapper ();
            auto guestFound = mapper.Query (entity)
                .Where (field (entity.room) == room)
                .ToList ();

            if (guestFound.empty ())
                throw std::runtime_error ("Invalid Room ID or Guest ID");

            auto energy = guestFound.front ().lastEnergy;
            auto cost = guestFound.front ().lastCost;
            return std::make_pair (energy, cost);
        }

        static void WriteLastState (const RoomId &room,
                                    const Energy &energy,
                                    const Cost &cost)
        {
            static GuestEntity entity;
            static auto field = BOT_ORM::FieldExtractor { entity };

            auto &mapper = _mapper ();
            mapper.Update (
                entity,
                (field (entity.lastEnergy) = energy) &&
                (field (entity.lastCost) = cost),
                field (entity.room) == room);
        }

        static std::list<GuestInfo> GetGuestList ()
        {
            static GuestEntity entity;
            static auto field = BOT_ORM::FieldExtractor { entity };

            auto &mapper = _mapper ();
            auto list = mapper.Query (entity).ToList ();

            std::list<GuestInfo> ret;
            for (auto &entry : list)
            {
                ret.emplace_back (GuestInfo {
                    std::move (entry.room),
                    std::move (entry.guest) });
            }
            return ret;
        }
    };

    class ScheduleHelper
    {
        static bool HasWind (const ClientState &state,
                             const ServerInfo &config)
        {
            // Case: Server Off
            if (!config.isOn) return false;

            // Case: Enough already
            if (config.mode == 0 &&
                state.current <= state.target) return false;
            if (config.mode == 1 &&
                state.current >= state.target) return false;

            // Case: Need to send wind
            if (config.mode == 0 &&
                state.current - state.target >= THRESHOLD) return true;
            if (config.mode == 1 &&
                state.target - state.current >= THRESHOLD) return true;

            // Case: Keep the state
            return state.hasWind;
        }

    public:
        static void Schedule (ClientList &clients,
                              const ServerInfo &config)
        {
            auto count = 0;
            std::unordered_map<RoomId, bool> hasWindList;
            for (auto &client : clients)
                if (HasWind (client.second, config) && count < 3)
                {
                    hasWindList[client.first] = true;
                    ++count;
                }
                else
                    hasWindList[client.first] = false;

            for (auto &client : clients)
                client.second.hasWind = hasWindList[client.first];
        }
    };

    class PulseHelper
    {
    public:
        static void HandleReqBeg (const TimePoint &time,
                                  ClientState &state)
        {
            state.lastRequest.timeBeg = time;
            state.lastRequest.tempBeg = state.current;
            state.lastRequest.costBeg = state.cost;
            state.lastRequest.wind = state.wind;
        }

        static void HandleReqEnd (const RoomId &room,
                                  const TimePoint &time,
                                  ClientState &state)
        {
            state.lastRequest.timeEnd = time;
            state.lastRequest.tempEnd = state.current;
            state.lastRequest.costEnd = state.cost;

            // hack the code here :-)
            // TODO: to fix
            if (state.lastRequest.wind != 0)
                LogManager::WriteRequest (room, state.lastRequest);
        }

        static void HandleTurnOn (const TimePoint &time,
                                  ClientState &state)
        {
            state.lastOnOff.timeBeg = time;
        }

        static void HandleTurnOff (const RoomId &room,
                                   const TimePoint &time,
                                   ClientState &state)
        {
            if (state.lastRequest.costBeg != state.cost)
                HandleReqEnd (room, time, state);

            state.lastOnOff.timeEnd = state.pulse;

            // Log down previous state of client
            GuestManager::WriteLastState (room, state.energy, state.cost);

            LogManager::WriteOnOff (room, state.lastOnOff);
        }

        static void CheckAlive (ClientList &clients)
        {
            auto now = std::chrono::system_clock::now ();
            auto deadTime = now - std::chrono::seconds { DEADTIME };

            for (auto p = clients.begin (); p != clients.end ();)
                if (p->second.pulse < deadTime)
                {
                    HandleTurnOff (p->first, now, p->second);
                    p = clients.erase (p);
                }
                else ++p;
        }
    };

    class ScheduleManager
    {
        // In Memory
        static ClientList &_clients ()
        {
            static ClientList clients;
            return clients;
        }
        static std::mutex &_clientsMtx ()
        {
            static std::mutex mtx;
            return mtx;
        }

        // Helper
        static ClientInfo StateToInfo (const ClientState &state)
        {
            return ClientInfo {
                state.hasWind, state.energy, state.cost
            };
        }

    public:
        // Called by Protocol Controller
        static ClientInfo AddClient (const GuestInfo &room)
        {
            // Check Alive
            std::lock_guard<std::mutex> lg (_clientsMtx ());
            auto &clients = _clients ();
            PulseHelper::CheckAlive (clients);

            // Login already
            if (clients.find (room.room) != clients.end ())
                throw std::runtime_error ("Login already");

            // Restore previous state of client
            auto lastState = GuestManager::ReadLastState (room.room);

            // New Login
            auto now = std::chrono::system_clock::now ();
            auto state = ClientState {
                room.guest, Temperature { 0 }, Temperature { 0 },
                Wind { 0 }, false, lastState.first, lastState.second,
                now, LogOnOff {}, LogRequest {}
            };

            PulseHelper::HandleTurnOn (now, state);
            clients.emplace (room.room, std::move (state));

            return StateToInfo (state);
        }

        // Called by View Controller
        static void RemoveClient (const RoomId &room)
        {
            try
            {
                std::lock_guard<std::mutex> lg (_clientsMtx ());
                auto &clients = _clients ();

                auto &state = clients.at (room);  // throw
                auto now = std::chrono::system_clock::now ();

                PulseHelper::HandleTurnOff (room, now, state);
                clients.erase (room);
            }
            catch (...) {}
        }

        // Called by Protocol Controller
        static ClientInfo Pulse (const RoomRequest &req)
        {
            // Check Alive
            std::lock_guard<std::mutex> lg (_clientsMtx ());
            auto &clients = _clients ();
            PulseHelper::CheckAlive (clients);

            // Logout already
            auto pState = clients.find (req.room);
            if (pState == clients.end ())
                throw std::runtime_error ("Logout already");
            auto &state = clients.at (req.room);

            // Record states before scheduling
            auto isChanged =
                state.target != req.target ||
                state.wind != req.wind;
            auto hasWindBefore = state.hasWind;

            // Update Client State
            state.current = req.current;
            state.target = req.target;
            state.wind = req.wind;

            // Schedule
            const auto &config = ConfigManager::GetConfig ();
            ScheduleHelper::Schedule (clients, config);

            // Get Delta Time and Pulse
            auto now = std::chrono::system_clock::now ();
            std::chrono::duration<double> deltaTime = now - state.pulse;
            state.pulse = now;

            // Handle Beg/End Request
            if (!hasWindBefore && state.hasWind)
                PulseHelper::HandleReqBeg (now, state);
            else if (hasWindBefore && !state.hasWind)
                PulseHelper::HandleReqEnd (req.room, now, state);
            else if (isChanged)
            {
                PulseHelper::HandleReqEnd (req.room, now, state);
                PulseHelper::HandleReqBeg (now, state);
            }

            // Calc Energy and Cost
            if (state.hasWind)
            {
                // Get Delta Energy
                auto deltaEnergy = Energy { deltaTime.count () / 60.0 };
                if (state.wind == 1)
                    deltaEnergy = deltaEnergy * 0.8;
                else if (state.wind == 3)
                    deltaEnergy = deltaEnergy * 1.3;

                // Add up energy
                state.energy += deltaEnergy;
                state.cost = state.energy * 5;
            }

            return StateToInfo (state);
        }

        // Called by View Controller
        static ClientList GetClientList ()
        {
            // Check Alive
            std::lock_guard<std::mutex> lg (_clientsMtx ());
            auto &clients = _clients ();
            PulseHelper::CheckAlive (clients);

            return clients;
        }
    };
}

#endif // !AC_SERVER_SERVICE_H