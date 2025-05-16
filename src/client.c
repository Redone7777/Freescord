#include "../include/client.h"

#include "../include/utils.h"

/*====== Fonction principale ======*/
int main(int argc, char *argv[]) {
    char *host = argc == 3 ? argv[1] : CONNECTION_HOST;
    uint16_t port = argc == 3 ? atoi(argv[2]) : PORT_FREESCORD;

    setvbuf(stdout, NULL, _IONBF, 0);

    int socketFD = connect_serveur_tcp(host, port);

    // Création des buffers
    Buffer *stdinBuf = buff_create(STDIN_FILENO, BUFFER_SIZE);
    if (stdinBuf == NULL) {
        fprintf(stderr, "[CLIENT ERROR] - buffer stdin\n");
        close(socketFD);
        exit(EXIT_FAILURE);
    }

    Buffer *socketBuf = buff_create(socketFD, BUFFER_SIZE);
    if (socketBuf == NULL) {
        fprintf(stderr, "[CLIENT ERROR] - buffer socket\n");
        buff_free(stdinBuf);
        close(socketFD);
        exit(EXIT_FAILURE);
    }

    welcome_sequence(socketFD);
    printf("%s", PROMPT);
    fflush(stdout);

    struct pollfd fds[] = {{STDIN_FILENO, POLLIN, 0}, {socketFD, POLLIN, 0}};

    bool done = false;
    while (!done) {
        int pollResult = poll(fds, 2, -1);
        CHECK_ERR(pollResult, "poll");

        // Gestion de l'entrée standard (stdin)
        if (buff_ready(stdinBuf) || (fds[0].revents & POLLIN))
            if (handle_stdin(socketFD, stdinBuf) < 0) done = true;

        // Gestion des messages du serveur
        if (buff_ready(socketBuf) || (fds[1].revents & POLLIN))
            if (handle_socket(socketFD, socketBuf) < 0) done = true;
    }

    buff_free(stdinBuf);
    buff_free(socketBuf);

    close(socketFD);
    return EXIT_SUCCESS;
}

/*====== Connexion TCP ======*/
int connect_serveur_tcp(char *adresse, uint16_t port) {
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_ERR(socketFD, "socket");

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    int inetRes = inet_pton(AF_INET, adresse, &serverAddr.sin_addr);
    CHECK_ERR(inetRes, "inet_pton");

    int connectRes =
        connect(socketFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    CHECK_ERR(connectRes, "connect");

    return socketFD;
}

/*====== Séquence d'accueil et pseudo ======*/
void welcome_sequence(int sock) {
    char buffer[BUFFER_SIZE];
    int received, sended, status;

    // Message de bienvenue
    received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    CHECK_ERR(received, "recv welcome");

    buffer[received] = '\0';
    printf("%s", buffer);
    fflush(stdout);

    do {
        received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        CHECK_ERR(received, "recv");

        buffer[received] = '\0';
        printf("%s", buffer);
        fflush(stdout);

        // Lire le pseudo depuis stdin
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            CHECK_ERR(-1, "fgets");
        buffer[strcspn(buffer, "\n")] = '\0';

        lf_to_crlf(buffer);
        sended = send(sock, buffer, strlen(buffer), 0);
        CHECK_ERR(sended, "send nickname");

        // Recevoir la réponse du serveur
        received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        CHECK_ERR(received, "recv");
        buffer[received] = '\0';

        crlf_to_lf(buffer);
        printf("%s", buffer);
        fflush(stdout);

        // Extraire le status (premier caractère)
        status = buffer[0] - '0';
    } while (status != 0);

    printf("\n\n");
}

/*====== Gère les saisies clavier ======*/
int handle_stdin(int sock, Buffer *stdinBuf) {
    char buffer[BUFFER_SIZE];
    int sended;

    if (buff_fgets(stdinBuf, buffer, sizeof(buffer)) == NULL) return -1;

    buffer[strcspn(buffer, "\n")] = '\0';

    if (is_exit_command(buffer)) {
        lf_to_crlf(buffer);
        sended = send(sock, buffer, strlen(buffer), 0);
        CHECK_ERR(sended, "send exit");

        printf("\nDéconnexion...\n");
        close(sock);
        return -1;
    }

    lf_to_crlf(buffer);
    int sendRes = send(sock, buffer, strlen(buffer), 0);
    CHECK_ERR(sendRes, "send");

    printf("%s", PROMPT);
    fflush(stdout);
    return 0;
}

/*====== Gère les messages reçus ======*/
int handle_socket(int sock, Buffer *socketBuf) {
    char buffer[BUFFER_SIZE];

    if (buff_fgets_crlf(socketBuf, buffer, sizeof(buffer)) == NULL) {
        if (buff_eof(socketBuf)) {
            printf("\nLa connexion au serveur a été fermée.\n");
            return -1;
        }
        return 0;
    }

    crlf_to_lf(buffer);

    printf("\r");
    printf("\033[K");  // Effacer la ligne
    printf("%s", buffer);

    // Réafficher le prompt
    printf("%s", PROMPT);
    fflush(stdout);

    return 0;
}

int is_exit_command(char *buffer) { return (strcmp(buffer, "/exit") == 0); }