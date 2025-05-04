#include "../include/serveur.h"

int main(int argc, char *argv[]) {
    uint16_t port = argc == 2 ? atoi(argv[1]) : PORT_FREESCORD;

    int listenFD = create_listening_sock(port);
    CHECK_ERR(listenFD, "create_listening_sock");
    printf("Listening on port %d\n", port);

    // Boucle d'acceptation des connexions
    while (1) {
        // Acceptation d'une connexi
        struct user *u = user_accept(listenFD);

        // Création d'un thread pour gérer le client
        pthread_t thread;
        int threadRes = pthread_create(&thread, NULL, handle_client, u);
        CHECK_ERR(threadRes, "pthread_create");

        // Détachement du thread
        int detachRes = pthread_detach(thread);
        CHECK_ERR(detachRes, "pthread_detach");
    }

    // Fermeture de la socket
    close(listenFD);
    printf("Server stopped\n");

    return EXIT_SUCCESS;
}

int create_listening_sock(uint16_t port) {
    // Création de la socket
    int sockFD = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_ERR(sockFD, "socket");

    // Configuration de la socket
    struct sockaddr_in socketAdresse;
    socketAdresse.sin_family = AF_INET;
    socketAdresse.sin_port = htons(port);
    socketAdresse.sin_addr.s_addr = INADDR_ANY;

    // Liaison de la socket
    int bindRes =
        bind(sockFD, (struct sockaddr *)&socketAdresse, sizeof(socketAdresse));
    CHECK_ERR(bindRes, "bind");

    // Attente de connexions
    int listenRes = listen(sockFD, MAX_CLIENTS);
    CHECK_ERR(listenRes, "listen");

    return sockFD;
}

void *handle_client(void *user) {
    struct user *u = (struct user *)user;

    // Message de bienvenue général
    const char *welcome = "Bienvenue sur Freescord !\n";
    int sent = send(u->sock, welcome, strlen(welcome), 0);
    CHECK_ERR(sent, "send");

    // Demander le pseudo
    ask_username(u->sock, u->username, 32);

    // Message sur le serveur

    printf("\nUtilisateur connecté : [%s]\n\n", u->username);

    // Messagerie
    char BUFFER[BUFFER_SIZE];
    while (recv(u->sock, BUFFER, sizeof(BUFFER), 0) > 0) {
        printf("%s : %s", u->username, BUFFER);
    }

    user_free(u);
    return NULL;
}
