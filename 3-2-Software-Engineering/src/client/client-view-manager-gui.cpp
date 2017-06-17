
//
// Air Conditioner - Client MVC View (GUI View Manager)
// Youjie Zhang, 2017
//

#include <memory>
#include <functional>

#include "client-controller.h"
#include "client-view-gui.h"

namespace Air_Conditioner
{
    void ClientViewManager::ToAuthView ()
    {
        using namespace std::placeholders;
        auto controller = std::make_shared<ClientFacadeController> (*this);
        _Navigate<AuthViewGUI> (
            std::bind (&ClientFacadeController::Auth, controller, _1));
    }

    void ClientViewManager::ToControlView (
        const GuestInfo &guestInfo,
        const ServerInfo &serverInfo)
    {
        using namespace std::placeholders;
        auto controller = std::make_shared<ClientFacadeController> (*this);
        _Navigate<ControlViewGUI> (
            guestInfo, serverInfo,
            std::bind (&ClientFacadeController::Pulse, controller, _1),
            std::bind (&ClientFacadeController::Simulate, controller, _1, _2));
    }
    void ClientViewManager::PromptErr (const std::string &msg)
    {
        int tmpArgc = 0;
        char **tmpArgv = nullptr;
        QApplication _app(tmpArgc,tmpArgv);
        QMessageBox::critical(NULL,"Fatal Error",QString::fromStdString(msg),QStringLiteral("确定"));
        _app.exec();
    }
}
