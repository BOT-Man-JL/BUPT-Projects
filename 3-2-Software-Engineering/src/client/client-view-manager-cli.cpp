
//
// Air Conditioner - Client MVC View (CLI View Manager)
// BOT Man, 2017
//

#include <memory>
#include <functional>

#include "client-controller.h"
#include "client-view-cli.h"
#include "../common/cli-helper.h"

namespace Air_Conditioner
{
    void ClientViewManager::ToAuthView ()
    {
        using namespace std::placeholders;
        auto controller = std::make_shared<ClientFacadeController> (*this);
        _Navigate<AuthViewCLI> (
            std::bind (&ClientFacadeController::Auth, controller, _1));
    }

    void ClientViewManager::ToControlView (
        const GuestInfo &guestInfo,
        const ServerInfo &serverInfo)
    {
        using namespace std::placeholders;
        auto controller = std::make_shared<ClientFacadeController> (*this);
        _Navigate<ControlViewCLI> (
            guestInfo, serverInfo,
            std::bind (&ClientFacadeController::Pulse, controller, _1),
            std::bind (&ClientFacadeController::Simulate, controller, _1, _2));
    }

    void ClientViewManager::PromptErr (const std::string &msg)
    {
        std::cout << msg << " (Press Enter to Quit)\n";
        InputHelper::GetLine ();
    }
}