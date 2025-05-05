#include "../include/client.h"

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
        exit(EXIT_FAILURE);
    }

    Buffer *socketBuf = buff_create(socketFD, BUFFER_SIZE);
    if (socketBuf == NULL) {
        fprintf(stderr, "[CLIENT ERROR] - buffer socket\n");
        buff_free(stdinBuf);
        exit(EXIT_FAILURE);
    }

    welcome_sequence(socketFD);
    printf("%s", PROMPT);

    struct pollfd fds[] = {{STDIN_FILENO, POLLIN, 0}, {socketFD, POLLIN, 0}};

    bool done = false;
    while (!done) {
        // Attendre que l'un des deux buffers soit prêt
        if (!buff_ready(stdinBuf) && !buff_ready(socketBuf)) {
            int pollResult = poll(fds, 2, -1);
            CHECK_ERR(pollResult, "poll");
        }

        if (buff_ready(stdinBuf) || (fds[0].revents & POLLIN))
            if (handle_stdin(socketFD, stdinBuf) < 0) done = true;

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

    // Message de bienvenue
    int received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    CHECK_ERR(received, "recv");
    buffer[received] = '\0';
    printf("%s", buffer);

    // Demande du pseudo
    printf("Entrez votre pseudo : ");
    fflush(stdout);

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) exit(EXIT_FAILURE);
    int sent = send(sock, buffer, strlen(buffer), 0);
    CHECK_ERR(sent, "send");
}

/*====== Gère les saisies clavier ======*/
int handle_stdin(int sock, Buffer *stdinBuf) {
    char buffer[BUFFER_SIZE];

    if (buff_fgets(stdinBuf, buffer, sizeof(buffer)) == NULL) {
        if (buff_eof(stdinBuf)) {
            return -1;
        }
        return 0;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    if (is_exit_command(buffer)) {
        printf("\n[Déconnexion...]\n");
        send(sock, "/exit", strlen("/exit"), 0);
        return -1;
    }

    strcat(buffer, "\n");

    /* LF → CRLF */
    char *crlfLine = lf_to_crlf(buffer);
    if (crlfLine == NULL) {
        fprintf(stderr, "[CLIENT ERROR] - LF to CRLF\n");
        return 0;
    }

    /* envoi au serveur */
    int sent = send(sock, crlfLine, strlen(crlfLine), 0);
    CHECK_ERR(sent, "send");

    printf("%s", PROMPT);
    return 0;
}

/*====== Gère les messages reçus ======*/
int handle_socket(int sock, Buffer *socketBuf) {
    char buffer[BUFFER_SIZE];

    if (buff_fgets_crlf(socketBuf, buffer, sizeof(buffer)) == NULL) {
        if (buff_eof(socketBuf)) {
            printf("\n[Serveur déconnecté]\n");
            return -1;
        }
        return 0;
    }

    /* CRLF → LF */
    char *lfLine = crlf_to_lf(buffer);
    if (lfLine == NULL) {
        fprintf(stderr, "[CLIENT ERROR] - CRLF to LF\n");
        return 0;
    }

    lfLine[strcspn(lfLine, "\n")] = '\0';

    printf("\r\033[K%s\n%s", lfLine, PROMPT);
    return 0;
}

int is_exit_command(char *buffer) {
    while (*buffer && (*buffer == ' ' || *buffer == '\t')) buffer++;
    return (strcmp(buffer, "/exit") == 0);
}