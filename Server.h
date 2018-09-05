#ifndef EPOLLSERVER_SERVER_H
#define EPOLLSERVER_SERVER_H

#include <vector>
#include "Socket.h"

class Server {
private:
    Socket mMasterSocket;
    size_t mPort;

    std::vector<Socket> mSlaveSockets;

public:
    Server(size_t port): mPort(port) {
    }

    void run() {
        mMasterSocket.bind(mPort);
        mMasterSocket.listen();

        std::cout << "Server run on localhost:" << mPort << std::endl;

        while(Socket clientSocket = mMasterSocket.accept()) {
            auto message = clientSocket.receive();
            std::cout << message << std::endl;
            clientSocket.send(message);
            //mSlaveSockets.push_back(std::move(clientSocket));
        }
    }
};
#endif //EPOLLSERVER_SERVER_H
