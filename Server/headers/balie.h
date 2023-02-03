#include <iostream>
#include <wiringPi.h>

#ifndef BALIE_H
#define BALIE_H

class balie
{
public:
    balie();
    ~balie();
    void turnLedOn(int);
    void turnLedOff(int);

private:
int status;
};

#endif