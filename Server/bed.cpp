#include "headers/bed.h"

bed::bed(string ID) : socketMeubel(ID), device_ID(ID), connected(0)
{
}

bed::~bed()
{
}

// Leest de data die door de client gestuurd wordt en verwerkt deze.
void bed::readData()
{
    string opBed = "Persoon ligt op bed";
    string nietOpBed = "Persoon ligt niet op bed";
    string knop = "<knopBed>";
    yeetbuffer();
    if (connected && read(socket_fd, buff, 1024) > 0)
    {
        cout << buff;

        if (opBed.compare(buff) == 0)
            ligtOpBed = true;
        if (nietOpBed.compare(buff) == 0)
            ligtOpBed = false;
        if (knop.compare(buff) == 0)
            sendData("<veranderKnop>");

        yeetbuffer();
    }
}

// Voor het versturen van data zijn '<' en '>' heel belangrijk.
// Deze tekens geven het start en het einde van het bericht aan voor de client.
void bed::sendData(string temp)
{
    send(socket_fd, temp.c_str(), strlen(temp.c_str()), 0);
}

// Maakt een connectie met de client. De field descriptor in de functie is dan gelinkt aan de client.
// Hierdoor kunnen meerdere clients tegelijkertijd worden verbonden en weet de server nog steeds welke client waarbij hoort.
void bed::connect(int new_socket)
{
    socket_fd = new_socket;
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    cout << device_ID << " connected" << endl;

    send(socket_fd, "<id_accept>", strlen("<id_accept>"), 0);
    connected = 1;
}
// yeet de buffer.
void bed::yeetbuffer()
{
    memset(buff, 0, 1024);
}

// returnt of persoon op bed ligt.
bool bed::ligtpersoonopbed()
{
    return ligtOpBed;
}
