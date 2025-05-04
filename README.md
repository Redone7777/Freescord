# Freescord

Petit projet de chat en ligne réalisé en C dans le cadre du cours de Systèmes et Réseaux (L2 - 2024/2025).



## Description

L'application `freescord` permet à plusieurs clients de se connecter à un serveur TCP et d'échanger des messages en temps réel via le terminal.

## Contenu

* `serveur.c` : code du serveur multithreadé.
* `client.c` : client simple avec envoi/réception de messages.
* `user.c` : gestion des connexions clients (struct user).
* `Makefile` : compilation automatique avec `make`.

## Compilation

```bash
make
```

## Utilisation

### Serveur

```bash
./serveur [port]
```

> Par défaut : `4321`

### Client

```bash
./client [adresse_ip] [port]
```

> Par défaut : `127.0.0.1` et `4321`

## Fonctionnalités principales

* Connexion TCP client/serveur
* Prise en charge de plusieurs clients (via `pthread`)
* Identification par pseudo
* Echo des messages reçus
* Diffusion des messages à tous les clients


## À faire / améliorations possibles

* Ajout du format CRLF (`\r\n`)
* Gestion bufferisée des entrées/sorties
* Envoi de fichiers ou commandes

---

## Auteur

**Redwan Khan**
**N° Étudiant : 12212148**
**Projet – Réseaux & Systèmes**
**L2 Informatique - Université Sorbonne Paris Nord**