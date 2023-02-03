#include "socketMeubel.h"
#include "zuil.h"
#include "balie.h"

#ifndef KOELKAST_H
#define KOELKAST_H

class koelkast : public socketMeubel
{
public:
    koelkast(string ID, zuil*, balie *);
    ~koelkast();
    void readData();
    void sendData(string);
    void connect(int new_socket);

private:
    string device_ID;
    int socket_fd;
    char buff[1024];
    int connected;
    void yeetbuffer();
    zuil *Zuil;
    balie *b;
};

#endif