#include "headers/zuil.h"

zuil::zuil(string ID, balie *B, muur *m) : socketMeubel(ID), device_ID(ID), connected(0), Balie(B), M(m), rookalarm(false)
{
    digitalWrite(18, 0);
}

zuil::~zuil()
{
}

// Leest de data die door de client gestuurd wordt en verwerkt deze.
void zuil::readData()
{
    string alarm = "<ALARM>";
    string stopAlarm = "<STOP>";
    yeetbuffer();
    if (connected && read(socket_fd, buff, 1024) > 0)
    {
        cout << buff ;
        if (alarm.compare(buff) == 0)
        {
            Balie->turnLedOn(18);
            M->sendData("<ga dicht>");
            rookalarm = true;
        }
        else if(stopAlarm.compare(buff) == 0)
        {
            Balie->turnLedOff(18);
            M->sendData("<ga open>");
            rookalarm = false;
        }

        yeetbuffer();
    }
}

// Voor het versturen van data zijn '<' en '>' heel belangrijk.
// Deze tekens geven het start en het einde van het bericht aan voor de client.
void zuil::sendData(string temp)
{
    send(socket_fd, temp.c_str(), strlen(temp.c_str()), 0);
}

// Maakt een connectie met de client. De field descriptor in de functie is dan gelinkt aan de client.
// Hierdoor kunnen meerdere clients tegelijkertijd worden verbonden en weet de server nog steeds welke client waarbij hoort.
void zuil::connect(int new_socket)
{
    socket_fd = new_socket;
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    std::cout << device_ID << " connected" << endl;

    send(socket_fd, "<id_accept>", strlen("<id_accept>"), 0);
    connected = 1;
}

// yeet de buffer.
void zuil::yeetbuffer()
{
    memset(buff, 0, 1024);
}

// Returnt of het rookalarm afgaat.
bool zuil::returnRookAlarm()
{
    return rookalarm;
}