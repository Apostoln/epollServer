#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cassert>
#include <vector>
#include <exception>


const size_t PORT = 12345;
const size_t BUFFER_SIZE = 1024;


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

    public:
        Socket(FileDescriptor fd): mSocketFd(fd) {}

        Socket(TransportProtocol protocol = TransportProtocol::TCP) {
            assert(protocol == TransportProtocol::TCP);

            auto socketType = (protocol == TransportProtocol::TCP
                              ? SOCK_STREAM
                              : SOCK_DGRAM);
            mSocketFd = ::socket(AF_INET, SOCK_STREAM, 0);
            if (mSocketFd < 0) {
                throw SocketException("Can't create socket");
            }

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

            size_t receivedBytes = ::recv(mSocketFd, buffer, BUFFER_SIZE, MSG_NOSIGNAL);
            if ( receivedBytes < 0) {
                throw SocketException("Can't receive data from socket");
            }
            return {std::begin(buffer), std::begin(buffer) + receivedBytes};
        }

        void send(const std::string& msg) {
            size_t sentBytes = ::send(mSocketFd, msg.data(), msg.size(), MSG_NOSIGNAL);
            if (sentBytes < 0) {
                throw SocketException("Can't send socket");
            }
        }

        operator int() {
            return mSocketFd;
        }

        ~Socket() {
            ::close(mSocketFd); // TODO: correct shutdown
        }
};

class Server {
    private:
        Socket mMasterSocket;
        std::vector<Socket> mSlaveSockets;
        size_t mPort;

    public:
        Server(size_t port): mPort(port) {};

        void run() {
            mMasterSocket.bind(mPort);
            mMasterSocket.listen();

            while(Socket clientSocket = mMasterSocket.accept()) {
                mSlaveSockets.push_back(clientSocket);
                auto message = clientSocket.receive();
                std::cout << message << std::endl;
            }
        }
};

int main() {
    Server server(PORT);
    server.run();

    return 0;
}