#include "socketMeubel.h"

#ifndef BED_H
#define BED_H

class bed : public socketMeubel
{
public:
    bed(string ID);
    ~bed();
    void readData();
    void sendData(string);
    void connect(int new_socket);
    bool ligtpersoonopbed();

private:
    string device_ID;
    bool ligtOpBed;
    int socket_fd;
    char buff[1024];
    int connected;
    void yeetbuffer();
};

#endif