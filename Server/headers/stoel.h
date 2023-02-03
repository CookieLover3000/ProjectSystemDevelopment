#include "socketMeubel.h"
#include "deur.h"

#ifndef STOEL_H
#define STOEL_H

class stoel : public socketMeubel
{
public:
    stoel(string ID, deur*);
    ~stoel();
    void readData();
    void sendData(string);
    void connect(int new_socket);
    bool zitPersoonOpStoel();

private:
    bool zitOpStoel;
    string device_ID;
    int socket_fd;
    char buff[1024];
    int connected;
    void yeetbuffer();
    deur *Deur;
};

#endif