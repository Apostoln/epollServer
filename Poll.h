#ifndef EPOLLSERVER_POLL_H
#define EPOLLSERVER_POLL_H

#include <algorithm>
#include <vector>
#include <cstring>

#include <poll.h>
#include <cassert>

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

        void remove(Socket socket) {
            mPolledSockets.erase(std::remove_if(mPolledSockets.begin(),
                                                mPolledSockets.end(),
                                                [&socket](PollSocket ps) {
                                                    return ps.fd == socket.getFd();
                                                }),
                                 mPolledSockets.end());
        }

        std::vector<Socket> select(int timeout) {
            //std::cout << "In poll " << mPolledSockets.size() << std::endl;
            auto rawPollfd = getRawPollfd();
            int rc = ::poll(rawPollfd.data(), rawPollfd.size(), timeout);
            if (rc < 0) {
                throw SocketException("Poll select failed");
            }
            else if (rc == 0) {
                return {};
            }
            else {
                std::vector<Socket> result;
                for(auto socket : rawPollfd) {
                    if (socket.revents == socket.events) {
                        socket.revents = 0;
                        //todo: replace this crutch
                        auto it = find_if(mPolledSockets.begin(), mPolledSockets.end(), [&socket](PollSocket& other) {
                            return other.fd == socket.fd;
                        });
                        if (it != mPolledSockets.end()) {
                            result.push_back(it->getSocket());
                        }
                        else {
                            assert(false);
                        }
                    }
                }
                assert(!result.empty());
                return result;
            }
        }

};

#endif //EPOLLSERVER_POLL_H
