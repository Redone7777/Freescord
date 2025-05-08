/**
 * @file freescord_gui.c
 * @brief Implémentation de l'interface graphique GTK3 pour le client Freescord
 */

#include "freescord_gui.h"

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
 * @brief Configure le style CSS
 */
void setup_css(FreescordApp *app) {
    app->provider = gtk_css_provider_new();

    const char *css =
        /* Thème global plus moderne et sombre */
        "window { background-color: #1a1b26; } "

        /* Formulaire de connexion */
        "entry { background-color: #24283b; color: #c0caf5; border-radius: "
        "6px; border: 1px solid #414868; padding: 8px; margin: 5px; } "

        /* Boutons */
        "button { background-color: #7aa2f7; color: #1a1b26; border-radius: "
        "6px; border: none; padding: 8px 15px; margin: 5px; font-weight: bold; "
        "transition: all 200ms ease; } "
        "button:hover { background-color: #9aa5ce; color: #1a1b26; } "
        "button:active { background-color: #bb9af7; } "

        "GtkTextView.message-view text {background-color: #2a2d3e; color: "
        "#c0caf5;} "
        /* Labels */
        "label { color: #c0caf5; } "
        ".status-connected { color: #9ece6a; font-weight: bold; } "
        ".status-disconnected { color: #f7768e; font-weight: bold; } "
        ".status-connecting { color: #ff9e64; font-weight: bold; } "

        /* Zone de texte */
        "GtkTextView.message-view text {background-color: #2a2d3e; color: "
        "#c0caf5;} "

        /* Séparateur */
        "separator { background-color: #414868; margin: 10px 0; } "

        /* ScrolledWindow */
        "scrolledwindow { border: none; background-color: #24283b; "
        "border-radius: 8px; } "
        "scrollbar { background-color: #24283b; border-radius: 0; } "
        "scrollbar slider { background-color: #7aa2f7; border-radius: 6px; "
        "min-width: 12px; } "
        "scrollbar slider:hover { background-color: #bb9af7; } "
        "scrollbar slider:active { background-color: #9aa5ce; } "

        /* Zone de messages */
        "#message_view {"
        "  background-color: #1e1e1e;"
        "  color: #c0caf5;"
        "  border-radius: 8px;"
        "}";

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
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 15);
    gtk_container_add(GTK_CONTAINER(app->window), main_box);

    // Création de la zone de connexion avec un arrière-plan
    GtkWidget *connect_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(connect_frame), GTK_SHADOW_NONE);
    gtk_widget_set_margin_bottom(connect_frame, 10);
    gtk_box_pack_start(GTK_BOX(main_box), connect_frame, FALSE, FALSE, 0);

    GtkWidget *connect_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(connect_box), 10);
    gtk_container_add(GTK_CONTAINER(connect_frame), connect_box);

    // Label et champ pour le nom d'utilisateur
    GtkWidget *username_label = gtk_label_new("Pseudo:");
    gtk_box_pack_start(GTK_BOX(connect_box), username_label, FALSE, FALSE, 5);

    app->username_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(app->username_entry),
                             MAX_USERNAME_LENGTH - 1);
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->username_entry),
                                   "Entrez votre pseudo");
    gtk_box_pack_start(GTK_BOX(connect_box), app->username_entry, TRUE, TRUE,
                       0);

    // Label et champ pour le serveur
    GtkWidget *server_label = gtk_label_new("Serveur:");
    gtk_box_pack_start(GTK_BOX(connect_box), server_label, FALSE, FALSE, 5);

    app->server_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app->server_entry), DEFAULT_SERVER);
    gtk_box_pack_start(GTK_BOX(connect_box), app->server_entry, TRUE, TRUE, 0);

    // Label et champ pour le port
    GtkWidget *port_label = gtk_label_new("Port:");
    gtk_box_pack_start(GTK_BOX(connect_box), port_label, FALSE, FALSE, 5);

    app->port_entry = gtk_entry_new();
    char port_str[10];
    snprintf(port_str, sizeof(port_str), "%d", DEFAULT_PORT);
    gtk_entry_set_text(GTK_ENTRY(app->port_entry), port_str);
    gtk_entry_set_width_chars(GTK_ENTRY(app->port_entry), 6);
    gtk_box_pack_start(GTK_BOX(connect_box), app->port_entry, FALSE, FALSE, 0);

    // Bouton de connexion
    app->connect_button = gtk_button_new_with_label("Connexion");
    gtk_box_pack_start(GTK_BOX(connect_box), app->connect_button, FALSE, FALSE,
                       0);

    // Label pour l'état de la connexion
    app->status_label = gtk_label_new("Non connecté");
    gtk_widget_set_halign(app->status_label, GTK_ALIGN_END);
    gtk_style_context_add_class(gtk_widget_get_style_context(app->status_label),
                                "status-disconnected");
    gtk_box_pack_start(GTK_BOX(main_box), app->status_label, FALSE, FALSE, 5);

    // Création du séparateur
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_box), separator, FALSE, FALSE, 0);

    // Création de la zone des messages
    GtkWidget *chat_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(chat_frame), GTK_SHADOW_NONE);
    gtk_widget_set_vexpand(chat_frame, TRUE);
    gtk_box_pack_start(GTK_BOX(main_box), chat_frame, TRUE, TRUE, 0);

    app->scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(app->scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(chat_frame), app->scrolled_window);

    app->message_view = gtk_text_view_new();
    gtk_widget_set_name(app->message_view, "message_view");

    app->message_buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->message_view));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->message_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(app->message_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->message_view),
                                GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(app->message_view), 4);
    gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(app->message_view), 4);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(app->message_view), 10);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(app->message_view), 10);

    // Appliquer la classe CSS
    gtk_style_context_add_class(gtk_widget_get_style_context(app->message_view),
                                "message-view");

    gtk_container_add(GTK_CONTAINER(app->scrolled_window), app->message_view);

    // Création des tags pour le formatage des messages
    gtk_text_buffer_create_tag(app->message_buffer, "bold", "weight",
                               PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag(app->message_buffer, "timestamp", "foreground",
                               "#7aa2f7", "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag(app->message_buffer, "username", "foreground",
                               "#bb9af7", "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag(app->message_buffer, "you", "foreground",
                               "#9ece6a", "weight", PANGO_WEIGHT_BOLD, NULL);

    // Création de la zone de saisie et d'envoi de messages
    GtkWidget *input_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(input_frame), GTK_SHADOW_NONE);
    gtk_widget_set_margin_top(input_frame, 10);
    gtk_box_pack_start(GTK_BOX(main_box), input_frame, FALSE, FALSE, 0);

    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(input_box), 5);
    gtk_container_add(GTK_CONTAINER(input_frame), input_box);

    app->entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->entry),
                                   "Saisissez votre message...");
    gtk_widget_set_sensitive(app->entry, FALSE);
    gtk_box_pack_start(GTK_BOX(input_box), app->entry, TRUE, TRUE, 0);

    app->send_button = gtk_button_new_with_label("Envoyer");
    gtk_widget_set_sensitive(app->send_button, FALSE);
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
 * @brief Gère l'événement de clic sur le bouton d'envoi
 */
void on_send_button_clicked(GtkButton *button, FreescordApp *app) {
    const char *text = gtk_entry_get_text(GTK_ENTRY(app->entry));
    if (strlen(text) == 0) return;

    // --- 1. Construire le timestamp [HH:MM]
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    char ts[6];
    strftime(ts, sizeof(ts), "%H:%M", lt);

    // --- 2. Afficher localement (vous)
    format_message_display(app, ts, "Vous", text, TRUE);

    // --- 3. Envoyer au serveur
    send_message(app, text);

    // --- 4. Remettre le champ à blanc
    gtk_entry_set_text(GTK_ENTRY(app->entry), "");
}

/**
 * @brief Formatte l'affichage d'un message avec timestamp et pseudo en gras
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

    // 2. Insérer le nom d'utilisateur en gras avec couleur différenciée
    gtk_text_buffer_get_end_iter(app->message_buffer, &end);
    if (is_me) {
        gtk_text_buffer_insert_with_tags_by_name(app->message_buffer, &end,
                                                 username, -1, "you", NULL);
    } else {
        gtk_text_buffer_insert_with_tags_by_name(
            app->message_buffer, &end, username, -1, "username", NULL);
    }

    // 3. Insérer le message et un saut de ligne
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

        // Envoi du nom d'utilisateur
        if (send(app->socket_fd, username, strlen(username), 0) < 0) {
            close(app->socket_fd);
            app->socket_fd = -1;
            set_status(app, "Erreur d'envoi du pseudo", "status-disconnected");
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

        format_system_message(app, ts, "Connecté au serveur");

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

    // Créer un tag pour le message système s'il n'existe pas
    GtkTextTag *tag = gtk_text_tag_table_lookup(
        gtk_text_buffer_get_tag_table(app->message_buffer), "system");

    if (!tag) {
        tag = gtk_text_buffer_create_tag(app->message_buffer, "system",
                                         "foreground", "#e0af68", "style",
                                         PANGO_STYLE_ITALIC, NULL);
    }

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

    char message_with_newline[BUFFER_SIZE];
    snprintf(message_with_newline, sizeof(message_with_newline), "%s\n",
             message);
    send(app->socket_fd, message_with_newline, strlen(message_with_newline), 0);
}

/**
 * @brief Thread de réception des messages
 */
void *receive_messages(void *data) {
    FreescordApp *app = (FreescordApp *)data;
    char buffer[BUFFER_SIZE];
    int received;

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
 * @brief Traite un message entrant et l'affiche correctement formaté
 */
void process_incoming_message(FreescordApp *app, const char *buffer) {
    if (!app || !buffer) return;

    // Créer une copie du message pour le traitement
    char *message_copy = strdup(buffer);
    if (!message_copy) return;

    // Structure pour stocker le message formatté pour idle
    MessageData *data = malloc(sizeof(MessageData));
    if (!data) {
        free(message_copy);
        return;
    }

    // Format attendu des messages "[HH:MM] Username: message"
    char timestamp[10] = "";
    char username[MAX_USERNAME_LENGTH] = "";
    char *message_content = NULL;

    // Essayer d'extraire le timestamp et le username
    // Format: "[HH:MM] Username: message"
    if (sscanf(message_copy, "[%9[^]]] %31[^:]:", timestamp, username) == 2) {
        // Trouver le contenu du message après le ":"
        char *content_start = strstr(message_copy, ":");
        if (content_start) {
            message_content = content_start + 1;  // Sauter le ':'

            // Utiliser la nouvelle fonction pour afficher le message formaté
            // thread-safe via gdk_threads_add_idle
            data->app = app;
            data->timestamp = strdup(timestamp);
            data->username = strdup(username);
            data->message = strdup(message_content);
            data->is_me = (strcmp(username, app->username) == 0);

            gdk_threads_add_idle(format_received_message_idle, data);
        }
    } else {
        // Si le format n'est pas reconnu, afficher tel quel comme message
        // système
        time_t now = time(NULL);
        struct tm *lt = localtime(&now);
        char ts[6];
        strftime(ts, sizeof(ts), "%H:%M", lt);

        data->app = app;
        data->timestamp = strdup(ts);
        data->message = strdup(message_copy);

        gdk_threads_add_idle(format_system_message_idle, data);
    }

    free(message_copy);
}

/**
 * @brief Fonction d'affichage de message reçu appelée dans le thread principal
 */
gboolean format_received_message_idle(gpointer data) {
    if (!data) return G_SOURCE_REMOVE;

    MessageData *msg_data = (MessageData *)data;
    format_message_display(msg_data->app, msg_data->timestamp,
                           msg_data->username, msg_data->message,
                           msg_data->is_me);

    // Libérer la mémoire
    free(msg_data->timestamp);
    free(msg_data->username);
    free(msg_data->message);
    free(msg_data);

    return G_SOURCE_REMOVE;
}

/**
 * @brief Fonction d'affichage de message système appelée dans le thread
 * principal
 */
gboolean format_system_message_idle(gpointer data) {
    if (!data) return G_SOURCE_REMOVE;

    MessageData *msg_data = (MessageData *)data;
    format_system_message(msg_data->app, msg_data->timestamp,
                          msg_data->message);

    // Libérer la mémoire
    free(msg_data->timestamp);
    free(msg_data->message);
    free(msg_data);

    return G_SOURCE_REMOVE;
}

/**
 * @brief Libère les ressources d'un message
 */
static void free_message_data(gpointer data) {
    if (!data) return;

    MessageData *msg_data = (MessageData *)data;
    if (msg_data->timestamp) free(msg_data->timestamp);
    if (msg_data->username) free(msg_data->username);
    if (msg_data->message) free(msg_data->message);
    free(msg_data);
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
 * @brief Ajoute un message à la zone de messages
 */
static gboolean append_message_idle(gpointer data) {
    if (!data) return G_SOURCE_REMOVE;

    MessageData *msg_data = (MessageData *)data;
    FreescordApp *app = msg_data->app;

    if (!app || !app->message_buffer) {
        free_message_data(data);
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

    free_message_data(data);
    return G_SOURCE_REMOVE;
}

/**
 * @brief Fonction utilitaire pour ajouter un simple message à la zone de
 * messages
 */
void append_raw_message(FreescordApp *app, const char *message) {
    if (!app || !message) return;

    MessageData *data = malloc(sizeof(MessageData));
    if (!data) return;

    data->app = app;
    data->timestamp = NULL;
    data->username = NULL;
    data->message = strdup(message);
    data->is_me = FALSE;

    gdk_threads_add_idle(append_message_idle, data);
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