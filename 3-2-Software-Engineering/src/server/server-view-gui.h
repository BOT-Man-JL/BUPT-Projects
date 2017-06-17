
//
// Air Conditioner - Server MVC View (GUI View)
// Youjie Zhang, 2017
//

#ifndef AC_SERVER_VIEW_GUI_H
#define AC_SERVER_VIEW_GUI_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <thread>
#include <chrono>

#include <QApplication>


#include "server-view-gui-qt.h"
#include "server-view.h"
#include "../common/cli-helper.h"
#include "log-helper.h"

#define ONOFFLOGFILE "on-off-log.csv"
#define REQUESTLOGFILE "request-log.csv"

namespace Air_Conditioner
{
    class WelcomeViewGUI : public WelcomeView
    {     
        OnNav _onNav;
    public:
        WelcomeViewGUI (OnNav &&onNav)
            : _onNav (onNav)
        {}

        virtual void Show () override
        {

            int tmpArgc = 0;
            char ** tmpArgv = nullptr;
            QApplication app(tmpArgc,tmpArgv);
            WelcomeWindow welcome;
            welcome.setOnQuit([&]{
                welcome.close();
                _onNav(ViewType::Quit);

            });

            welcome.setOnConfig([&]{
                welcome.close();
                _onNav(ViewType::ConfigView);
            });

            welcome.setOnLog([&]{
                welcome.close();
                _onNav(ViewType::LogView);
            });

            welcome.setOnClient([&]{
                welcome.close();
                _onNav(ViewType::ClientView);
            });

            welcome.setOnGuest([&]{
                welcome.close();
                _onNav(ViewType::GuestView);
            });

            welcome.show();
            app.exec();

        }
    };

    class ConfigViewGUI : public ConfigView
    {
        ServerInfo _config;
        OnSet _onSet;
        OnBack _onBack;
    public:
        ConfigViewGUI (const ServerInfo &config,
                       OnSet &&onSet, OnBack &&onBack)
            : _config (config), _onSet (onSet), _onBack (onBack)
        {}

        virtual void Show () override
        {                    
            int tmpArgc = 0;
            char ** tmpArgv = nullptr;
            QApplication app(tmpArgc,tmpArgv);
            ConfigWindow configWin;
            configWin.LoadConfig(_config);
            configWin.SetOnBack(std::move(_onBack));
            configWin.SetOnSet(std::move(_onSet));
            configWin.show();
            app.exec();
        }
    };

    class GuestViewGUI : public GuestView
    {   
        std::list<GuestInfo> _list;
        OnAdd _onAdd;
        OnDel _onDel;
        OnBack _onBack;

    public:
        GuestViewGUI (const std::list<GuestInfo> &list,
                      OnAdd &&onAdd, OnDel &&onDel,
                      OnBack &&onBack)
            : _list (list),
            _onAdd (onAdd), _onDel (onDel), _onBack (onBack)
        {}

        virtual void Show () override
        {        
            int tmpArgc = 0;
            char ** tmpArgv = nullptr;
            QApplication app(tmpArgc,tmpArgv);
            GuestWindow guest;
            guest.SetOnBack(std::move(_onBack));
            guest.LoadGuest(_list);
            guest.SetOnAdd(std::move(_onAdd));
            guest.SetOnDel(std::move(_onDel));
            guest.SetOnBack(std::move(_onBack));
            guest.show();
            app.exec();
        }
    };

    class LogViewGUI : public LogView
    {
        TimePoint _GetTimeEnd (int mode, const TimePoint &tBeg) const
        {
            std::cout << "Select a Log Type (day/week/month)\n";
            while (true)
            {

                if (mode == 0)
                    return tBeg + std::chrono::hours { 24 };
                else if (mode == 1)
                    return tBeg + std::chrono::hours { 24 * 7 };
                else if (mode == 2)
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
        LogViewGUI (TimePoint timeBeg,
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
            int tmpArgc = 0;
            char ** tmpArgv = nullptr;
            QApplication app(tmpArgc,tmpArgv);
            StatisticWindow log;
            log.SetTimeRange(TimeHelper::TimeToString(_timeBeg),
                             TimeHelper::TimeToString(_timeEnd));
            log.SetOnBack(std::move(_onBack));
            log.SetOnTimeBegin([&](const int mode,const std::string &time) -> std::pair<TimePoint,TimePoint>
                    {
                        auto begin = TimeHelper::TimeFromString (time);
                        // Range Validation
                        if (begin < _timeBeg - std::chrono::hours { 24 } ||
                            begin > _timeEnd + std::chrono::hours { 24 })
                        {
                            std::cout << "Time "
                                << TimeHelper::TimeToString (begin)
                                << " out of range\n";
                            QString prompt = QStringLiteral("报表由 ") + QString::fromStdString(TimeHelper::TimeToString (_timeBeg))+
                                    QStringLiteral(" 至 ")+QString::fromStdString(TimeHelper::TimeToString (_timeEnd));
                            throw std::runtime_error(prompt.toStdString());
                        }
                        else {
                            auto end = _GetTimeEnd(mode,begin);
                            return std::make_pair(begin,end);
                        }
                   }
            );
            log.SetOnExport([&](TimePoint timeBeg,TimePoint timeEnd){
                // Range Validation
                if (timeBeg < _timeBeg - std::chrono::hours { 24 } ||
                    timeBeg > _timeEnd + std::chrono::hours { 24 })
                {
                    std::cout << "Time "
                        << TimeHelper::TimeToString (timeBeg)
                        << " out of range\n";
                    QString prompt = QStringLiteral("报表由 ") + QString::fromStdString(TimeHelper::TimeToString (_timeBeg))+
                            QStringLiteral(" 至 ")+QString::fromStdString(TimeHelper::TimeToString (_timeEnd));
                    throw std::runtime_error(prompt.toStdString());
                }
                auto onOffList = _onQueryOnOff (timeBeg, timeEnd);
                auto requestList = _onQueryRequest (timeBeg, timeEnd);
                _PrintLog(onOffList, requestList);
            });
            log.show();
            app.exec();
        }
    };

    class ClientViewGUI : public ClientView
    {
        ClientList _clients;
        OnUpdate _onUpdate;
        OnBack _onBack;

    public:
        ClientViewGUI (OnUpdate &&onUpdate, OnBack &&onBack)
            : _onUpdate (onUpdate), _onBack (onBack)
        {}

        virtual void Show () override
           {
               int tmpArgc = 0;
               char ** tmpArgv = nullptr;
               QApplication app(tmpArgc,tmpArgv);
               ClientWindow client;
               client.SetOnBack(std::move(_onBack));
               client.SetOnUpdate(std::move(_onUpdate));
               client.show();
               app.exec();
           }


    };
}

#endif // !AC_SERVER_VIEW_GUI_H