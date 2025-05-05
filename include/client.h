#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "buffer/buffer.h"
#include "utils.h"

#define PROMPT "Moi : "
#define BUFFER_SIZE 1024
#define PORT_FREESCORD 4321
#define CONNECTION_HOST "127.0.0.1"

#define CHECK_ERR(x, msg)                              \
    if (x < 0) {                                       \
        fprintf(stderr, "[CLIENT ERROR] - %s\n", msg); \
        exit(EXIT_FAILURE);                            \
    }

/** se connecter au serveur TCP d'adresse donnée en argument sous forme de
 * chaîne de caractère et au port donné en argument
 * retourne le descripteur de fichier de la socket obtenue ou -1 en cas
 * d'erreur. */
int connect_serveur_tcp(char *adresse, uint16_t port);

/** Reçoit un message de bienvenue de la part du server */
void welcome_sequence(int sock);

/** Gère l'entrée standard (stdin) et envoie le message au serveur
 * en utilisant un buffer pour la lecture */
int handle_stdin(int sock, Buffer *stdinBuf);

/** Gère la socket du serveur et affiche le message reçu
 * en utilisant un buffer pour la lecture */
int handle_socket(int sock, Buffer *socketBuf);

/** Vérifie si la commande est une commande de déconnexion */
int is_exit_command(char *buffer);

#endif  // CLIENT_H