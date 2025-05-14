/**
 * @file freescord_gui.c
 * @brief Implémentation de l'interface graphique GTK3 pour le client Freescord
 */

#include "freescord_gui.h"

// Couleurs pour les pseudos des différents utilisateurs
static const char *user_colors[] = {"#FF6B6B", "#4ECDC4", "#45B7D1", "#96CEB4",
                                    "#FFEAA7", "#DDA0DD", "#98D8C8", "#F7DC6F",
                                    "#BB9AF7", "#7AA2F7"};
static const int num_user_colors = sizeof(user_colors) / sizeof(user_colors[0]);

// Déclarations de fonctions statiques
static MessageData *create_message_data(FreescordApp *app,
                                        const char *timestamp,
                                        const char *username,
                                        const char *message, gboolean is_me);
static void free_message_data(MessageData *data);
static gboolean format_received_message_idle(gpointer data);
static gboolean format_system_message_idle(gpointer data);
static gboolean on_connection_error(gpointer data);
static int handle_welcome_sequence(FreescordApp *app);
static gboolean append_message_idle(gpointer data);

/**
 * @brief Point d'entrée de l'application
 */
int main(int argc, char *argv[]) {
    FreescordApp *app = malloc(sizeof(FreescordApp));
    if (!app) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        return EXIT_FAILURE;
    }

    memset(app, 0, sizeof(FreescordApp));

    // Initialisation du mutex
    pthread_mutex_init(&app->mutex, NULL);

    init_gui(app, argc, argv);

    gtk_main();

    // Libération du mutex
    pthread_mutex_destroy(&app->mutex);
    free(app);

    return EXIT_SUCCESS;
}

/**
 * @brief Initialise l'interface graphique
 */
void init_gui(FreescordApp *app, int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Créer la fenêtre principale
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "Freescord");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 900, 700);
    gtk_window_set_position(GTK_WINDOW(app->window), GTK_WIN_POS_CENTER);
    g_signal_connect(app->window, "destroy", G_CALLBACK(on_window_destroy),
                     app);

    // Configuration du CSS
    setup_css(app);

    // Création des widgets
    create_widgets(app);

    // Connexion des signaux
    connect_signals(app);

    // Affichage de la fenêtre
    gtk_widget_show_all(app->window);
}

/**
 * @brief Configure le style CSS amélioré
 */
void setup_css(FreescordApp *app) {
    app->provider = gtk_css_provider_new();

    const char *css =
        /* Thème global moderne et sombre */
        "window { "
        "  background-color: #1a1b26; "
        "  font-family: 'Source Code Pro', 'Consolas', monospace; "
        "} "

        /* Formulaire de connexion avec gradient subtil */
        "entry { "
        "  background: #24283b; "
        "  color: #c0caf5; "
        "  border-radius: 8px; "
        "  border: 1px solid #414868; "
        "  padding: 10px 15px; "
        "  margin: 5px; "
        "  font-size: 14px; "
        "  transition: all 0.3s ease; "
        "} "
        "entry:focus { "
        "  border-color: #7aa2f7; "
        "  box-shadow: 0 0 0 3px rgba(122, 162, 247, 0.2); "
        "} "

        /* Boutons avec styles modernes */
        "button { "
        "  background: #7aa2f7; "
        "  color: #1a1b26; "
        "  border-radius: 8px; "
        "  border: none; "
        "  padding: 10px 20px; "
        "  margin: 5px; "
        "  font-weight: 600; "
        "  font-size: 14px; "
        "  transition: all 0.3s ease; "
        "  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); "
        "} "
        "button:hover { "
        "  background: #9aa5ce; "
        "  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2); "
        "} "
        "button:active { "
        "  box-shadow: 0 1px 2px rgba(0, 0, 0, 0.1); "
        "} "

        /* Labels avec meilleure typographie */
        "label { "
        "  color: #c0caf5; "
        "  font-weight: 500; "
        "} "
        ".status-connected { "
        "  color: #9ece6a; "
        "  font-weight: 700; "
        "  text-shadow: 0 0 5px rgba(158, 206, 106, 0.3); "
        "} "
        ".status-disconnected { "
        "  color: #f7768e; "
        "  font-weight: 700; "
        "  text-shadow: 0 0 5px rgba(247, 118, 142, 0.3); "
        "} "
        ".status-connecting { "
        "  color: #ff9e64; "
        "  font-weight: 700; "
        "  text-shadow: 0 0 5px rgba(255, 158, 100, 0.3); "
        "} "

        /* Zone de messages améliorée */
        "#message_view { "
        "  background-color: #24283b; "
        "  color: #c0caf5; "
        "  border-radius: 12px; "
        "  border: 1px solid #414868; "
        "  font-family: 'Source Code Pro', 'Consolas', monospace; "
        "  font-size: 16px; "
        "} "

        /* Zone de messages avec style de chat moderne */
        "textview text { "
        "  background-color: #363a4f; "
        "  color: #c0caf5; "
        "} "

        /* Séparateur moderne */
        "separator { "
        "  background: linear-gradient(90deg, transparent 0%, #414868 50%, "
        "transparent 100%); "
        "  margin: 15px 0; "
        "  min-height: 1px; "
        "} "

        /* ScrolledWindow avec scrollbar personnalisée */
        "scrolledwindow { "
        "  border: none; "
        "  background-color: #24283b; "
        "  border-radius: 12px; "
        "  border: 1px solid #414868; "
        "} "
        "scrollbar { "
        "  background-color: transparent; "
        "  border-radius: 8px; "
        "  min-width: 8px; "
        "} "
        "scrollbar slider { "
        "  background-color: #7aa2f7; "
        "  border-radius: 8px; "
        "  min-width: 8px; "
        "  min-height: 30px; "
        "  transition: all 0.3s ease; "
        "} "
        "scrollbar slider:hover { "
        "  background-color: #bb9af7; "
        "} "
        "scrollbar slider:active { "
        "  background-color: #9aa5ce; "
        "} "

        /* Frame avec bordures subtiles */
        "frame { "
        "  background-color: rgba(36, 40, 59, 0.5); "
        "  border-radius: 12px; "
        "  border: 1px solid rgba(65, 72, 104, 0.3); "
        "  padding: 10px; "
        "} ";

    gtk_css_provider_load_from_data(app->provider, css, -1, NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(app->provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

/**
 * @brief Crée tous les widgets de l'interface
 */
void create_widgets(FreescordApp *app) {
    // Création du layout principal
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 20);
    gtk_container_add(GTK_CONTAINER(app->window), main_box);

    // Zone de connexion avec frame moderne
    GtkWidget *connect_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(connect_frame), GTK_SHADOW_NONE);
    gtk_widget_set_margin_bottom(connect_frame, 15);
    gtk_box_pack_start(GTK_BOX(main_box), connect_frame, FALSE, FALSE, 0);

    GtkWidget *connect_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(connect_box), 15);
    gtk_container_add(GTK_CONTAINER(connect_frame), connect_box);

    // Champs de connexion avec labels améliorés
    GtkWidget *username_label = gtk_label_new("Pseudo:");
    gtk_widget_set_size_request(username_label, 70, -1);
    gtk_widget_set_halign(username_label, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(connect_box), username_label, FALSE, FALSE, 5);

    app->username_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(app->username_entry),
                             MAX_USERNAME_LENGTH - 1);
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->username_entry),
                                   "Votre pseudo");
    gtk_widget_set_size_request(app->username_entry, 150, -1);
    gtk_box_pack_start(GTK_BOX(connect_box), app->username_entry, FALSE, TRUE,
                       0);

    GtkWidget *server_label = gtk_label_new("Serveur:");
    gtk_widget_set_size_request(server_label, 70, -1);
    gtk_widget_set_halign(server_label, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(connect_box), server_label, FALSE, FALSE, 5);

    app->server_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app->server_entry), DEFAULT_SERVER);
    gtk_widget_set_size_request(app->server_entry, 120, -1);
    gtk_box_pack_start(GTK_BOX(connect_box), app->server_entry, FALSE, TRUE, 0);

    GtkWidget *port_label = gtk_label_new("Port:");
    gtk_widget_set_size_request(port_label, 50, -1);
    gtk_widget_set_halign(port_label, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(connect_box), port_label, FALSE, FALSE, 5);

    app->port_entry = gtk_entry_new();
    char port_str[10];
    snprintf(port_str, sizeof(port_str), "%d", DEFAULT_PORT);
    gtk_entry_set_text(GTK_ENTRY(app->port_entry), port_str);
    gtk_widget_set_size_request(app->port_entry, 80, -1);
    gtk_box_pack_start(GTK_BOX(connect_box), app->port_entry, FALSE, FALSE, 0);

    // Bouton de connexion
    app->connect_button = gtk_button_new_with_label("Connexion");
    gtk_widget_set_size_request(app->connect_button, 120, -1);
    gtk_box_pack_start(GTK_BOX(connect_box), app->connect_button, FALSE, FALSE,
                       10);

    // Label de statut avec style amélioré
    app->status_label = gtk_label_new("Non connecté");
    gtk_widget_set_halign(app->status_label, GTK_ALIGN_END);
    gtk_widget_set_margin_end(app->status_label, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(app->status_label),
                                "status-disconnected");
    gtk_box_pack_start(GTK_BOX(main_box), app->status_label, FALSE, FALSE, 5);

    // Séparateur
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_box), separator, FALSE, FALSE, 0);

    // Zone de chat avec frame moderne
    GtkWidget *chat_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(chat_frame), GTK_SHADOW_NONE);
    gtk_widget_set_vexpand(chat_frame, TRUE);
    gtk_widget_set_margin_bottom(chat_frame, 10);
    gtk_box_pack_start(GTK_BOX(main_box), chat_frame, TRUE, TRUE, 0);
    gtk_widget_set_size_request(chat_frame, -1, 500);

    app->scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(app->scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(
        GTK_SCROLLED_WINDOW(app->scrolled_window), GTK_SHADOW_NONE);
    gtk_container_add(GTK_CONTAINER(chat_frame), app->scrolled_window);

    app->message_view = gtk_text_view_new();
    gtk_widget_set_name(app->message_view, "message_view");

    app->message_buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->message_view));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->message_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(app->message_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->message_view),
                                GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(app->message_view), 6);
    gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(app->message_view), 6);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(app->message_view), 15);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(app->message_view), 15);

    gtk_container_add(GTK_CONTAINER(app->scrolled_window), app->message_view);

    // Création des tags pour le formatage des messages avec couleurs améliorées
    gtk_text_buffer_create_tag(app->message_buffer, "timestamp", "foreground",
                               "#7aa2f7", "weight", PANGO_WEIGHT_BOLD, "size",
                               14 * PANGO_SCALE, NULL);

    gtk_text_buffer_create_tag(app->message_buffer, "you", "foreground",
                               "#9ece6a", "weight", PANGO_WEIGHT_BOLD, "size",
                               16 * PANGO_SCALE, NULL);

    gtk_text_buffer_create_tag(app->message_buffer, "system", "foreground",
                               "#e0af68", "style", PANGO_STYLE_ITALIC, "weight",
                               PANGO_WEIGHT_BOLD, NULL);

    // Créer des tags pour chaque couleur de pseudo
    for (int i = 0; i < num_user_colors; i++) {
        char tag_name[32];
        snprintf(tag_name, sizeof(tag_name), "user_%d", i);
        gtk_text_buffer_create_tag(app->message_buffer, tag_name, "foreground",
                                   user_colors[i], "weight", PANGO_WEIGHT_BOLD,
                                   "size", 16 * PANGO_SCALE, NULL);
    }

    // Zone de saisie avec frame moderne
    GtkWidget *input_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(input_frame), GTK_SHADOW_NONE);
    gtk_widget_set_margin_top(input_frame, 5);
    gtk_box_pack_start(GTK_BOX(main_box), input_frame, FALSE, FALSE, 0);

    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(input_box), 10);
    gtk_container_add(GTK_CONTAINER(input_frame), input_box);

    app->entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->entry),
                                   "Tapez votre message...");
    gtk_widget_set_sensitive(app->entry, FALSE);
    gtk_widget_set_hexpand(app->entry, TRUE);
    gtk_box_pack_start(GTK_BOX(input_box), app->entry, TRUE, TRUE, 0);

    app->send_button = gtk_button_new_with_label("Envoyer");
    gtk_widget_set_sensitive(app->send_button, FALSE);
    gtk_widget_set_size_request(app->send_button, 100, -1);
    gtk_box_pack_start(GTK_BOX(input_box), app->send_button, FALSE, FALSE, 0);
}

/**
 * @brief Connecte les signaux aux callbacks
 */
void connect_signals(FreescordApp *app) {
    g_signal_connect(app->send_button, "clicked",
                     G_CALLBACK(on_send_button_clicked), app);
    g_signal_connect(app->entry, "activate", G_CALLBACK(on_entry_activate),
                     app);
    g_signal_connect(app->connect_button, "clicked",
                     G_CALLBACK(on_connect_button_clicked), app);
}

/**
 * @brief Obtient la couleur d'un utilisateur basée sur son pseudo
 */
const char *get_user_color_tag(const char *username) {
    if (!username) return "user_0";

    // Fonction de hachage simple pour assigner une couleur
    unsigned int hash = 0;
    for (int i = 0; username[i]; i++) {
        hash = hash * 31 + username[i];
    }

    char *tag_name = malloc(32);
    snprintf(tag_name, 32, "user_%d", hash % num_user_colors);
    return tag_name;
}

/**
 * @brief Gère l'événement de clic sur le bouton d'envoi
 */
void on_send_button_clicked(GtkButton *button, FreescordApp *app) {
    const char *text = gtk_entry_get_text(GTK_ENTRY(app->entry));
    if (strlen(text) == 0) return;

    // Construire le timestamp [HH:MM]
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    char ts[6];
    strftime(ts, sizeof(ts), "%H:%M", lt);

    // Afficher localement (vous)
    format_message_display(app, ts, "Vous", text, TRUE);

    // Envoyer au serveur
    send_message(app, text);

    // Remettre le champ à blanc
    gtk_entry_set_text(GTK_ENTRY(app->entry), "");
}

/**
 * @brief Formatte l'affichage d'un message avec timestamp et pseudo coloré
 */
void format_message_display(FreescordApp *app, const char *timestamp,
                            const char *username, const char *message,
                            gboolean is_me) {
    if (!app || !app->message_buffer) return;

    pthread_mutex_lock(&app->mutex);

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);

    // 1. Insérer le timestamp entre crochets
    gtk_text_buffer_insert(app->message_buffer, &end, "[", -1);
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    gtk_text_buffer_insert_with_tags_by_name(app->message_buffer, &end,
                                             timestamp, -1, "timestamp", NULL);
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    gtk_text_buffer_insert(app->message_buffer, &end, "] ", -1);

    // 2. Insérer le nom d'utilisateur avec couleur appropriée
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    if (is_me) {
        gtk_text_buffer_insert_with_tags_by_name(app->message_buffer, &end,
                                                 username, -1, "you", NULL);
    } else {
        const char *color_tag = get_user_color_tag(username);
        gtk_text_buffer_insert_with_tags_by_name(app->message_buffer, &end,
                                                 username, -1, color_tag, NULL);
        free((void *)color_tag);
    }

    // 3. Insérer le message
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    gtk_text_buffer_insert(app->message_buffer, &end, ": ", -1);
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    gtk_text_buffer_insert(app->message_buffer, &end, message, -1);
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    gtk_text_buffer_insert(app->message_buffer, &end, "\n", -1);

    // Défilement automatique vers le bas
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    GtkTextMark *mark =
        gtk_text_buffer_create_mark(app->message_buffer, NULL, &end, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(app->message_view), mark, 0.0,
                                 TRUE, 0.0, 1.0);
    gtk_text_buffer_delete_mark(app->message_buffer, mark);

    pthread_mutex_unlock(&app->mutex);
}

/**
 * @brief Gère la fermeture de la fenêtre
 */
void on_window_destroy(GtkWidget *widget, FreescordApp *app) {
    if (app && app->connected) {
        disconnect_from_server(app);
    }

    if (app && app->provider) {
        g_object_unref(app->provider);
        app->provider = NULL;
    }

    gtk_main_quit();
}

/**
 * @brief Se connecte au serveur
 * @return Le descripteur de socket ou -1 en cas d'erreur
 */
int connect_to_server(const char *server, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;

    // Création de la socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    // Configuration de l'adresse du serveur
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return -1;
    }

    // Connexion au serveur
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

/**
 * @brief Envoie un message au serveur
 */
void send_message(FreescordApp *app, const char *message) {
    if (!app->connected) return;

    // Envoyer le message sans newline - le serveur gère ça
    send(app->socket_fd, message, strlen(message), 0);
}

/**
 * @brief Thread de réception des messages
 */
void *receive_messages(void *data) {
    FreescordApp *app = (FreescordApp *)data;
    char buffer[BUFFER_SIZE];
    int received;

    // Gestion de la séquence de bienvenue d'abord
    if (handle_welcome_sequence(app) < 0) {
        gdk_threads_add_idle((GSourceFunc)on_connection_error, app);
        return NULL;
    }

    while (app->thread_running) {
        received = recv(app->socket_fd, buffer, BUFFER_SIZE - 1, 0);

        if (received <= 0) {
            if (app->thread_running) {
                // Erreur ou déconnexion
                gdk_threads_add_idle((GSourceFunc)gtk_button_clicked,
                                     app->connect_button);
            }
            break;
        }

        buffer[received] = '\0';

        // Analyser et formater le message reçu
        process_incoming_message(app, buffer);
    }

    return NULL;
}

/**
 * @brief Gère la séquence de bienvenue du serveur
 */
static int handle_welcome_sequence(FreescordApp *app) {
    char buffer[BUFFER_SIZE];
    int received, status;

    // Recevoir le message de bienvenue
    received = recv(app->socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) return -1;

    buffer[received] = '\0';

    // Afficher le message de bienvenue
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    char ts[6];
    strftime(ts, sizeof(ts), "%H:%M", lt);

    MessageData *welcome_data =
        create_message_data(app, ts, NULL, buffer, FALSE);
    gdk_threads_add_idle((GSourceFunc)format_system_message_idle, welcome_data);

    do {
        // Recevoir le prompt pour le pseudo
        received = recv(app->socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) return -1;
        buffer[received] = '\0';

        // Envoyer le pseudo
        send(app->socket_fd, app->username, strlen(app->username), 0);

        // Recevoir la réponse
        received = recv(app->socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) return -1;
        buffer[received] = '\0';

        status = buffer[0] - '0';

        if (status != 0) {
            // Afficher l'erreur (extraire le message après "X | ")
            char *error_msg = strchr(buffer, '|');
            if (error_msg) {
                error_msg += 2;  // Sauter "| "
                MessageData *error_data =
                    create_message_data(app, ts, NULL, error_msg, FALSE);
                gdk_threads_add_idle((GSourceFunc)format_system_message_idle,
                                     error_data);
            }
        }
    } while (status != 0);

    return 0;
}

/**
 * @brief Crée une structure MessageData
 */
static MessageData *create_message_data(FreescordApp *app,
                                        const char *timestamp,
                                        const char *username,
                                        const char *message, gboolean is_me) {
    MessageData *data = malloc(sizeof(MessageData));
    if (!data) return NULL;

    data->app = app;
    data->timestamp = timestamp ? strdup(timestamp) : NULL;
    data->username = username ? strdup(username) : NULL;
    data->message = message ? strdup(message) : NULL;
    data->is_me = is_me;

    return data;
}

/**
 * @brief Traite un message entrant et l'affiche correctement formaté
 */
void process_incoming_message(FreescordApp *app, const char *buffer) {
    if (!app || !buffer) return;

    // Créer une copie du message pour le traitement
    char *message_copy = strdup(buffer);
    if (!message_copy) return;

    // Supprimer les retours à la ligne de fin
    char *newline = strchr(message_copy, '\n');
    if (newline) *newline = '\0';

    // Créer le timestamp
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    char ts[6];
    strftime(ts, sizeof(ts), "%H:%M", lt);

    // Analyser le format du message
    // Format attendu: "Username: message"
    char *colon_pos = strchr(message_copy, ':');
    if (colon_pos) {
        *colon_pos = '\0';  // Séparer le username
        char *username = message_copy;
        char *message_content = colon_pos + 1;

        // Supprimer les espaces au début du message
        while (*message_content == ' ') message_content++;

        // Vérifier si c'est notre propre message (normalement ne devrait pas
        // arriver)
        gboolean is_me = (strcmp(username, app->username) == 0);

        // Afficher le message formaté
        MessageData *data =
            create_message_data(app, ts, username, message_content, is_me);
        gdk_threads_add_idle(format_received_message_idle, data);
    } else {
        // Message système
        MessageData *data =
            create_message_data(app, ts, NULL, message_copy, FALSE);
        gdk_threads_add_idle(format_system_message_idle, data);
    }

    free(message_copy);
}

/**
 * @brief Fonction d'affichage de message reçu appelée dans le thread principal
 */
static gboolean format_received_message_idle(gpointer data) {
    if (!data) return G_SOURCE_REMOVE;

    MessageData *msg_data = (MessageData *)data;
    format_message_display(msg_data->app, msg_data->timestamp,
                           msg_data->username, msg_data->message,
                           msg_data->is_me);

    free_message_data(msg_data);
    return G_SOURCE_REMOVE;
}

/**
 * @brief Fonction d'affichage de message système appelée dans le thread
 * principal
 */
static gboolean format_system_message_idle(gpointer data) {
    if (!data) return G_SOURCE_REMOVE;

    MessageData *msg_data = (MessageData *)data;
    format_system_message(msg_data->app, msg_data->timestamp,
                          msg_data->message);

    free_message_data(msg_data);
    return G_SOURCE_REMOVE;
}

/**
 * @brief Fonction appelée en cas d'erreur de connexion
 */
static gboolean on_connection_error(gpointer data) {
    FreescordApp *app = (FreescordApp *)data;

    // Forcer la déconnexion
    gtk_button_clicked(GTK_BUTTON(app->connect_button));

    // Afficher l'erreur
    set_status(app, "Erreur de connexion", "status-disconnected");

    return G_SOURCE_REMOVE;
}

/**
 * @brief Libère les ressources d'un message
 */
static void free_message_data(MessageData *data) {
    if (!data) return;

    if (data->timestamp) free(data->timestamp);
    if (data->username) free(data->username);
    if (data->message) free(data->message);
    free(data);
}

/**
 * @brief Déconnexion du serveur
 */
void disconnect_from_server(FreescordApp *app) {
    if (!app) return;

    // Arrêter le thread de réception
    app->thread_running = 0;

    // Fermer la socket
    if (app->socket_fd >= 0) {
        shutdown(app->socket_fd, SHUT_RDWR);
        close(app->socket_fd);
        app->socket_fd = -1;
    }

    // Attendre la fin du thread
    if (app->connected) {
        pthread_join(app->receive_thread, NULL);
    }

    app->connected = 0;
}

/**
 * @brief Définit le statut de connexion
 */
void set_status(FreescordApp *app, const char *status,
                const char *style_class) {
    if (!app || !app->status_label) return;

    // Supprimer toutes les classes existantes
    GtkStyleContext *context = gtk_widget_get_style_context(app->status_label);
    gtk_style_context_remove_class(context, "status-connected");
    gtk_style_context_remove_class(context, "status-disconnected");
    gtk_style_context_remove_class(context, "status-connecting");

    // Ajouter la nouvelle classe
    if (style_class) {
        gtk_style_context_add_class(context, style_class);
    }

    // Définir le texte
    gtk_label_set_text(GTK_LABEL(app->status_label), status);
}

/**
 * @brief Efface la zone de messages
 */
void clear_messages(FreescordApp *app) {
    if (!app || !app->message_buffer) return;

    pthread_mutex_lock(&app->mutex);
    gtk_text_buffer_set_text(app->message_buffer, "", -1);
    pthread_mutex_unlock(&app->mutex);
}

/**
 * @brief Ajoute un message à la zone de messages (fonction d'idle)
 */
static gboolean append_message_idle(gpointer data) {
    if (!data) return G_SOURCE_REMOVE;

    MessageData *msg_data = (MessageData *)data;
    FreescordApp *app = msg_data->app;

    if (!app || !app->message_buffer) {
        free_message_data(msg_data);
        return G_SOURCE_REMOVE;
    }

    pthread_mutex_lock(&app->mutex);

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    gtk_text_buffer_insert(app->message_buffer, &end, msg_data->message, -1);
    gtk_text_buffer_insert(app->message_buffer, &end, "\n", -1);

    // Défilement automatique vers le bas
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    GtkTextMark *mark =
        gtk_text_buffer_create_mark(app->message_buffer, NULL, &end, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(app->message_view), mark, 0.0,
                                 TRUE, 0.0, 1.0);
    gtk_text_buffer_delete_mark(app->message_buffer, mark);

    pthread_mutex_unlock(&app->mutex);

    free_message_data(msg_data);
    return G_SOURCE_REMOVE;
}

/**
 * @brief Fonction utilitaire pour ajouter un simple message à la zone de
 * messages
 */
void append_raw_message(FreescordApp *app, const char *message) {
    if (!app || !message) return;

    MessageData *data = create_message_data(app, NULL, NULL, message, FALSE);
    if (data) {
        gdk_threads_add_idle(append_message_idle, data);
    }
}

/**
 * @brief Affiche un dialogue d'erreur
 */
void show_error_dialog(GtkWindow *parent, const char *title,
                       const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(
        parent, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK, "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/**
 * @brief Gère l'événement d'activation de la zone de saisie (touche Entrée)
 */
void on_entry_activate(GtkEntry *entry, FreescordApp *app) {
    on_send_button_clicked(GTK_BUTTON(app->send_button), app);
}

/**
 * @brief Gère l'événement de clic sur le bouton de connexion
 */
void on_connect_button_clicked(GtkButton *button, FreescordApp *app) {
    if (!app) return;

    if (!app->connected) {
        // Récupération des informations de connexion
        const char *username =
            gtk_entry_get_text(GTK_ENTRY(app->username_entry));
        const char *server = gtk_entry_get_text(GTK_ENTRY(app->server_entry));
        const char *port_str = gtk_entry_get_text(GTK_ENTRY(app->port_entry));
        int port = atoi(port_str);

        if (strlen(username) == 0) {
            set_status(app, "Erreur: Pseudo requis", "status-disconnected");
            return;
        }

        // Sauvegarde du nom d'utilisateur
        strncpy(app->username, username, MAX_USERNAME_LENGTH - 1);
        app->username[MAX_USERNAME_LENGTH - 1] = '\0';

        // Connexion au serveur
        set_status(app, "Connexion en cours...", "status-connecting");
        app->socket_fd = connect_to_server(server, port);

        if (app->socket_fd < 0) {
            set_status(app, "Erreur de connexion", "status-disconnected");
            return;
        }

        // Démarrer le thread de réception
        app->connected = 1;
        app->thread_running = 1;
        if (pthread_create(&app->receive_thread, NULL, receive_messages, app) !=
            0) {
            app->connected = 0;
            app->thread_running = 0;
            close(app->socket_fd);
            app->socket_fd = -1;
            set_status(app, "Erreur de création du thread",
                       "status-disconnected");
            return;
        }

        // Mise à jour de l'interface
        gtk_button_set_label(GTK_BUTTON(app->connect_button), "Déconnexion");
        gtk_widget_set_sensitive(app->username_entry, FALSE);
        gtk_widget_set_sensitive(app->server_entry, FALSE);
        gtk_widget_set_sensitive(app->port_entry, FALSE);
        gtk_widget_set_sensitive(app->entry, TRUE);
        gtk_widget_set_sensitive(app->send_button, TRUE);
        gtk_widget_grab_focus(app->entry);

        set_status(app, "Connecté", "status-connected");
        clear_messages(app);

        // Message de bienvenue formaté
        time_t now = time(NULL);
        struct tm *lt = localtime(&now);
        char ts[6];
        strftime(ts, sizeof(ts), "%H:%M", lt);

        format_system_message(app, ts, "Connecté au serveur Freescord");

    } else {
        // Déconnexion
        disconnect_from_server(app);

        // Mise à jour de l'interface
        gtk_button_set_label(GTK_BUTTON(app->connect_button), "Connexion");
        gtk_widget_set_sensitive(app->username_entry, TRUE);
        gtk_widget_set_sensitive(app->server_entry, TRUE);
        gtk_widget_set_sensitive(app->port_entry, TRUE);
        gtk_widget_set_sensitive(app->entry, FALSE);
        gtk_widget_set_sensitive(app->send_button, FALSE);

        set_status(app, "Non connecté", "status-disconnected");

        // Message de déconnexion formaté
        time_t now = time(NULL);
        struct tm *lt = localtime(&now);
        char ts[6];
        strftime(ts, sizeof(ts), "%H:%M", lt);

        format_system_message(app, ts, "Déconnecté du serveur");
    }
}

/**
 * @brief Formatte et affiche un message système
 */
void format_system_message(FreescordApp *app, const char *timestamp,
                           const char *message) {
    if (!app || !app->message_buffer) return;

    pthread_mutex_lock(&app->mutex);

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);

    // Formater et afficher le message système
    gtk_text_buffer_insert(app->message_buffer, &end, "[", -1);
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    gtk_text_buffer_insert_with_tags_by_name(app->message_buffer, &end,
                                             timestamp, -1, "timestamp", NULL);
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    gtk_text_buffer_insert(app->message_buffer, &end, "] ", -1);
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    gtk_text_buffer_insert_with_tags_by_name(app->message_buffer, &end, message,
                                             -1, "system", NULL);
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    gtk_text_buffer_insert(app->message_buffer, &end, "\n", -1);

    // Défilement automatique vers le bas
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    GtkTextMark *mark =
        gtk_text_buffer_create_mark(app->message_buffer, NULL, &end, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(app->message_view), mark, 0.0,
                                 TRUE, 0.0, 1.0);
    gtk_text_buffer_delete_mark(app->message_buffer, mark);

    pthread_mutex_unlock(&app->mutex);
}