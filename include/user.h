#ifndef USER_H
#define USER_H

#include <error.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "list/list.h"

struct user {
    char *username;

    struct sockaddr *address;
    socklen_t addr_len;
    int sock;
};

/** accepter une connection TCP depuis la socket d'écoute sl et retourner un
 * pointeur vers un struct user, dynamiquement alloué et convenablement
 * initialisé */
struct user *user_accept(int sl);

/** libérer toute la mémoire associée à user */
void user_free(struct user *user);

#endif /* USER_H */
