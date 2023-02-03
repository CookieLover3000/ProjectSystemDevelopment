#include "headers/koelkast.h"

koelkast::koelkast(string ID, zuil *Z, balie *B) : socketMeubel(ID), device_ID(ID), connected(0), Zuil(Z), b(B)
{
}

koelkast::~koelkast()
{
}

// Leest de data die door de client gestuurd wordt en verwerkt deze.
void koelkast::readData()
{
    string open = "Koelkast open";
    string langOpen = "10s open";
    string zeerLangOpen = "15s open";
    string dicht = "Koelkast dicht";
    yeetbuffer();
    if (connected && read(socket_fd, buff, 1024) > 0)
    {
        cout << buff;
        if (open.compare(buff) == 0)
            sendData("<Fan uit>");
        else if (langOpen.compare(buff) == 0)
            Zuil->sendData("<koelkast>");
        else if (zeerLangOpen.compare(buff) == 0)
            b->turnLedOn(24);
        else if (dicht.compare(buff) == 0)
        {
            sendData("<Fan aan>");
            Zuil->sendData("<koelkast dicht>");
            b->turnLedOff(24);
        }
        yeetbuffer();
    }
}
// Voor het versturen van data zijn '<' en '>' heel belangrijk.
// Deze tekens geven het start en het einde van het bericht aan voor de client.
void koelkast::sendData(string temp)
{
    send(socket_fd, temp.c_str(), strlen(temp.c_str()), 0);
}

// Maakt een connectie met de client. De field descriptor in de functie is dan gelinkt aan de client.
// Hierdoor kunnen meerdere clients tegelijkertijd worden verbonden en weet de server nog steeds welke client waarbij hoort.
void koelkast::connect(int new_socket)
{
    socket_fd = new_socket;
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    std::cout << device_ID << " connected" << endl;

    send(socket_fd, "<id_accept>", strlen("<id_accept>"), 0);
    connected = 1;
}

// yeet de buffer.
void koelkast::yeetbuffer()
{
    memset(buff, 0, 1024);
}