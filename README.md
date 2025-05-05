# Freescord

Petit projet de chat en ligne réalisé en C dans le cadre du cours de Systèmes et Réseaux (L2 - 2024/2025).



## Description

L'application `freescord` permet à plusieurs clients de se connecter à un serveur TCP et d'échanger des messages en temps réel via le terminal.

## Contenu

Le projet est organisé comme suit :

```
├── include
│   ├── buffer
│   │   ├── buffer.c
│   │   └── buffer.h
│   ├── client.h
│   ├── list
│   │   ├── list.c
│   │   ├── list.h
│   │   └── test_list.c
│   ├── serveur.h
│   ├── user.h
│   └── utils.h
├── Makefile
├── projet-freescord.pdf
├── README.md
└── src
    ├── client.c
    ├── serveur.c
    ├── user.c
    └── utils.c
```

- **src/serveur.c** : code du serveur multithreadé.
- **src/client.c** : client simple avec envoi/réception de messages.
- **src/user.c** : gestion des connexions clients (struct user).
- **src/utils.c** : fonctions utilitaires diverses.
- **include/buffer/buffer.c** : gestion des buffers de messages.
- **include/list/list.c** : gestion des listes de clients connectés.
- **Makefile** : compilation automatique avec `make`.
- **projet-freescord.pdf** : rapport du projet.

## Compilation

```bash
make
```

## Utilisation


### Serveur


```bash
make serveur
```
ou 

```bash
./bin/srv [port]
```

> Par défaut : `4321`

### Client

```bash
make serveur
```
ou 

```bash
./bin/clt [adresse_ip] [port]
```

> Par défaut : `127.0.0.1` et `4321`


## À faire / améliorations possibles

* Envoi de fichiers ou commandes

---

## Auteur

**Redwan Khan**
**N° Étudiant : 12212148**
**Projet – Réseaux & Systèmes**
**L2 Informatique - Université Sorbonne Paris Nord**