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
    fflush(stdout);

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
    int received, status;

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

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            CHECK_ERR(-1, "fgets");
        buffer[strcspn(buffer, "\n")] = '\0';
        send(sock, buffer, strlen(buffer), 0);

        received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        CHECK_ERR(received, "recv");
        buffer[received] = '\0';
        printf("%s", buffer);
        fflush(stdout);

        status = buffer[0] - '0';
    } while (status != 0);

    printf("\n\n");
}

/*====== Gère les saisies clavier ======*/
int handle_stdin(int sock, Buffer *stdinBuf) {
    char buffer[BUFFER_SIZE];

    // Lire depuis le buffer
    if (buff_fgets(stdinBuf, buffer, sizeof(buffer)) == NULL) return -1;

    // Supprimer le saut de ligne
    buffer[strcspn(buffer, "\n")] = '\0';

    // Vérifier si c'est une commande de sortie
    if (is_exit_command(buffer)) {
        send(sock, buffer, strlen(buffer), 0);
        return -1;
    }

    // Envoyer le message
    int sendRes = send(sock, buffer, strlen(buffer), 0);
    if (sendRes < 0) {
        perror("send");
        return -1;
    }

    printf("%s", PROMPT);  // Réafficher le prompt
    fflush(stdout);
    return 0;
}

/*====== Gère les messages reçus ======*/
int handle_socket(int sock, Buffer *socketBuf) {
    char buffer[BUFFER_SIZE];

    // Lire depuis le buffer de socket
    int bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        if (bytesRead == 0)
            printf("\nLa connexion au serveur a été fermée.\n");
        else
            perror("recv");
        return -1;  // Signal pour quitter
    }

    buffer[bytesRead] = '\0';

    // Supprimer le prompt actuel avant d'afficher le message
    printf("\r");      // Retour en début de ligne
    printf("\033[K");  // Effacer la ligne

    // Afficher le message reçu
    printf("%s", buffer);

    // Réafficher le prompt après le message
    printf("%s", PROMPT);
    fflush(stdout);

    return 0;
}

int is_exit_command(char *buffer) {
    while (*buffer && (*buffer == ' ' || *buffer == '\t')) buffer++;
    return (strcmp(buffer, "/exit") == 0);
}