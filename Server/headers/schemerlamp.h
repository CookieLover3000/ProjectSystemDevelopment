#include "socketMeubel.h"

#ifndef SCHEMERLAMP_H
#define SCHEMERLAMP_H

class schemerlamp : public socketMeubel
{
public:
    schemerlamp(string ID);
    ~schemerlamp();
    void readData();
    void sendData(string);
    void connect(int new_socket);
    bool isAanHetBewegen();
    void lampStand();

private:
    string device_ID;
    int socket_fd;
    char buff[1024];
    int connected;
    void yeetbuffer();
    bool beweging;
    bool S1;
    bool S2;
    bool S3;
};

#endif