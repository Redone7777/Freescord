#include "../include/client.h"

int main(int argc, char *argv[]) {
    // Vérification des arguments
    char *host = CONNECTION_HOST;
    uint16_t port = PORT_FREESCORD;
    if (argc == 2) {
        host = argv[1];
    } else if (argc == 3) {
        host = argv[1];
        port = atoi(argv[2]);
    }

    // Connexion au serveur
    int socketFD = connect_serveur_tcp(host, port);

    char BUFFER[1024];

    // Message de bienvenue
    recv(socketFD, BUFFER, sizeof(BUFFER), 0);
    printf("%s", BUFFER);

    // Demande du pseudo
    int received = recv(socketFD, BUFFER, sizeof(BUFFER), 0);
    BUFFER[received] = '\0';
    printf("%s", BUFFER);

    // Envoi du pseudo
    if (fgets(BUFFER, sizeof(BUFFER), stdin) == NULL) exit(EXIT_FAILURE);
    int sent = send(socketFD, BUFFER, strlen(BUFFER), 0);
    CHECK_ERR(sent, "send");

    // échange de messages
    while (1) {
        printf("Moi : ");
        if (fgets(BUFFER, sizeof(BUFFER), stdin) == NULL) break;
        sent = send(socketFD, BUFFER, strlen(BUFFER), 0);
        CHECK_ERR(sent, "send");
    }

    // Fermeture de la connexion
    close(socketFD);
    return EXIT_SUCCESS;
}

int connect_serveur_tcp(char *adresse, uint16_t port) {
    // Créer la socket
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_ERR(socketFD, "socket");

    // Configurer l'adresse du serveur
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    int inetRes = inet_pton(AF_INET, adresse, &serverAddr.sin_addr);
    CHECK_ERR(inetRes, "inet_pton");

    // Se connecter au serveur
    int connectRes =
        connect(socketFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    CHECK_ERR(connectRes, "connect");

    return socketFD;
}