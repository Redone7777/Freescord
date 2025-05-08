# Freescord

Freescord est une application de messagerie client-serveur avec une architecture mixte C/Java. Le projet contient deux clients : un client Java avec interface graphique JavaFX et un client en ligne de commande en C, ainsi qu'un serveur en C.

## Structure du projet

```
.
├── bin/                    # Dossier des exécutables C (généré)
├── build/                  # Dossier des objets compilés (généré)
├── include/                # En-têtes C et bibliothèques
│   ├── buffer/             # Gestion des buffers
│   │   ├── buffer.c
│   │   └── buffer.h
│   ├── client.h            # En-tête du client C
│   ├── list/               # Gestion des listes chaînées
│   │   ├── list.c
│   │   ├── list.h
│   │   └── test_list.c
│   ├── serveur.h           # En-tête du serveur C
│   ├── user.h              # Gestion des utilisateurs
│   └── utils.h             # Fonctions utilitaires
├── src/                    # Code source C
│   ├── client.c            # Client en ligne de commande C
│   ├── serveur.c           # Serveur C
│   ├── user.c              # Implémentation des utilisateurs
│   └── utils.c             # Fonctions utilitaires
├── FreescordJavaFXClient.java  # Client graphique Java
├── FreescordJNI.c              # Implémentation JNI (Java Native Interface)
├── FreescordNative.h           # En-tête JNI généré
├── FreescordNative.java        # Interface Java vers le code natif C
├── libfreescord_jni.so         # Bibliothèque native JNI compilée (générée)
├── Makefile                    # Fichier de compilation
└── README.md                   # Ce fichier
```

## Prérequis

- JDK 11 ou supérieur
- JavaFX SDK 11 ou supérieur
- GCC
- Make
- Bibliothèques de développement pour les sockets et pthreads

## Installation de JavaFX

1. Téléchargez JavaFX SDK depuis [https://openjfx.io/](https://openjfx.io/)
2. Décompressez l'archive dans un dossier de votre choix
3. Mettez à jour la variable `JAVAFX_HOME` dans le Makefile pour pointer vers votre dossier JavaFX SDK

## Compilation

Pour compiler tout le projet (client Java, serveur C et client C) :

```bash
make all
```

Pour compiler uniquement le client Java avec JNI :

```bash
make java-client
```

Pour compiler uniquement les applications C (serveur et client) :

```bash
make c-apps
```

## Exécution

### Serveur C

```bash
make run-srv
# ou
./bin/srv [port]  # port optionnel, par défaut 6667
```

### Client Java avec interface graphique

```bash
make run-java
# ou
java --module-path /chemin/vers/javafx-sdk/lib --add-modules javafx.controls -Djava.library.path=. FreescordJavaFXClient
```

### Client C en ligne de commande

```bash
make run-clt
# ou
./bin/clt [host] [port]  # host et port optionnels, par défaut 127.0.0.1:6667
```

## Fonctionnalités

### Serveur C

- Acceptation des connexions clients
- Gestion multi-client avec threads
- Rediffusion des messages à tous les clients connectés
- Affichage du pseudo lors de la connexion
- Notification lors de la déconnexion d'un client

### Client Java (interface graphique)

- Interface construite avec JavaFX
- Connexion au serveur via JNI (Java Native Interface)
- Boîte de dialogue pour entrer son pseudo
- Affichage des messages dans une zone de texte défilante
- Envoi de messages via un champ de texte et un bouton
- Déconnexion propre avec la commande `/exit`

### Client C (ligne de commande)

- Interface en ligne de commande
- Connexion au serveur
- Réception et affichage des messages des autres utilisateurs
- Envoi de messages
- Déconnexion propre avec la commande `/exit`

## Fonctionnement technique

Le projet utilise une architecture client-serveur TCP/IP avec :

1. **Communication réseau** : Utilisation de sockets TCP pour la communication entre clients et serveur
2. **Multithreading** : Le serveur utilise des threads pour gérer plusieurs clients simultanément
3. **JNI (Java Native Interface)** : Permet au client Java d'utiliser le code C pour la communication réseau
4. **Buffers optimisés** : Gestion efficace des buffers d'entrée/sortie pour éviter la perte de données
5. **Conversion des fins de ligne** : Gestion automatique de la conversion CRLF/LF pour la compatibilité réseau

## Dépannage

### Erreurs JavaFX

Si vous rencontrez des erreurs de compilation liées à JavaFX :

1. Vérifiez que vous avez téléchargé et installé JavaFX SDK
2. Assurez-vous que la variable `JAVAFX_HOME` dans le Makefile pointe vers le bon dossier
3. Pour une exécution manuelle, utilisez les options `--module-path` et `--add-modules` avec Java

### Erreurs JNI

Si vous rencontrez des erreurs liées à JNI :

1. Vérifiez que la variable d'environnement `JAVA_HOME` est correctement configurée
2. Assurez-vous que la bibliothèque `libfreescord_jni.so` est bien générée
3. Vérifiez que l'option `-Djava.library.path=.` est utilisée lors de l'exécution de Java

## Licence

Ce projet est fourni à titre éducatif.