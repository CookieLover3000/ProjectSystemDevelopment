#include "headers/balie.h"

balie::balie() : status(0)
{
    wiringPiSetupGpio();
    pinMode(18, OUTPUT);
    digitalWrite(18, 0);
    pinMode(23, OUTPUT);
    digitalWrite(23, 0);
    pinMode(24, OUTPUT);
    digitalWrite(24, 0);
}
balie::~balie()
{
}

void balie::turnLedOff(int pin)
{
    status = 0;
    digitalWrite(pin, status);
}

void balie::turnLedOn(int pin)
{
    status = 1;
    digitalWrite(pin, status);
}