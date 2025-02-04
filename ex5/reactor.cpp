#include "reactor.hpp"
#include <thread>
#include <iostream>
#include <algorithm>

Reactor::Reactor() : running(false), maxFd(0) {
    FD_ZERO(&masterSet);
}

Reactor::~Reactor() {
    stopReactor();
}

void* Reactor::startReactor() {
    running = true;
    std::thread reactorThread(&Reactor::run, this);
    reactorThread.detach();
    return this;
}

int Reactor::addFdToReactor(int fd, reactorFunc func) {
    if (fdMap.find(fd) != fdMap.end()) {
        std::cerr << "FD " << fd << " already exists in reactor!" << std::endl;
        return -1; 
    }

    fdMap[fd] = func;
    FD_SET(fd, &masterSet);
    if (fd > maxFd) {
        maxFd = fd;
    }

    return 0;
}

int Reactor::removeFdFromReactor(int fd) {
    if (fdMap.find(fd) == fdMap.end()) {
        std::cerr << "FD " << fd << " not found in reactor!" << std::endl;
        return -1; 
    }

    fdMap.erase(fd);
    FD_CLR(fd, &masterSet);
    if (fd == maxFd) {
        if (fdMap.empty()) {
            maxFd = 0;
        } else {
            maxFd = std::max_element(fdMap.begin(), fdMap.end(),
                                     [](const std::pair<int, reactorFunc>& lhs, const std::pair<int, reactorFunc>& rhs) {
                                         return lhs.first < rhs.first;
                                     })->first;
        }
    }

    return 0;
}

int Reactor::stopReactor() {
    running = false;
    return 0;
}

void Reactor::run() {
    while (running) {
        fd_set readSet = masterSet;
        int activity = select(maxFd + 1, &readSet, nullptr, nullptr, nullptr);

        if (activity < 0) {
            std::cerr << "select error" << std::endl;
            break;
        }

        for (int fd = 0; fd <= maxFd; ++fd) {
            if (FD_ISSET(fd, &readSet)) {
                auto it = fdMap.find(fd);
                if (it != fdMap.end()) {
                    it->second(fd);
                }
            }
        }
    }
}







