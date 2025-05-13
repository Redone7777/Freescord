#include "../include/user.h"

struct user *user_accept(int sl) {
    struct user *u = malloc(sizeof(struct user));
    if (!u) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    u->address = malloc(sizeof(struct sockaddr));
    if (!u->address) {
        perror("malloc");
        free(u);
        exit(EXIT_FAILURE);
    }

    u->addr_len = sizeof(struct sockaddr);
    u->sock = accept(sl, u->address, &u->addr_len);
    if (u->sock < 0) {
        perror("accept");
        free(u->address);
        free(u);
        exit(EXIT_FAILURE);
    }

    u->username = malloc(32 * sizeof(char));
    if (!u->username) {
        perror("malloc");
        free(u->address);
        free(u);
        exit(EXIT_FAILURE);
    }

    return u;
}

void user_free(struct user *user) {
    if (user) {
        if (user->address) free(user->address);
        if (user->username) free(user->username);
        if (user->sock >= 0) close(user->sock);
        free(user);
    }
}
