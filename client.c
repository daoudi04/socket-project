#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

struct AcceptedSocket {
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

void startListeningAndPrintMessagesOnNewThreads(struct AcceptedSocket *client_socket);
void *listenAndPrint(void *arg);

char name[1024];

int main()
{
    int socket_client = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_client == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3001);

    if (inet_pton(AF_INET,
                  "127.0.0.1",
                  &server_addr.sin_addr.s_addr) != 1)
    {
        perror("inet_pton");
        close(socket_client);
        return EXIT_FAILURE;
    }

    if (connect(socket_client,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(socket_client);
        return EXIT_FAILURE;
    }

    printf("====================================\n");
    printf("       CONNECTÉ AU SERVEUR\n");
    printf("====================================\n");

    struct AcceptedSocket client_socket;

    client_socket.acceptedSocketFD = socket_client;
    client_socket.address = server_addr;
    client_socket.error = 0;
    client_socket.acceptedSuccessfully = true;

    printf("Votre nom : ");
    fflush(stdout);

    fgets(name, sizeof(name), stdin);

    name[strcspn(name, "\n")] = '\0';

    printf("\nBienvenue %s !\n", name);
    printf("------------------------------------\n");

    startListeningAndPrintMessagesOnNewThreads(&client_socket);

    while (1)
    {
        char message[1024];

        printf("%s > ", name);
        fflush(stdout);

        if (fgets(message, sizeof(message), stdin) == NULL)
        {
            break;
        }

        char final_message[4096];

        snprintf(final_message,
                 sizeof(final_message),
                 "%s : %s",
                 name,
                 message);

        ssize_t bytes_sent =
            send(socket_client,
                 final_message,
                 strlen(final_message),
                 0);

        if (bytes_sent == -1)
        {
            perror("send");
            break;
        }

        if (strcmp(message, "bye\n") == 0)
        {
            printf("\nDéconnexion...\n");
            break;
        }
    }

    close(socket_client);

    return 0;
}

void startListeningAndPrintMessagesOnNewThreads(
    struct AcceptedSocket *client_socket)
{
    pthread_t id;

    pthread_create(&id,
                   NULL,
                   listenAndPrint,
                   client_socket);

    pthread_detach(id);
}

void *listenAndPrint(void *arg)
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
                printf("\nServeur déconnecté.\n");
            }

            break;
        }

        buffer[bytes_received] = '\0';

        printf("\r");
        printf("\033[2K");

        printf("%s", buffer);

        printf("%s > ", name);

        fflush(stdout);
    }

    return NULL;
}