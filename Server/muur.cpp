#include "headers/muur.h"

muur::muur(string ID) : socketMeubel(ID), device_ID(ID), connected(0), automatisch(false), uit(false), lichtLock(false), kleur(0)
{
}

muur::~muur()
{
}

// Leest de data die door de client gestuurd wordt en verwerkt deze.
void muur::readData()
{
    string lichtaan = "aan";
    string lichtuit = "uit";
    string autoUit = "autoUit";
    yeetbuffer();
    if (connected && read(socket_fd, buff, 1024) > 0)
    {
        cout << "Muur: " << buff << endl;

        if (lichtaan.compare(buff) == 0)
            sendData("<aan>");
        else if (lichtuit.compare(buff) == 0)
            sendData("<uit>");
        else if (autoUit.compare(buff) == 0)
            sendData("<autoUit>");
        else
            potentiometer(buff);
        yeetbuffer();
    }
}
// Voor het versturen van data zijn '<' en '>' heel belangrijk.
// Deze tekens geven het start en het einde van het bericht aan voor de client.
void muur::sendData(string temp)
{
    send(socket_fd, temp.c_str(), strlen(temp.c_str()), 0);
}

// Maakt een connectie met de client. De field descriptor in de functie is dan gelinkt aan de client.
// Hierdoor kunnen meerdere clients tegelijkertijd worden verbonden en weet de server nog steeds welke client waarbij hoort.
void muur::connect(int new_socket)
{
    socket_fd = new_socket;
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    cout << device_ID << " connected" << endl;
    send(socket_fd, "<id_accept>", strlen("<id_accept>"), 0);
    connected = 1;
}
// yeet de buffer.
void muur::yeetbuffer()
{
    memset(buff, 0, 1024);
}

// Krijgt de waardes van de potentiometer en gebruikt deze om het licht van de muur aan te sturen.
void muur::potentiometer(char *a)
{
    int a1 = atoi(a);
    if (a1 != 0)
    {
        if (a1 < 2 && !automatisch)
        {
            sendData("<auto>");
            automatisch = true;
            uit = false;
        }
        if (a1 >= 6 && a1 < 100 && !uit)
        {
            sendData("<uit>");
            uit = true;
            automatisch = false;
            lichtLock = true;
        }
        if (automatisch || uit)
        {
            if (a1 > 150 && a1 < 600)
                sendData("<ga dicht>");
            else if (a1 > 600 && a1 < 1022)
                sendData("<ga open>");
        }
        if (a1 > 1022)
        {
            automatisch = false;
            uit = false;
            lichtLock = false;
        }
        if (!automatisch && !lichtLock)
        {
            if ((a1 >= 800) && (a1 < 1022) && (kleur != 1))
            {
                sendData("<geel>");
                kleur = 1;
            }
            else if ((a1 >= 600) && (a1 < 800) && (kleur != 2))
            {
                sendData("<blauw>");
                kleur = 2;
            }
            else if ((a1 >= 400)&& (a1 < 600) && (kleur != 3))
            {
                sendData("<paars>");
                kleur = 3;
            }
            else if ((a1 >= 200) && (a1 < 400) && (kleur != 4))
            {
                sendData("<rood>");
                kleur = 4;
            }
        }
    }
}
