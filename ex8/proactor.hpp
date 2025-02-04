#ifndef PROACTOR_H
#define PROACTOR_H

#include <pthread.h>
#include <functional>
#include <memory>

using proactorFunc = std::function<void(int)>;

class Proactor {
public:
    Proactor(int sockfd);
    ~Proactor();

    bool start(proactorFunc handlerFunc);
    void stop();

private:
    int sockfd;
    pthread_t proactorThread;
    bool running;
    proactorFunc handler;

    static void* proactorLoop(void* arg);
};

#endif // PROACTOR_H


