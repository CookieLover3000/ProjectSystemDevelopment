#include "headers/server.h"

#define PORT 8080

int main(int argc, char const *argv[])
{
    /* ID's voor de sockets */
    string ID_DEUR = "DEUR_ID";
    string ID_STOEL = "STOEL_ID";
    string ID_ZUIL = "ZUIL_ID";
    string ID_BED = "BED_ID";
    string ID_KOELKAST = "KOELKAST_ID";
    string ID_LAMP = "LAMP_ID";
    string ID_MUUR = "MUUR_ID";

    balie b;
    muur socketmuur(ID_MUUR);
    zuil socketzuil(ID_ZUIL, &b, &socketmuur);
    koelkast socketkoelkast(ID_KOELKAST, &socketzuil, &b);
    deur socketdeur(ID_DEUR, &socketzuil);
    stoel socketstoel(ID_STOEL, &socketdeur);
    bed socketbed(ID_BED);
    schemerlamp socketlamp(ID_LAMP);
    Alarm gevallen(&socketbed, &socketlamp, &socketstoel, &b);

    /* server init enzo */
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) // maak de socket aan
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    while (1)
    {
        if (listen(server_fd, 10) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) > 0)
        {
            send(new_socket, "<id?>", strlen("<id?>"), 0);

            // Creating socket file descriptor
            valread = read(new_socket, buffer, 1024);
            if (ID_DEUR.compare(buffer) == 0)
                socketdeur.connect(new_socket);
            else if (ID_STOEL.compare(buffer) == 0)
                socketstoel.connect(new_socket);
            else if (ID_ZUIL.compare(buffer) == 0)
                socketzuil.connect(new_socket);
            else if (ID_BED.compare(buffer) == 0)
                socketbed.connect(new_socket);
            else if (ID_LAMP.compare(buffer) == 0)
                socketlamp.connect(new_socket);
            else if (ID_MUUR.compare(buffer) == 0)
                socketmuur.connect(new_socket);
            else if (ID_KOELKAST.compare(buffer) == 0)
                socketkoelkast.connect(new_socket);
            else
            {
                std::cout << buffer << " geen id" << std::endl;
                memset(buffer, 0, 1024);
            }
        }
        socketdeur.readData();
        socketzuil.readData();
        socketbed.readData();
        socketstoel.readData();
        socketlamp.readData();
        socketmuur.readData();
        socketkoelkast.readData();
        gevallen.melding();
    }
    return 0;
}