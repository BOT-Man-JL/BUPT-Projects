
//
// Air Conditioner - Server MVC View
// BOT Man, 2017
//

#ifndef AC_SERVER_VIEW_H
#define AC_SERVER_VIEW_H

#include "server-model.h"
#include "../common/common-view.h"

namespace Air_Conditioner
{
    enum class ViewType
    {
        ConfigView, GuestView, LogView, ClientView, Quit
    };

    class WelcomeView : public ViewBase
    {
    public:
        using OnNav = std::function<void (ViewType)>;
    };

    class ConfigView : public ViewBase
    {
    public:
        using OnSet = std::function<void (const ServerInfo &)>;
        using OnBack = std::function<void ()>;
    };

    class GuestView : public ViewBase
    {
    public:
        using OnAdd = std::function<void (const GuestInfo &)>;
        using OnDel = std::function<void (const RoomId &)>;
        using OnBack = std::function<void ()>;
    };

    class LogView : public ViewBase
    {
    public:
        using OnQueryOnOff = std::function<LogOnOffList (const TimePoint &, const TimePoint &)>;
        using OnQueryRequest = std::function<LogRequestList (const TimePoint &, const TimePoint &)>;
        using OnBack = std::function<void ()>;
    };

    class ClientView : public ViewBase
    {
    public:
        using OnUpdate = std::function<ClientList ()>;
        using OnBack = std::function<void ()>;
    };

    class ServerViewManager : public ViewManager
    {
    public:
        // Implemented in 'server-view-*.cpp'
        void ToWelcomeView ();
        void ToConfigView ();
        void ToGuestView ();
        void ToLogView ();
        void ToClientView ();
    };
}

#endif // !AC_SERVER_VIEW_H