// interface class (als het goed is)
#ifndef SOCKETMEUBEL_H
#define SOCKETMEUBEL_H

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <fcntl.h>
#include <wiringPi.h>

using namespace std;
#pragma once

class socketMeubel
{
public:
    socketMeubel(string ID){}
    ~socketMeubel(){}
    virtual void readData() = 0;
    virtual void sendData(string) = 0;
    virtual void connect(int new_socket) = 0;
};

#endif