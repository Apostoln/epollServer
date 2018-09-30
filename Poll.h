#ifndef EPOLLSERVER_POLL_H
#define EPOLLSERVER_POLL_H

#include <algorithm>
#include <vector>
#include <cstring>

#include <poll.h>

#include "Socket.h"

class Poll {
    private:
        using PollEvent = short;

        struct PollSocket : pollfd  {
            Socket mSocket;

            PollSocket(Socket socket, PollEvent event) :
                pollfd{socket.getFd(), event, 0},
                mSocket{socket} {
            }

            Socket getSocket() {
                return mSocket;
            }

        };

        std::vector<pollfd> getRawPollfd() {
            return {mPolledSockets.begin(), mPolledSockets.end()};
        }

    private:
        std::vector<PollSocket> mPolledSockets;

    public:
        void add(Socket socket, PollEvent event) {
            mPolledSockets.emplace_back(socket, event);
        }

        std::vector<Socket> select(int timeout) {
            std::cout << "In poll " << mPolledSockets.size() << std::endl;
            int rc = ::poll(getRawPollfd().data(), getRawPollfd().size(), timeout); //TODO: Bug? pollfd
            if (rc < 0) {
                throw SocketException("Poll select failed");
            }
            else if (rc == 0) {
                return {};
            }
            else {
                std::vector<Socket> result;
                for(auto socket : mPolledSockets) {
                    if (socket.revents == socket.events) {
                        socket.revents = 0;
                        result.push_back(socket.getSocket());
                    }
                }
                return result;
            }
        }

};

#endif //EPOLLSERVER_POLL_H
