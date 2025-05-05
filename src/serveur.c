#include "../include/serveur.h"

/*================== Variables globales ==================*/
int myTube[2];

int socketFD;
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
        connectUsers = list_add(connectUsers, u);

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

    struct sockaddr_in socketAdresse;
    socketAdresse.sin_family = AF_INET;
    socketAdresse.sin_port = htons(port);
    socketAdresse.sin_addr.s_addr = INADDR_ANY;

    // Autoriser la réutilisation de l'adresse
    int optval = 1;
    setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

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
    const char *welcome = "Bienvenue sur Freescord !\r\n";
    int sent = send(u->sock, welcome, strlen(welcome), 0);
    CHECK_ERR(sent, "send");

    // Demande du pseudo
    ask_username(u->sock, u->username, 32);
    printf("\nUtilisateur connecté : [%s]\n\n", u->username);

    char BUFFER[BUFFER_SIZE];
    int recvRes;

    while (1) {
        // Réception du message
        recvRes = recv(u->sock, BUFFER, sizeof(BUFFER), 0);

        // Vérifier si le client s'est déconnecté
        if (recvRes <= 0) {
            if (recvRes == 0)
                printf("Connexion fermée par le client [%s]\n", u->username);
            else
                perror("recv");
            break;
        }

        BUFFER[recvRes] = '\0';

        // Vérifier si c'est une commande de déconnexion
        if (is_exit_command(BUFFER)) break;

        // Ajouter le pseudo devant le message
        struct message_info msg;
        msg.sender_socket = u->sock;
        snprintf(msg.content, sizeof(msg.content), "%s : %s", u->username,
                 BUFFER);

        printf("%s", msg.content);

        // Écriture dans le tube
        int writeRes = write(myTube[1], &msg, sizeof(msg));
        CHECK_ERR(writeRes, "write");
    }

    printf("Utilisateur déconnecté : [%s]\n", u->username);

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

    list_free(connectUsers, (void *)user_free);

    printf("\n[Serveur arrêté]\n");

    exit(EXIT_SUCCESS);
}
