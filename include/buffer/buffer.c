#include "buffer.h"

struct buffer {
    int fd;           // Descripteur de fichier
    char *buf;        // Buffer de lecture
    size_t buffsz;    // Taille du buffer
    size_t pos;       // Position actuelle dans le buffer
    size_t end;       // Nombre d'octets valides dans le buffer
    int eof;          // Indicateur de fin de fichier
    int ungetc_char;  // Caractère remis dans le buffer par ungetc
};

Buffer *buff_create(int fd, size_t buffsz) {
    Buffer *b = malloc(sizeof(Buffer));
    if (b == NULL) {
        return NULL;
    }

    b->buf = malloc(buffsz);
    if (b->buf == NULL) {
        free(b);
        return NULL;
    }

    b->fd = fd;
    b->buffsz = buffsz;
    b->pos = 0;
    b->end = 0;
    b->eof = 0;
    b->ungetc_char = EOF;

    return b;
}

void buff_free(Buffer *buff) {
    if (buff != NULL) {
        free(buff->buf);
        free(buff);
    }
}

int buff_fill(Buffer *b) {
    if (b->eof) {
        return EOF;
    }

    ssize_t n = read(b->fd, b->buf, b->buffsz);
    if (n < 0) {
        // Erreur de lecture
        return EOF;
    } else if (n == 0) {
        // Fin de fichier atteinte
        b->eof = 1;
        return EOF;
    }

    b->pos = 0;
    b->end = n;
    return 0;
}

int buff_getc(Buffer *b) {
    int c;

    // Si un caractère a été remis dans le buffer avec ungetc, le retourner
    if (b->ungetc_char != EOF) {
        c = b->ungetc_char;
        b->ungetc_char = EOF;
        return c;
    }

    // Si le buffer est vide ou tous les caractères ont été lus
    if (b->pos >= b->end) {
        if (buff_fill(b) == EOF) {
            return EOF;
        }
    }

    c = (unsigned char)b->buf[b->pos++];
    return c;
}

int buff_ungetc(Buffer *b, int c) {
    if (c == EOF) {
        return EOF;
    }

    b->ungetc_char = c;
    return c;
}

int buff_eof(const Buffer *buff) {
    return buff->eof && (buff->pos >= buff->end) && (buff->ungetc_char == EOF);
}

int buff_ready(const Buffer *buff) {
    return (buff->ungetc_char != EOF) || (buff->pos < buff->end);
}

char *buff_fgets(Buffer *b, char *dest, size_t size) {
    if (size == 0) {
        return NULL;
    }

    size_t i = 0;
    int c;

    while (i < size - 1) {
        c = buff_getc(b);

        if (c == EOF) {
            break;
        }

        dest[i++] = c;

        if (c == '\n') {
            break;
        }
    }

    if (i == 0) {
        // Aucun caractère lu avant EOF
        return NULL;
    }

    dest[i] = '\0';
    return dest;
}

char *buff_fgets_crlf(Buffer *b, char *dest, size_t size) {
    if (size == 0) {
        return NULL;
    }

    size_t i = 0;
    int c, prev_c = 0;

    while (i < size - 1) {
        c = buff_getc(b);

        if (c == EOF) {
            break;
        }

        dest[i++] = c;

        // Si on a trouvé la séquence \r\n
        if (prev_c == '\r' && c == '\n') {
            break;
        }

        prev_c = c;
    }

    if (i == 0) {
        // Aucun caractère lu avant EOF
        return NULL;
    }

    dest[i] = '\0';
    return dest;
}