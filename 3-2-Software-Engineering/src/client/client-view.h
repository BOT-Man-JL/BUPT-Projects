
//
// Air Conditioner - Client MVC View
// BOT Man, 2017
//

#ifndef AC_CLIENT_VIEW_H
#define AC_CLIENT_VIEW_H

#include <string>

#include "client-model.h"
#include "../common/common-view.h"

namespace Air_Conditioner
{
    class AuthView : public ViewBase
    {
    public:
        using OnAuth = std::function<void (const GuestInfo &)>;
    };

    class ControlView : public ViewBase
    {
    public:
        using OnPulse = std::function<
            std::pair<ClientInfo, ServerInfo> (const RoomRequest &)>;
        using OnSim = std::function<
            void (RoomRequest &, const bool)>;
    };

    class ClientViewManager : public ViewManager
    {
    public:
        // Implemented in 'client-view-manager-*.cpp'
        void ToAuthView ();
        void ToControlView (const GuestInfo &guestInfo,
                            const ServerInfo &serverInfo);

        void PromptErr (const std::string &msg);
    };
}

#endif // !AC_CLIENT_VIEW_H