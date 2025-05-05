#include "utils.h"

char *crlf_to_lf(char *lineWithCrlf) {
    if (lineWithCrlf == NULL) {
        return NULL;
    }

    int lineLength = strlen(lineWithCrlf);

    // Vérifier si la chaîne est vide
    if (lineLength == 0) return lineWithCrlf;

    // Rechercher les séquences '\r\n' et les remplacer par '\n'
    int i = 0;
    int j = 0;

    while (i < lineLength) {
        // Si on trouve '\r' suivi de '\n'
        if (lineWithCrlf[i] == '\r' && i + 1 < lineLength &&
            lineWithCrlf[i + 1] == '\n') {
            lineWithCrlf[j] = '\n';
            i += 2;  // Sauter les deux caractères '\r\n'
            j++;
        } else {
            lineWithCrlf[j] = lineWithCrlf[i];
            i++;
            j++;
        }
    }

    lineWithCrlf[j] = '\0';

    return lineWithCrlf;
}

char *lf_to_crlf(char *lineWithLf) {
    if (lineWithLf == NULL) {
        return NULL;
    }

    int lineLength = strlen(lineWithLf);
    int crlfCount = 0;

    // Compter le nombre de '\n' qui ne sont pas précédés par '\r'
    for (int i = 0; i < lineLength; i++)
        if (lineWithLf[i] == '\n' && (i == 0 || lineWithLf[i - 1] != '\r'))
            crlfCount++;

    // Si aucun remplacement n'est nécessaire, retourner la chaîne originale
    if (crlfCount == 0) return lineWithLf;

    int oldPos = lineLength;
    int newPos = lineLength + crlfCount;

    lineWithLf[newPos] = '\0';
    newPos--;

    // Faire le remplacement en partant de la fin
    while (oldPos > 0) {
        oldPos--;
        char currentChar = lineWithLf[oldPos];

        if (currentChar == '\n' &&
            (oldPos == 0 || lineWithLf[oldPos - 1] != '\r')) {
            // Remplacer '\n' par '\r\n'
            lineWithLf[newPos] = '\n';
            newPos--;
            lineWithLf[newPos] = '\r';
            newPos--;
        } else {
            lineWithLf[newPos] = currentChar;
            newPos--;
        }
    }

    return lineWithLf;
}