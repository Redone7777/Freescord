#include "../include/serveur.h"

/*================== Variables globales ==================*/
int socketFD;
int myTube[2];
LIST *connectUsers;
pthread_t threadRepeater;
pthread_mutex_t mutexUser = PTHREAD_MUTEX_INITIALIZER;

/*================== Fonction principale ==================*/
int main(int argc, char *argv[]) {
    uint16_t port = argc == 2 ? atoi(argv[1]) : PORT_FREESCORD;

    // Gestion de la fermeture du serveur
    signal(SIGINT, on_exit);

    // Création du pipe
    int pipeRes = pipe(myTube);
    CHECK_ERR(pipeRes, "pipe");

    connectUsers = list_create();

    // Création de la socket d'écoute
    socketFD = create_listening_sock(port);
    printf("Listening on port %d\n", port);

    // Lancement du thread répéteur
    int repThreadRes = pthread_create(&threadRepeater, NULL, read_tupe, NULL);
    CHECK_ERR(repThreadRes, "pthread_create");

    // Boucle d'acceptation des clients
    while (1) {
        struct user *u = user_accept(socketFD);
        pthread_mutex_lock(&mutexUser);
        connectUsers = list_add(connectUsers, u);
        pthread_mutex_unlock(&mutexUser);

        pthread_t threadUser;
        int threadRes = pthread_create(&threadUser, NULL, handle_client, u);
        CHECK_ERR(threadRes, "pthread_create");

        int detachRes = pthread_detach(threadUser);
        CHECK_ERR(detachRes, "pthread_detach");
    }

    return EXIT_SUCCESS;
}

/*================== Création de la socket d'écoute ==================*/
int create_listening_sock(uint16_t port) {
    int sockFD = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_ERR(sockFD, "socket");

    // Option pour réutiliser l'adresse
    int opt = 1;
    setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in socketAdresse;
    socketAdresse.sin_family = AF_INET;
    socketAdresse.sin_port = htons(port);
    socketAdresse.sin_addr.s_addr = INADDR_ANY;

    int bindRes =
        bind(sockFD, (struct sockaddr *)&socketAdresse, sizeof(socketAdresse));
    CHECK_ERR(bindRes, "bind");

    int listenRes = listen(sockFD, MAX_CLIENTS);
    CHECK_ERR(listenRes, "listen");

    return sockFD;
}

/*================== Gestion d'un client ==================*/
void *handle_client(void *user) {
    struct user *u = (struct user *)user;

    // Message de bienvenue
    const char *welcome =
        "__        __   _                            _         \n"
        "\\ \\      / /__| | ___ ___  _ __ ___   ___  | |__  ___ \n"
        " \\ \\ /\\ / / _ \\ |/ __/ _ \\| '_ ` _ \\ / _ \\ | '_ \\/ __|\n"
        "  \\ V  V /  __/ | (_| (_) | | | | | |  __/ | | | \\__ \\\n"
        "   \\_/\\_/ \\___|_|\\___\\___/|_| |_| |_|\\___| |_| |_|___/\n"
        "                                                     \n"
        "  Bienvenue sur le serveur Freescord !              \n"
        "  Ici, on code, on rigole, et parfois... on crash. 💥\n";

    int sent = send(u->sock, welcome, strlen(welcome), 0);
    CHECK_ERR(sent, "send");

    // Demande du pseudo
    ask_username(u->sock, u->username, 16);

    printf("\n[CONNEXION] Utilisateur connecté : %s\n", u->username);

    char BUFFER[BUFFER_SIZE];
    int recvRes;

    while (1) {
        // Réception du message
        recvRes = recv(u->sock, BUFFER, sizeof(BUFFER) - 1, 0);

        // Vérifier si le client s'est déconnecté
        if (recvRes <= 0) {
            if (recvRes == 0)
                printf("[DECONNEXION] Connexion fermée par le client %s\n",
                       u->username);
            else
                perror("recv");
            break;
        }

        BUFFER[recvRes] = '\0';

        // Vérifier si c'est une commande de déconnexion
        if (is_exit_command(BUFFER)) {
            printf("[DECONNEXION] %s a quitté le chat\n", u->username);
            break;
        }

        // Ajouter le pseudo devant le message
        struct message_info msg;
        msg.sender_socket = u->sock;
        snprintf(msg.content, sizeof(msg.content), "%s: %s\n", u->username,
                 BUFFER);

        printf("[MESSAGE] %s", msg.content);

        // Écriture dans le tube
        int writeRes = write(myTube[1], &msg, sizeof(msg));
        CHECK_ERR(writeRes, "write");
    }

    // Supprimer l'utilisateur de la liste des connectés
    pthread_mutex_lock(&mutexUser);
    list_remove_element(connectUsers, u);
    pthread_mutex_unlock(&mutexUser);

    // Libérer la structure utilisateur
    user_free(u);

    return NULL;
}

/*================== Lecture du pipe et envoie à tous  ==================*/
void *read_tupe(void *arg) {
    struct message_info msg;

    while (1) {
        int readRes = read(myTube[0], &msg, sizeof(msg));
        if (readRes <= 0) {
            if (readRes < 0) perror("read");
            continue;
        }

        send_messageAll(connectUsers, msg.content, msg.sender_socket);
    }

    return NULL;
}

/*================== Envoi aux utilisateur ==================*/
void repeat_message(struct user *u, char *message) {
    int sent = send(u->sock, message, strlen(message), 0);
    CHECK_ERR(sent, "send");
}

void send_messageAll(LIST *users, char *message, int sender_socket) {
    if (!users) return;

    pthread_mutex_lock(&mutexUser);

    for (NODE *curr = users->first; curr; curr = curr->next) {
        struct user *u = curr->elt;
        if (u->sock != sender_socket) repeat_message(u, message);
    }

    pthread_mutex_unlock(&mutexUser);
}

int is_exit_command(char *buffer) {
    while (*buffer && (*buffer == ' ' || *buffer == '\t')) buffer++;
    return (strcmp(buffer, "/exit") == 0);
}

void on_exit(int sig) {
    close(myTube[0]);
    close(myTube[1]);
    close(socketFD);

    pthread_mutex_destroy(&mutexUser);

    if (connectUsers) {
        list_free(connectUsers, (void *)user_free);
    }

    printf("\n[ARRET] Serveur arrêté\n");

    exit(EXIT_SUCCESS);
}

// Demande au client de saisir un username et le stocke dans username
void ask_username(int client, char *username, size_t size) {
    char buffer[64];
    int status, received, sent;
    const char *prompt = "Entrez votre pseudo : ";

    do {
        sent = send(client, prompt, strlen(prompt), 0);
        CHECK_ERR(sent, "send prompt");

        received = recv(client, buffer, sizeof(buffer) - 1, 0);
        CHECK_ERR(received, "recv");

        // Remplace le CRLF par un NUL
        buffer[received] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';

        status = check_nickname(buffer, size);
        send_error_nickname(client, status);

    } while (status != 0);

    strncpy(username, buffer, size - 1);
    username[size - 1] = '\0';
}

int check_nickname(char *buffer, size_t size) {
    buffer[strcspn(buffer, "\r\n")] = '\0';
    size_t len = strlen(buffer);

    // 2. Vérifie la taille du nickname
    if (len == 0 || len >= size) return 2;

    for (size_t i = 0; i < len; i++)
        if (buffer[i] == ' ' || buffer[i] == ':') return 2;

    // 1. Vérifie si le nickname est déjà pris
    pthread_mutex_lock(&mutexUser);
    for (NODE *curr = connectUsers->first; curr; curr = curr->next) {
        struct user *u = curr->elt;
        if (strcmp(u->username, buffer) == 0) {
            pthread_mutex_unlock(&mutexUser);
            return 1;
        }
    }
    pthread_mutex_unlock(&mutexUser);

    // 0. Le nickname est valide
    return 0;
}

void send_error_nickname(int client, int status) {
    const char *responses[] = {
        "0 | Pseudo accepté.\n",
        "1 | Ce pseudo est déjà pris.\nEntrez votre pseudo : ",
        "2 | Pseudo invalide.\nRègles :\n- Pas d'espaces ni de ':'\n- Taille "
        "entre 1 et 15 caractères\nEntrez votre pseudo : ",
        "3 | Erreur inconnue.\nEntrez votre pseudo : "};

    int idx = (status >= 0 && status <= 2) ? status : 3;
    send(client, responses[idx], strlen(responses[idx]), 0);
}