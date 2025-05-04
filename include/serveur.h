#ifndef SERVEUR_H
#define SERVEUR_H

#include <arpa/inet.h>
#include <pthread.h>
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

/** Gérer toutes les communications avec le client renseigné dans
 * user, qui doit être l'adresse d'une struct user */
void *handle_client(void *user);

/** Créer et configurer une socket d'écoute sur le port donné en argument
 * retourne le descripteur de cette socket, ou -1 en cas d'erreur */
int create_listening_sock(uint16_t port);

#endif  // SERVEUR_H
