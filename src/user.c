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

// Demande au client de saisir un username et le stocke dans user->username
void ask_username(int client, char *username, size_t size) {
    const char *msg = "Votre Pseudo : ";
    int sent = send(client, msg, strlen(msg), 0);
    if (sent < 0) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    int received = recv(client, username, size - 1, 0);
    if (received < 0) {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    // retire le \n
    username[received] = '\0';
    char *newline = strchr(username, '\n');
    if (newline) *newline = '\0';
}

void user_free(struct user *user) {
    if (user) {
        free(user->address);
        free(user->username);
        close(user->sock);
        free(user);
    }
}
