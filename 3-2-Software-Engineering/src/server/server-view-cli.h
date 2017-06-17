
//
// Air Conditioner - Server MVC View (CLI View)
// BOT Man, 2017
//

#ifndef AC_SERVER_VIEW_CLI_H
#define AC_SERVER_VIEW_CLI_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <thread>

#include "server-view.h"
#include "../common/cli-helper.h"
#include "log-helper.h"

#define ONOFFLOGFILE "on-off-log.csv"
#define REQUESTLOGFILE "request-log.csv"

namespace Air_Conditioner
{
    class WelcomeViewCLI : public WelcomeView
    {
        ViewType _GetViewType () const
        {
            static std::unordered_map<std::string, ViewType> viewStr
            {
                { "config", ViewType::ConfigView },
                { "guest", ViewType::GuestView },
                { "log", ViewType::LogView },
                { "client", ViewType::ClientView },
                { "quit", ViewType::Quit }
            };

            while (true)
            {
                std::string cmd;
                std::cout << "$ ";
                if (!InputHelper::GetByRef (cmd))
                    continue;

                try { return viewStr.at (cmd); }
                catch (...) { std::cout << "Invalid Command\n"; }
            }
        }

        OnNav _onNav;

    public:
        WelcomeViewCLI (OnNav &&onNav)
            : _onNav (onNav)
        {}

        virtual void Show () override
        {
            std::cout << "Welcome to Air Conditioner System :-)\n"
                "What you wanna do? Enter command to goto the page\n"
                " - 'config': Config the master machine\n"
                " - 'guest': Manage the guest list of the rooms\n"
                " - 'log': View logs\n"
                " - 'client': View logs\n"
                " - 'quit': Quit\n";
            if (_onNav)
                _onNav (_GetViewType ());
        }
    };

    class ConfigViewCLI : public ConfigView
    {
        void _PrintInfo () const
        {
            std::cout << "Current Config:"
                << "\n - Master is " << (_config.isOn ? "ON" : "OFF")
                << "\n - Mode: " << (!_config.mode ? "Summer" : "Winter")
                << "\n";
        }

        ServerInfo _config;
        OnSet _onSet;
        OnBack _onBack;

    public:
        ConfigViewCLI (const ServerInfo &config,
                       OnSet &&onSet, OnBack &&onBack)
            : _config (config), _onSet (onSet), _onBack (onBack)
        {}

        virtual void Show () override
        {
            std::cout << "\nWhat you wanna do?"
                " Enter command to update config or back\n"
                " - 'on' / 'off' to turn on/off the master\n"
                " - 'summer' / 'winter' to update working mode\n"
                " - Press Enter to go back to welcome page\n";
            _PrintInfo ();

            while (true)
            {
                std::string cmd;
                std::cout << "$ ";
                if (!InputHelper::GetByRef (cmd))
                {
                    if (_onBack) _onBack ();
                    std::cout << std::endl;
                    return;
                }

                if (cmd == "on")
                    _config.isOn = true;
                else if (cmd == "off")
                    _config.isOn = false;
                else if (cmd == "summer")
                    _config.mode = 0;
                else if (cmd == "winter")
                    _config.mode = 1;
                else
                {
                    std::cout << "Invalid Command\n";
                    continue;
                }

                if (_onSet) _onSet (_config);
                std::cout << "Updated\n";
                _PrintInfo ();
            }
        }
    };

    class GuestViewCLI : public GuestView
    {
        void _List () const
        {
            if (_list.empty ())
            {
                std::cout << "No guest is registered...\n";
                return;
            }

            std::cout << "Guests on the list:\n";
            for (const auto &guest : _list)
            {
                std::cout
                    << " - Room: " << guest.room
                    << " Guest: " << guest.guest
                    << "\n";
            }
        }

        void _Add ()
        {
            try
            {
                auto roomId = InputHelper::Get<RoomId> ("Room ID");
                auto guestId = InputHelper::Get<GuestId> ("Guest ID");
                auto guest = GuestInfo {
                    std::move (roomId), std::move (guestId)
                };

                if (_onAdd) _onAdd (guest);  // throw here
                _list.emplace_back (std::move (guest));
                std::cout << "Added\n";
            }
            catch (const std::exception &ex)
            {
                std::cout << ex.what () << std::endl;
            }
        }

        void _Del ()
        {
            try
            {
                auto room = InputHelper::Get<RoomId> ("Room ID");
                auto hasFound = false;
                for (auto p = _list.begin (); p != _list.end ();)
                {
                    if (p->room == room)
                    {
                        p = _list.erase (p);
                        hasFound = true;
                    }
                    else ++p;
                }
                if (!hasFound)
                    throw std::runtime_error ("No Such Registered Room");

                if (_onDel) _onDel (room);  // No throw here
                std::cout << "Deleted\n";
            }
            catch (const std::exception &ex)
            {
                std::cout << ex.what () << std::endl;
            }
        }

        std::list<GuestInfo> _list;
        OnAdd _onAdd;
        OnDel _onDel;
        OnBack _onBack;

    public:
        GuestViewCLI (const std::list<GuestInfo> &list,
                      OnAdd &&onAdd, OnDel &&onDel,
                      OnBack &&onBack)
            : _list (list),
            _onAdd (onAdd), _onDel (onDel), _onBack (onBack)
        {}

        virtual void Show () override
        {
            std::cout << "\nWhat you wanna do?"
                " Enter command to update guest info or back\n"
                " - 'list' to view current list of guests\n"
                " - 'add' to add new guest\n"
                " - 'del' to delete existing guest\n"
                " - Press Enter to go back to welcome page\n";

            while (true)
            {
                std::string cmd;
                std::cout << "$ ";
                if (!InputHelper::GetByRef (cmd))
                {
                    if (_onBack) _onBack ();
                    std::cout << std::endl;
                    return;
                }

                if (cmd == "list") _List ();
                else if (cmd == "add") _Add ();
                else if (cmd == "del") _Del ();
                else std::cout << "Invalid Command\n";
            }
        }
    };

    class LogViewCLI : public LogView
    {
        void _PrintTimeRange () const
        {
            std::cout << "Log starts from "
                << TimeHelper::TimeToString (_timeBeg)
                << " to " << TimeHelper::TimeToString (_timeEnd)
                << std::endl;
        }

        TimePoint _GetTimeBeg () const
        {
            std::cout << "Select a time point to start with"
                " (Time Format: 'Year-Month-Day')\n";

            while (true)
            {
                try
                {
                    auto str = InputHelper::Get<std::string> ("Time String");
                    auto ret = TimeHelper::TimeFromString (str);

                    // Range Validation
                    if (ret < _timeBeg - std::chrono::hours { 24 } ||
                        ret > _timeEnd + std::chrono::hours { 24 })
                    {
                        std::cout << "Time "
                            << TimeHelper::TimeToString (ret)
                            << " out of range\n";
                        _PrintTimeRange ();
                    }
                    else return ret;
                }
                catch (const std::exception &ex)
                {
                    std::cout << ex.what () << std::endl;
                }
            }
        }

        TimePoint _GetTimeEnd (const TimePoint &tBeg) const
        {
            std::cout << "Select a Log Type (day/week/month)\n";
            while (true)
            {
                auto type = InputHelper::Get<std::string> ("Log Mode");
                if (type == "day")
                    return tBeg + std::chrono::hours { 24 };
                else if (type == "week")
                    return tBeg + std::chrono::hours { 24 * 7 };
                else if (type == "month")
                    return tBeg + std::chrono::hours { 24 * 30 };
                else
                    std::cout << "Invalid Log Mode\n";
            }
        }

        void _PrintLog (const LogOnOffList &onOffList,
                        const LogRequestList &requestList)
        {
            if (onOffList.empty ())
                std::cout << "No On-Off records\n";
            else
            {
                try
                {
                    std::ofstream ofs (ONOFFLOGFILE);
                    ofs << LogHelper::LogOnOffListToCsv (onOffList);
                    std::cout << "On-Off records has been saved to '"
                        ONOFFLOGFILE "'\n";
                }
                catch (...)
                {
                    std::cout << "Unable to write to log file\n";
                }
            }

            if (requestList.empty ())
                std::cout << "No Request records\n";
            else
            {
                try
                {
                    std::ofstream ofs (REQUESTLOGFILE);
                    ofs << LogHelper::LogRequestListToCsv (requestList);
                    std::cout << "Reuqest records has been saved to '"
                        REQUESTLOGFILE "'\n";
                }
                catch (...)
                {
                    std::cout << "Unable to write to log file\n";
                }
            }
        }

        TimePoint _timeBeg, _timeEnd;
        OnQueryOnOff _onQueryOnOff;
        OnQueryRequest _onQueryRequest;
        OnBack _onBack;

    public:
        LogViewCLI (TimePoint timeBeg,
                    TimePoint timeEnd,
                    OnQueryOnOff &&onQueryOnOff,
                    OnQueryRequest &&onQueryRequest,
                    OnBack &&onBack)
            : _timeBeg (timeBeg), _timeEnd (timeEnd),
            _onQueryOnOff (onQueryOnOff),
            _onQueryRequest (onQueryRequest),
            _onBack (onBack)
        {}

        virtual void Show () override
        {
            std::cout << "\n";
            _PrintTimeRange ();
            auto timeBeg = _GetTimeBeg ();
            auto timeEnd = _GetTimeEnd (timeBeg);

            auto onOffList = _onQueryOnOff (timeBeg, timeEnd);
            auto requestList = _onQueryRequest (timeBeg, timeEnd);

            _PrintLog (onOffList, requestList);
            std::cout << std::endl;

            if (_onBack) _onBack ();
        }
    };

    class ClientViewCLI : public ClientView
    {
        void _PrintInfo () const
        {
            if (_clients.empty ())
            {
                std::cout << "No slave is connecting...\n";
                return;
            }

            static std::unordered_map<Wind, std::string> windStr
            {
                { 0, "Stop" },
                { 1, "Weak" },
                { 2, "Mid" },
                { 3, "Strong" }
            };

            std::cout << "Client List:\n";
            for (const auto &client : _clients)
            {
                const auto &roomId = client.first;
                const auto &roomState = client.second;
                auto wind = roomState.hasWind ? roomState.wind : 0;

                std::cout << std::fixed
                    << std::setprecision (2)
                    << " - Room: " << roomId
                    << " Guest: " << roomState.guest
                    << " Current: " << roomState.current
                    << " Target: " << roomState.target
                    << " Wind: " << windStr.at (wind)
                    << " Energy: " << roomState.energy
                    << " Cost: " << roomState.cost
                    << "\n";
            }
        }

        ClientList _clients;
        OnUpdate _onUpdate;
        OnBack _onBack;

    public:
        ClientViewCLI (OnUpdate &&onUpdate, OnBack &&onBack)
            : _onUpdate (onUpdate), _onBack (onBack)
        {}

        virtual void Show () override
        {
            std::cout << "\nInput the Refresh Rate of this Page\n";
            int refreshRate;

            while (true)
            {
                refreshRate = InputHelper::Get<int> ("Refresh Rate");
                if (refreshRate > 0) break;
                std::cout << "A positive Refresh Rate is required\n";
            }

            std::cout << "\nPress Enter to go back to welcome page\n";
            auto sleepTime = std::chrono::seconds { refreshRate };
            auto isQuit = false;

            std::thread thread ([&] {
                auto lastHit = std::chrono::system_clock::now ();
                while (!isQuit)
                {
                    try
                    {
                        if (_onUpdate) _clients = _onUpdate ();
                        _PrintInfo ();
                    }
                    catch (const std::exception &ex)
                    {
                        std::cout << ex.what () << std::endl;
                    }

                    // Prevent over sleep :-)
                    auto timeWasted =
                        std::chrono::system_clock::now () - lastHit;
                    if (timeWasted < sleepTime)
                        std::this_thread::sleep_for (sleepTime - timeWasted);
                    lastHit = std::chrono::system_clock::now ();
                }
            });

            InputHelper::GetLine ();
            if (_onBack) _onBack ();
            isQuit = true;
            if (thread.joinable ()) thread.join ();
        }
    };
}

#endif // !AC_SERVER_VIEW_CLI_H