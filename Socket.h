#ifndef EPOLLSERVER_SOCKET_H
#define EPOLLSERVER_SOCKET_H

#include <iostream>
#include <cstring>


#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <memory>

using FileDescriptor = int;

class SocketException : public std::runtime_error { //todo: replace to std::system_error
public:
    SocketException(const char* msg): std::runtime_error(msg) {}

    virtual const char* what() const noexcept override {
        std::string tmp = this->std::runtime_error::what();
        tmp += "; errno = ";
        tmp += std::to_string(errno);
        tmp += "; ";
        tmp += strerror(errno);
        char* result = new char[tmp.length() + 1];
        return strcpy(result, tmp.c_str());
    }
};


class SocketImpl {
    private:
        FileDescriptor mSocketFd = 0;
        static const size_t BUFFER_SIZE = 1024;

    public:
        SocketImpl() {
            mSocketFd = ::socket(AF_INET, SOCK_STREAM, 0);
            if (mSocketFd < 0) {
                throw SocketException("Can't create socket");
            }

            int opt_val = 1;
            setsockopt(mSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

            std::cout << "SocketImpl() " << this << " " << mSocketFd << std::endl;
        }

        SocketImpl(FileDescriptor fd): mSocketFd(fd) {
            std::cout << "SocketImpl(fd) " << this << " " << mSocketFd << std::endl;
        }

        SocketImpl(SocketImpl&& other): mSocketFd(other.getFd()) {
            std::cout << "SocketImpl(SocketImpl&&) " << this << " from " << &other << " , " << mSocketFd << std::endl;
            other.mSocketFd = -1;
        }

        void bind(int port) {
            sockaddr_in socketAddress;
            socketAddress.sin_family = AF_INET;
            socketAddress.sin_port = htons(port);
            socketAddress.sin_addr.s_addr = htons(INADDR_ANY);

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

        FileDescriptor accept() {
            std::cout << "accept()" << std::endl;
            return ::accept(mSocketFd, nullptr, nullptr);
        }

        std::string receive() {
            char buffer[BUFFER_SIZE];
            int receivedBytes = ::recv(mSocketFd, buffer, BUFFER_SIZE, MSG_NOSIGNAL);
            if (receivedBytes < 0) {
                throw SocketException("Can't receive data from socket");
            }
            if (receivedBytes == 0) {
                //Other peer has gracefully disconnected
                return {};
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

        FileDescriptor getFd() const {
            return mSocketFd;
        }

        operator int() {
            return mSocketFd;
        }

        ~SocketImpl() {
            std::cout << "SocketImpl destr " << this << " " << mSocketFd << std::endl;

            int rc = ::shutdown(mSocketFd, SHUT_RDWR);
            if (rc < 0) {
                auto e = SocketException("Can't shutdown socket");
                std::cout << e.what() << std::endl;
            }

            rc = ::close(mSocketFd);
            if (rc < 0) {
                auto e = SocketException("Can't close socket");
                std::cout << e.what() << std::endl;
            }
        }
};


class Socket {
    private:
        std::shared_ptr <SocketImpl> mSocketPtr;

    public:
        Socket(): mSocketPtr{new SocketImpl{}} {

        }

        Socket(FileDescriptor fd): mSocketPtr{new SocketImpl{fd}} {

        }

        Socket(const Socket& other): mSocketPtr{other.mSocketPtr} {

        }

        void bind(int port) {
            mSocketPtr->bind(port);
        }

        void listen() {
            mSocketPtr->listen();
        }

        Socket accept() {
            return {mSocketPtr->accept()};
        }

        std::string receive() {
            return mSocketPtr->receive();
        }

        void send(const std::string& msg) {
            mSocketPtr->send(msg);
        }

        FileDescriptor getFd() const {
            return mSocketPtr->getFd();
        }

        operator int() {
            return static_cast<int>(*mSocketPtr);
        }

        bool operator == (const Socket& other) {
            return this->getFd() == other.getFd();
        }

};

#endif //EPOLLSERVER_SOCKET_H
