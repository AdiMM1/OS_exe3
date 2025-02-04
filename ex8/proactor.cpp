#include "proactor.hpp"
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct ProactorArgs {
    int sockfd;
    proactorFunc handler;
};

Proactor::Proactor(int sockfd) : sockfd(sockfd), running(false) {}

Proactor::~Proactor() {
    stop();
    close(sockfd);
}

bool Proactor::start(proactorFunc handlerFunc) {
    if (running) return false;
    running = true;
    handler = handlerFunc;

    auto* args = new ProactorArgs{sockfd, handler};

    if (pthread_create(&proactorThread, nullptr, proactorLoop, args) != 0) {
        perror("pthread_create");
        delete args;
        running = false;
        return false;
    }
    return true;
}

void Proactor::stop() {
    if (running) {
        pthread_cancel(proactorThread);
        pthread_join(proactorThread, nullptr);
        running = false;
    }
}

void* Proactor::proactorLoop(void* arg) {
    auto* args = static_cast<ProactorArgs*>(arg);
    int sockfd = args->sockfd;
    proactorFunc handler = args->handler;
    delete args;

    while (true) {
        struct sockaddr_in clientaddr;
        socklen_t addrlen = sizeof(clientaddr);
        int newfd = accept(sockfd, (struct sockaddr*)&clientaddr, &addrlen);
        if (newfd == -1) {
            perror("accept");
            continue;
        }

        pthread_t clientThread;
        if (pthread_create(&clientThread, nullptr, [](void* arg) -> void* {
            auto* args = static_cast<std::pair<int, proactorFunc>*>(arg);
            int fd = args->first;
            proactorFunc handler = args->second;
            handler(fd);
            close(fd);
            delete args;
            return nullptr;
        }, new std::pair<int, proactorFunc>(newfd, handler)) != 0) {
            perror("pthread_create");
            close(newfd);
        } else {
            pthread_detach(clientThread);
        }
    }

    return nullptr;
}