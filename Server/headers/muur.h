#include "socketMeubel.h"

#ifndef MUUR_H
#define MUUR_H

class muur : public socketMeubel
{
public:
    muur(string ID);
    ~muur();
    void readData();
    void sendData(string);
    void connect(int new_socket);
    void potentiometer(char*);

private:
    string device_ID;
    int socket_fd;
    char buff[1024];
    int connected;
    void yeetbuffer();
    int kleur;
    bool automatisch;
    bool uit;
    bool lichtLock;
};

#endif