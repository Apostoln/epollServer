#include "Server.h"

const size_t PORT = 12345;

int main() {
    try {
        Server server(PORT);
        server.run();
    }
    catch (SocketException& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}