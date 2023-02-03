#include "headers/alarm.h"

Alarm::Alarm(bed *be, schemerlamp *la, stoel *st, balie *ba) : B(be), L(la), S(st), I(ba), ingeweest(false)
{
}
Alarm::~Alarm()
{
}

void Alarm::melding()
{
    ligtBed = B->ligtpersoonopbed();
    zitStoel = S->zitPersoonOpStoel();
    aanHetBewegen = L->isAanHetBewegen();
    if ((!ligtBed) && (!zitStoel) && (!aanHetBewegen) && (!ingeweest))
    {
        ingeweest = true;
        tijdmeting.reset();
        tijdmeting.begin();
    }
    if ((ligtBed) || (zitStoel) || (aanHetBewegen))
    {
        ingeweest = false;
        tijdmeting.stop();
        tijdmeting.reset();
        I->turnLedOff(23);
    }
    tijdmeting.stop();
    if (tijdmeting.deTijd() > 10 && ingeweest)
        I->turnLedOn(23);
}
