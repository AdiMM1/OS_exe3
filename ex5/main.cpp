#include "reactor.hpp"
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <chrono>

void handleClient(int fd) {
    char buffer[256];
    int n = read(fd, buffer, 255);
    if (n > 0) {
        buffer[n] = '\0';
        std::cout << "Received: " << buffer << std::endl;
        write(fd, "Message received", 16);
    } else {
        close(fd);
    }
}

int main() {
    Reactor* reactor = new Reactor();
    reactor->startReactor();
    
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(9034);
    if (bind(listener, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }
    listen(listener, 10);

    reactor->addFdToReactor(listener, [reactor](int fd) {
        struct sockaddr_in clientaddr;
        socklen_t addrlen = sizeof(clientaddr);
        int newfd = accept(fd, (struct sockaddr*)&clientaddr, &addrlen);
        if (newfd < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            return;
        }
        std::cout << "New connection on socket " << newfd << std::endl;
        reactor->addFdToReactor(newfd, handleClient);
    });

    std::cout << "Server is running on port 9034" << std::endl;
    std::this_thread::sleep_for(std::chrono::hours(1));

    reactor->stopReactor();
    delete reactor;
    return 0;
}







