#ifndef EPOLLSERVER_SERVER_H
#define EPOLLSERVER_SERVER_H

#include <vector>
#include <set>
#include "Socket.h"
#include "Poll.h"

class Server {
private:
    Socket mMasterSocket;
    size_t mPort;

    Poll mPoll;

    std::vector<Socket> mSlaveSockets;

public:
    Server(size_t port): mPort(port) {
    }

    void run() {
        mMasterSocket.bind(mPort);
        mMasterSocket.listen();

        std::cout << "Server run on localhost:" << mPort << std::endl;

        mPoll.add(mMasterSocket, POLLIN);
        while(true) {
            const static int TIMEOUT = 10 * 1000;  //s
            std::vector<Socket> result = mPoll.select(TIMEOUT);
            std::cout << "Result size: " << result.size() << std::endl;
            for(auto socket : result) {
                if (socket == mMasterSocket) { //todo: add event to condition
                    Socket newClientSocket = mMasterSocket.accept();
                    mPoll.add(newClientSocket, POLLIN);
                    mSlaveSockets.push_back(newClientSocket);
                }
                else { //todo: add event to condition
                    auto message = socket.receive();
                    std::cout << message << std::endl;
                    socket.send(message);
                }
            }
        }


    }
};
#endif //EPOLLSERVER_SERVER_H
