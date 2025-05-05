#ifndef SERVEUR_H
#define SERVEUR_H

#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "list/list.h"
#include "user.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT_FREESCORD 4321

#define CHECK_ERR(x, msg)                              \
    if (x < 0) {                                       \
        fprintf(stderr, "[SERVER ERROR] - %s\n", msg); \
        exit(EXIT_FAILURE);                            \
    }

/*================== Message avec ID de l'émetteur ==================*/
struct message_info {
    int sender_socket;
    char content[BUFFER_SIZE + 64];
};

/*================== Liste des fonctions ==================*/

/** Gérer toutes les communications avec le client renseigné dans
 * user, qui doit être l'adresse d'une struct user */
void *handle_client(void *user);

/** Créer et configurer une socket d'écoute sur le port donné en argument
 * retourne le descripteur de cette socket, ou -1 en cas d'erreur */
int create_listening_sock(uint16_t port);

/* Gère un client connecté */
void *handle_client(void *user);

/* Thread qui lit dans le tube et distribue les messages */
void *read_tupe(void *arg);

/* Envoie un message à un utilisateur */
void repeat_message(struct user *u, char *message);

/* Envoie un message à tous les utilisateurs connectés sauf l'émetteur */
void send_messageAll(LIST *users, char *message, int sender_socket);

/* Vérifie si la commande est une commande de déconnexion */
int is_exit_command(char *buffer);

/* Fonction de gestion des signaux */
void on_exit(int signum);

#endif  // SERVEUR_H
