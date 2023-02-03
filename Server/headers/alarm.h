#include <iostream>
#include <wiringPi.h>

#include "bed.h"
#include "stoel.h"
#include "schemerlamp.h"
#include "balie.h"
#include "Stopwatch.h"

#ifndef ALARM_H
#define ALARM_H

class Alarm
{
public:
    Alarm(bed *, schemerlamp *, stoel *, balie *);
    ~Alarm();
    void melding();

private:
bool ligtBed;
bool zitStoel;
bool aanHetBewegen;
bed *B;
stoel *S;
schemerlamp *L;
balie *I;
Stopwatch tijdmeting;
bool ingeweest;
};

#endif