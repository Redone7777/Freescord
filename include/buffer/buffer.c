#include "buffer.h"

struct buffer {
    int FD;
    char *memBuf;

    size_t bufSize;
    size_t readPos;
    size_t dataEnd;

    int eof;
    int saved; /* Caractère remis (ungetc) */
};

/* Création d'un nouveau buffer */
Buffer *buff_create(int fd, size_t buffsz) {
    Buffer *new = malloc(sizeof(Buffer));
    if (!new) return NULL;

    new->memBuf = malloc(buffsz);
    if (!new->memBuf) {
        free(new);
        return NULL;
    }

    new->FD = fd;
    new->bufSize = buffsz;
    new->readPos = 0;
    new->dataEnd = 0;
    new->eof = 0;
    new->saved = EOF;

    return new;
}

/* Libération mémoire */
void buff_free(Buffer *buf) {
    if (!buf) return;
    if (buf->memBuf) free(buf->memBuf);
    free(buf);
}

/* Remplir le buffer avec de nouvelles données */
int buff_fill(Buffer *buf) {
    if (buf->eof) return EOF;

    int bytesRead = read(buf->FD, buf->memBuf, buf->bufSize);

    if (bytesRead <= 0) {
        buf->eof = 1;
        return EOF;
    }

    buf->readPos = 0;
    buf->dataEnd = bytesRead;
    return 0;
}

/* Lire un caractère */
int buff_getc(Buffer *buf) {
    /* D'abord vérifier s'il y a un caractère mis de côté */
    if (buf->saved != EOF) {
        int tmp = buf->saved;
        buf->saved = EOF;
        return tmp;
    }

    if (buf->readPos >= buf->dataEnd) {
        int res = buff_fill(buf);
        if (res == EOF) return EOF;
    }

    return buf->memBuf[buf->readPos++];
}

/* Remettre un caractère dans le buffer */
int buff_ungetc(Buffer *buf, int c) {
    if (c == EOF) return EOF;
    buf->saved = c;
    return c;
}

/* Test pour EOF */
int buff_eof(const Buffer *buf) {
    return buf->eof && (buf->readPos >= buf->dataEnd) && (buf->saved == EOF);
}

/* Test si des données sont disponibles dans le buffer */
int buff_ready(const Buffer *buf) {
    return (buf->saved != EOF) || (buf->readPos < buf->dataEnd);
}

/* Lire une ligne terminée par LF */
char *buff_fgets(Buffer *buf, char *dest, size_t maxLen) {
    if (!maxLen) return NULL;

    int charCount = 0;
    int curChar;

    while (charCount < maxLen - 1) {
        curChar = buff_getc(buf);
        if (curChar == EOF) break;

        dest[charCount++] = curChar;
        if (curChar == '\n') break;
    }

    if (!charCount) return NULL;

    dest[charCount] = '\0';
    return dest;
}

/* Lire une ligne terminée par CRLF */
char *buff_fgets_crlf(Buffer *buf, char *dest, size_t maxLen) {
    if (!maxLen) return NULL;

    int charCount = 0;
    int curChar;
    int hasCR = 0;

    /* Lecture jusqu'à trouver CRLF ou atteindre la limite */
    while (charCount < maxLen - 1) {
        curChar = buff_getc(buf);
        if (curChar == EOF) break;

        dest[charCount++] = curChar;

        if (curChar == '\r')
            hasCR = 1;
        else if (curChar == '\n' && hasCR)
            break;
        else
            hasCR = 0;
    }

    if (!charCount) return NULL;

    dest[charCount] = '\0';
    return dest;
}