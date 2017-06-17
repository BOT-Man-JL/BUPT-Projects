
//
// Air Conditioner - Client
// BOT Man, 2017
//

#include <string>

#include "client-protocol.h"
#include "client-view.h"

#define IPADDR "127.0.0.1"
#define PORT 5768

int main (int argc, char *argv[])
{
    using namespace Air_Conditioner;

    // Read from start-up args to Init Protocol
    auto serverIp = IPADDR;
    auto serverPort = PORT;

    // Init viewManager
    ClientViewManager viewManager;

    try
    {
        if (argc > 1) serverIp = argv[1];
        if (argc > 2) serverPort = std::stoi (argv[2]);
    }
    catch (...)
    {
        viewManager.PromptErr ("Invalid command line args");
        return 1;
    }

    // Dependency Injection
    try { ProtocolClient::Init (serverIp, (unsigned short) serverPort); }
    catch (const std::exception &ex)
    {
        viewManager.PromptErr (ex.what ());
        return 2;
    }

    // Start the view
    viewManager.ToAuthView ();
    viewManager.Start ();

    return 0;
}