/**
 * @file freescord_gui.h
 * @brief Déclarations pour l'interface graphique GTK3 du client Freescord
 */

#ifndef FREESCORD_GUI_H
#define FREESCORD_GUI_H

#include <arpa/inet.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

// Constantes
#define MAX_USERNAME_LENGTH 32
#define BUFFER_SIZE 1024
#define DEFAULT_SERVER "127.0.0.1"
#define DEFAULT_PORT 4321

// Structure pour les données d'un message à traiter dans le thread principal
typedef struct {
    void *app;
    char *timestamp;
    char *username;
    char *message;
    gboolean is_me;
} MessageData;

// Structure principale de l'application
typedef struct {
    // Interface graphique
    GtkWidget *window;
    GtkWidget *message_view;
    GtkTextBuffer *message_buffer;
    GtkWidget *entry;
    GtkWidget *send_button;
    GtkWidget *connect_button;
    GtkWidget *username_entry;
    GtkWidget *server_entry;
    GtkWidget *port_entry;
    GtkWidget *status_label;
    GtkWidget *scrolled_window;
    GtkCssProvider *provider;

    // Connexion réseau
    int socket_fd;
    int connected;
    int thread_running;
    pthread_t receive_thread;
    pthread_mutex_t mutex;

    // Informations utilisateur
    char username[MAX_USERNAME_LENGTH];
} FreescordApp;

// Fonctions d'initialisation
void init_gui(FreescordApp *app, int argc, char *argv[]);
void setup_css(FreescordApp *app);
void create_widgets(FreescordApp *app);
void connect_signals(FreescordApp *app);

// Handlers d'événements
void on_send_button_clicked(GtkButton *button, FreescordApp *app);
void on_entry_activate(GtkEntry *entry, FreescordApp *app);
void on_connect_button_clicked(GtkButton *button, FreescordApp *app);
void on_window_destroy(GtkWidget *widget, FreescordApp *app);

// Fonctions réseau
int connect_to_server(const char *server, int port);
void send_message(FreescordApp *app, const char *message);
void *receive_messages(void *data);
void process_incoming_message(FreescordApp *app, const char *buffer);
void disconnect_from_server(FreescordApp *app);

// Fonctions d'affichage
void format_message_display(FreescordApp *app, const char *timestamp,
                            const char *username, const char *message,
                            gboolean is_me);
void format_system_message(FreescordApp *app, const char *timestamp,
                           const char *message);
void set_status(FreescordApp *app, const char *status, const char *style_class);
void clear_messages(FreescordApp *app);
void append_raw_message(FreescordApp *app, const char *message);
void show_error_dialog(GtkWindow *parent, const char *title,
                       const char *message);

// Prototypes de fonctions
static gboolean append_message_idle(gpointer data);
static void free_message_data(gpointer data);
static gboolean format_received_message_idle(gpointer data);
static gboolean format_system_message_idle(gpointer data);

#endif /* FREESCORD_GUI_H */