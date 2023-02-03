#include "headers/stoel.h"

stoel::stoel(string ID, deur *beweeg) : socketMeubel(ID), device_ID(ID), connected(0), Deur(beweeg)
{
}

stoel::~stoel()
{
}

// Leest de data die door de client gestuurd wordt en verwerkt deze.
void stoel::readData()
{
    string knopStoel = "<knopStoel>";
    string drukStoel1 = "Persoon zit op stoel";
    string drukStoel2 = "Persoon zit niet op stoel";
    yeetbuffer();
    if (connected && read(socket_fd, buff, 1024) > 0)
    {
        cout << buff;

        if (knopStoel.compare(buff) == 0)
            Deur->deurbewegen();
        else if (drukStoel1.compare(buff) == 0)
            zitOpStoel = true;
        else if (drukStoel2.compare(buff) == 0)
            zitOpStoel = false;


        yeetbuffer();
    }
}

// Voor het versturen van data zijn '<' en '>' heel belangrijk.
// Deze tekens geven het start en het einde van het bericht aan voor de client.
void stoel::sendData(string temp)
{
    send(socket_fd, temp.c_str(), strlen(temp.c_str()), 0);
}

// Maakt een connectie met de client. De field descriptor in de functie is dan gelinkt aan de client.
// Hierdoor kunnen meerdere clients tegelijkertijd worden verbonden en weet de server nog steeds welke client waarbij hoort.
void stoel::connect(int new_socket)
{
    socket_fd = new_socket;
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    std::cout << device_ID << " connected" << endl;

    send(socket_fd, "<id_accept>", strlen("<id_accept>"), 0);
    connected = 1;
}

// zegt tegen de client of de deur open of dicht moet. open bewaart de stand van de deur (dicht op startup).
void stoel::yeetbuffer()
{
    memset(buff, 0, 1024);
}

// returnt of er iemand op de stoel zit.
bool stoel::zitPersoonOpStoel()
{
    return zitOpStoel;
}