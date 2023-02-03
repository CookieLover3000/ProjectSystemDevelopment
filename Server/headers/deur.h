#include "socketMeubel.h"
#include "zuil.h"

#ifndef DEUR_H
#define DEUR_H

class deur : public socketMeubel
{
public:
    deur(string ID, zuil*);
    ~deur();
    void readData();
    void sendData(string);
    void connect(int new_socket);
    void deurbewegen();

private:
    string device_ID;
    int socket_fd;
    char buff[1024];
    int connected;
    void yeetbuffer();
    bool open;
    zuil *z;
};

#endif