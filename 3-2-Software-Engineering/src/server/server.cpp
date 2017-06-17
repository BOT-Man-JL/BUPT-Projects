
//
// Air Conditioner - Server
// BOT Man, 2017
//

#include <string>
#include <thread>

#include "server-view.h"
#include "server-protocol.h"

#define PORT 5768

int main (int argc, char *argv[])
{
    using namespace Air_Conditioner;

    // Read from start-up args
    auto serverPort = PORT;
    if (argc > 1) serverPort = std::stoi (argv[2]);

    // Run Server in a background thread
    ProtocolServer server (serverPort);
    std::thread serverThread ([&] { server.Run (); });

    // Start View in main thread
    ServerViewManager viewManager;
    viewManager.ToWelcomeView ();
    viewManager.Start ();

    // Wait the Server thread
    server.Stop ();
    if (serverThread.joinable ()) serverThread.join ();

    return 0;
}