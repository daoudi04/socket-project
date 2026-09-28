#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>

struct AcceptedSocket
{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

struct AcceptedSocket acceptedSockets[10];
int acceptedSocketCount = 0;


struct AcceptedSocket *acceptInComingConnection(int socket_serveur);

void startAcceptingIncomingConnections(int socket_serveur);

void receiveAndPrintIncomingDataOnSeparateThread(
    struct AcceptedSocket *client_socket);

void *receiveAndPrintIncomingData(void *arg);

void sendReceivedMessageToOtherClient(
    char *buffer,
    int socketFD);


int main()
{
    int socket_serveur;

    socket_serveur = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_serveur == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }


    int option = 1;

    setsockopt(socket_serveur,
               SOL_SOCKET,
               SO_REUSEADDR,
               &option,
               sizeof(option));


    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(3001);


    if (bind(socket_serveur,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1)
    {
        perror("bind");

        close(socket_serveur);

        return EXIT_FAILURE;
    }


    if (listen(socket_serveur, 5) == -1)
    {
        perror("listen");

        close(socket_serveur);

        return EXIT_FAILURE;
    }


    printf("\n");
    printf("========================================\n");
    printf("          SERVEUR DE CHAT\n");
    printf("========================================\n");
    printf("Port : 3001\n");
    printf("Statut : en attente de connexions...\n");
    printf("========================================\n\n");


    startAcceptingIncomingConnections(socket_serveur);


    close(socket_serveur);

    return 0;
}


void startAcceptingIncomingConnections(int socket_serveur)
{
    while (1)
    {
        struct AcceptedSocket *client_socket =
            acceptInComingConnection(socket_serveur);


        if (!client_socket->acceptedSuccessfully)
        {
            printf("[ERREUR] Impossible d'accepter le client.\n");

            free(client_socket);

            continue;
        }


        if (acceptedSocketCount < 10)
        {
            acceptedSockets[acceptedSocketCount++] =
                *client_socket;
        }


        printf("----------------------------------------\n");
        printf("[CONNEXION] Nouveau client connecté\n");
        printf("Socket : %d\n",
               client_socket->acceptedSocketFD);

        printf("Clients enregistrés : %d\n",
               acceptedSocketCount);

        printf("----------------------------------------\n");


        receiveAndPrintIncomingDataOnSeparateThread(
            client_socket);
    }
}


struct AcceptedSocket *acceptInComingConnection(
    int socket_serveur)
{
    struct sockaddr_in client_addr;

    socklen_t clientAdressSize =
        sizeof(struct sockaddr_in);


    int client_socket =
        accept(socket_serveur,
               (struct sockaddr *)&client_addr,
               &clientAdressSize);


    struct AcceptedSocket *acceptedSocket =
        malloc(sizeof(struct AcceptedSocket));


    acceptedSocket->address =
        client_addr;

    acceptedSocket->acceptedSocketFD =
        client_socket;

    acceptedSocket->acceptedSuccessfully =
        client_socket >= 0;


    if (!acceptedSocket->acceptedSuccessfully)
    {
        acceptedSocket->error =
            client_socket;
    }
    else
    {
        acceptedSocket->error = 0;
    }


    return acceptedSocket;
}


void receiveAndPrintIncomingDataOnSeparateThread(
    struct AcceptedSocket *client_socket)
{
    pthread_t id;


    if (pthread_create(
            &id,
            NULL,
            receiveAndPrintIncomingData,
            client_socket) != 0)
    {
        perror("pthread_create");

        free(client_socket);

        return;
    }


    pthread_detach(id);
}


void *receiveAndPrintIncomingData(void *arg)
{
    struct AcceptedSocket *client =
        (struct AcceptedSocket *)arg;


    while (1)
    {
        char buffer[4096];


        ssize_t bytes_received =
            recv(client->acceptedSocketFD,
                 buffer,
                 sizeof(buffer) - 1,
                 0);


        if (bytes_received <= 0)
        {
            if (bytes_received == -1)
            {
                perror("recv");
            }
            else
            {
                printf("\n");
                printf("----------------------------------------\n");
                printf("[DECONNEXION] Client déconnecté\n");
                printf("Socket : %d\n",
                       client->acceptedSocketFD);

                printf("----------------------------------------\n");
            }

            break;
        }


        buffer[bytes_received] = '\0';


        printf("[MESSAGE] %s", buffer);


        sendReceivedMessageToOtherClient(
            buffer,
            client->acceptedSocketFD);
    }


    close(client->acceptedSocketFD);

    free(client);

    return NULL;
}


void sendReceivedMessageToOtherClient(
    char *buffer,
    int socketFD)
{
    for (int i = 0;
         i < acceptedSocketCount;
         i++)
    {
        if (acceptedSockets[i].acceptedSocketFD
            != socketFD)
        {
            send(
                acceptedSockets[i].acceptedSocketFD,
                buffer,
                strlen(buffer),
                0);
        }
    }
}