#ifndef EPOLLSERVER_SERVER_H
#define EPOLLSERVER_SERVER_H

#include <vector>
#include <set>
#include "Socket.h"

class Server {
private:
    Socket mMasterSocket;
    size_t mPort;

    //Poll mPoll;

    std::vector<Socket> mSlaveSockets;

public:
    Server(size_t port): mPort(port) {
    }

    void run() {
        mMasterSocket.bind(mPort);
        mMasterSocket.listen();

        std::cout << "Server run on localhost:" << mPort << std::endl;

/*
 * Draft of poll:
        mPoll.add(Socket, Poll::POLLIN);
        while(true) {
            int timeout = 10;
            std::set<Socket> result = mPoll.select(timeout);
            for(auto socket : result) {
                if (socket == mMasterSocket) { //todo: add event to condition
                    Socket newClientSocket = mMasterSocket.accept();
                    mPoll.add(newClientSocket, Poll::POLLIN);
                    mSlaveSockets.push_back(Socket);
                }
                else { //todo: add event to condition
                    auto message = socket.receive();
                    std::cout << message << std::endl;
                    socket.send(message);
                }

            }

        }
*/



        while(Socket clientSocket = mMasterSocket.accept()) {
            auto message = clientSocket.receive();
            std::cout << message << std::endl;
            clientSocket.send(message);
            //mSlaveSockets.push_back(std::move(clientSocket));
        }


    }
};
#endif //EPOLLSERVER_SERVER_H
