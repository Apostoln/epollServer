#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cassert>
#include <vector>
#include <exception>

const size_t PORT = 12345;

enum class TransportProtocol {
    TCP,
    UDP
};

class SocketException : public std::runtime_error { //todo: replace to std::system_error
    public:
        SocketException(const char* msg): std::runtime_error(msg) {}
};

class Socket {
    public:
        using FileDescriptor = int;

    private:
        FileDescriptor mSocketFd = 0;
        static const size_t BUFFER_SIZE = 1024;

    public:
        Socket(FileDescriptor fd): mSocketFd(fd) {
        }

        Socket(TransportProtocol protocol = TransportProtocol::TCP) {
            assert(protocol == TransportProtocol::TCP);

            auto socketType = (protocol == TransportProtocol::TCP
                              ? SOCK_STREAM
                              : SOCK_DGRAM);
            mSocketFd = ::socket(AF_INET, SOCK_STREAM, 0);
            if (mSocketFd < 0) {
                throw SocketException("Can't create socket");
            }

            int opt_val = 1;
            setsockopt(mSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

        }

        void bind(int port) {
            sockaddr_in socketAddress;
            socketAddress.sin_family = AF_INET;
            socketAddress.sin_port = ::htons(PORT);
            socketAddress.sin_addr.s_addr = ::htons(INADDR_ANY);

            int rc = ::bind(mSocketFd,
                            reinterpret_cast<sockaddr*>(&socketAddress),
                            sizeof(socketAddress));
            if (rc < 0) {
                throw SocketException("Can't bind socket");
            }
        }

        void listen() {
            int rc = ::listen(mSocketFd, SOMAXCONN);
            if (rc < 0) {
                throw SocketException("Can't listen socket");
            }
        }

        Socket accept() {
            return {::accept(mSocketFd, nullptr, nullptr)};
        }

        std::string receive() {
            char buffer[BUFFER_SIZE];
            int receivedBytes = ::recv(mSocketFd, buffer, BUFFER_SIZE, MSG_NOSIGNAL);
            if ( receivedBytes < 0) {
                std::cerr << receivedBytes << " " << strerror(errno) << std::endl;
                throw SocketException("Can't receive data from socket");
            }
            return {std::begin(buffer), std::begin(buffer) + receivedBytes};
        }

        void send(const std::string& msg) {
            int sentBytes = ::send(mSocketFd, msg.data(), msg.size(), MSG_NOSIGNAL);
            if (sentBytes < 0) {
                std::cerr << sentBytes << " " << strerror(errno) << std::endl;
                throw SocketException("Can't send socket");
            }
        }

        operator int() {
            return mSocketFd;
        }

        ~Socket() {
            // There is possible bug here - socket is closed after destroying even if fd is copied to another object
            ::close(mSocketFd); // TODO: correct shutdown

        }
};

class Server {
    private:
        Socket mMasterSocket;
        size_t mPort;

    public:
        Server(size_t port): mPort(port) {};

        void run() {
            mMasterSocket.bind(mPort);
            mMasterSocket.listen();

            std::cout << "Server run on localhost:" << mPort << std::endl;

            while (Socket clientSocket = mMasterSocket.accept()) {
                auto message = clientSocket.receive();
                std::cout << message << std::endl;
                clientSocket.send(message);
            }
        }
};

int main() {
    Server server(PORT);
    server.run();

    return 0;
}