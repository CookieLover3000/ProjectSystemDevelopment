#include "headers/schemerlamp.h"

schemerlamp::schemerlamp(string ID) : socketMeubel(ID), device_ID(ID), connected(0), S1(false), S2(false), S3(false)
{
}

schemerlamp::~schemerlamp()
{
}

// Leest de data die door de client gestuurd wordt en verwerkt deze.
void schemerlamp::readData()
{
    string beweeg = "beweeg";
    string S4 = "S4";
    yeetbuffer();
    if (connected && read(socket_fd, buff, 1024) > 0)
    {
        cout << buff;

        if (beweeg.compare(buff) == 0)
            lampStand();
        else if(S4.compare(buff) == 0)
            beweging = false;
        yeetbuffer();
    }
}
// Voor het versturen van data zijn '<' en '>' heel belangrijk.
// Deze tekens geven het start en het einde van het bericht aan voor de client.
void schemerlamp::sendData(string temp)
{
    send(socket_fd, temp.c_str(), strlen(temp.c_str()), 0);
}

// Maakt een connectie met de client. De field descriptor in de functie is dan gelinkt aan de client.
// Hierdoor kunnen meerdere clients tegelijkertijd worden verbonden en weet de server nog steeds welke client waarbij hoort.
void schemerlamp::connect(int new_socket)
{
    socket_fd = new_socket;
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    cout << device_ID << " connected" << endl;
    send(socket_fd, "<id_accept>", strlen("<id_accept>"), 0);
    connected = 1;
}
// yeet de buffer.
void schemerlamp::yeetbuffer()
{
    memset(buff, 0, 1024);
}

bool schemerlamp::isAanHetBewegen()
{
    return beweging;
}

void schemerlamp::lampStand()
{
    beweging = true;
    if (!S1 && !S2)
    {
        sendData("<uit>");
        S1 = true;
        S2 = false;
    }

    else if (S1 && !S2)
    {
        S1 = false;
        S2 = true;
        sendData("<aan>");
    }

    else if (!S1 && S2)
    {
        S1 = false;
        S2 = false;
        sendData("<hoog>");
    }
}