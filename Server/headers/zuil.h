#include "socketMeubel.h"
#include "balie.h"
#include "muur.h"

#ifndef ZUIL_H
#define ZUIL_H

class zuil : public socketMeubel
{
public:
    zuil(string ID, balie*, muur*);
    ~zuil();
    void readData();
    void sendData(string);
    void connect(int new_socket);
    bool returnRookAlarm();

private:
    string device_ID;
    int socket_fd;
    char buff[1024];
    int connected;
    bool rookalarm;
    void yeetbuffer();
    balie *Balie;
    muur * M;
};

#endif