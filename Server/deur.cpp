#include "headers/deur.h"

deur::deur(string ID, zuil *Z) : socketMeubel(ID), device_ID(ID), connected(0), open(false), z(Z)
{
}

deur::~deur()
{
}

// Leest de data die door de client gestuurd wordt en verwerkt deze.
void deur::readData()
{
    string knopDeur = "<knopDeur>";
    string deurbel = "<deurbel>";
    bool test = true;
    yeetbuffer();
    if (connected && read(socket_fd, buff, 1024) > 0)
    {
        cout << buff;

        if (knopDeur.compare(buff) == 0)
            deurbewegen();
        else if (deurbel.compare(buff) == 0)
        {
            z->sendData("<deurbel>");
        }
        yeetbuffer();
    }
    if (connected && z->returnRookAlarm() && !open)
    {
        deurbewegen();
    }

}

// Voor het versturen van data zijn '<' en '>' heel belangrijk.
// Deze tekens geven het start en het einde van het bericht aan voor de client.
void deur::sendData(string temp)
{
    send(socket_fd, temp.c_str(), strlen(temp.c_str()), 0);
}

// Maakt een connectie met de client. De field descriptor in de functie is dan gelinkt aan de client.
// Hierdoor kunnen meerdere clients tegelijkertijd worden verbonden en weet de server nog steeds welke client waarbij hoort.
void deur::connect(int new_socket)
{
    socket_fd = new_socket;
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    cout << device_ID << " connected" << endl;

    send(socket_fd, "<id_accept>", strlen("<id_accept>"), 0);
    connected = 1;
}

// zegt tegen de client of de deur open of dicht moet. open bewaart de stand van de deur (dicht op startup).
void deur::deurbewegen()
{
    if (open)
    {
        send(socket_fd, "<dicht>", strlen("<dicht>"), 0);
        open = false;
        cout << "ga dicht" << endl;
    }
    else
    {
        send(socket_fd, "<open>", strlen("<open>"), 0);
        open = true;
        cout << "ga open" << endl;
    }
}
// yeet de buffer.
void deur::yeetbuffer()
{
    memset(buff, 0, 1024);
}