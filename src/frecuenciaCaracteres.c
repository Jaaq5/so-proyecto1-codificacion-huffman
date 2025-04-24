#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include <locale.h>

#include "frecuenciaCaracteres.h"

void ContarFrecuencias(const char* texto,
                       FrecuenciaCaracter** tablaFrecuencias,
                       size_t* numCaracteres, size_t* capacidadTabla) {
    *numCaracteres = 0;

    while (*texto) {
        uint32_t c = DecodificarUTF8(&texto);

        // Verificamos si ya existe
        int encontrado = 0;
        for (size_t j = 0; j < *numCaracteres; ++j) {
            if ((*tablaFrecuencias)[j].caracter == c) {
                (*tablaFrecuencias)[j].frecuencia++;
                encontrado = 1;
                break;
            }
        }

        if (!encontrado) {
            // Expandir si es necesario
            if (*numCaracteres >= *capacidadTabla) {
                *capacidadTabla = (*capacidadTabla == 0) ? 16 : (*capacidadTabla * 2);
                *tablaFrecuencias = realloc(*tablaFrecuencias, (*capacidadTabla) * sizeof(FrecuenciaCaracter));
                if (!(*tablaFrecuencias)) {
                    perror("Error al reallocar memoria");
                    exit(EXIT_FAILURE);
                }
            }

            (*tablaFrecuencias)[*numCaracteres].caracter = c;
            (*tablaFrecuencias)[*numCaracteres].frecuencia = 1;
            (*numCaracteres)++;
        }
    }
                       }


// Devuelve el siguiente carácter Unicode del texto UTF-8
// Avanza el puntero y retorna el código Unicode
uint32_t DecodificarUTF8(const char** texto) {

    const unsigned char* bytes = (const unsigned char*)(*texto);
    uint32_t unicode = 0;
    int longitud = 0;

    if (bytes[0] < 0x80) {
        unicode = bytes[0];
        longitud = 1;
    } else if ((bytes[0] & 0xE0) == 0xC0) {
        unicode = ((bytes[0] & 0x1F) << 6) | (bytes[1] & 0x3F);
        longitud = 2;
    } else if ((bytes[0] & 0xF0) == 0xE0) {
        unicode = ((bytes[0] & 0x0F) << 12) | ((bytes[1] & 0x3F) << 6) | (bytes[2] & 0x3F);
        longitud = 3;
    } else if ((bytes[0] & 0xF8) == 0xF0) {
        unicode = ((bytes[0] & 0x07) << 18) | ((bytes[1] & 0x3F) << 12) |
        ((bytes[2] & 0x3F) << 6) | (bytes[3] & 0x3F);
        longitud = 4;
    } else {
        // Carácter inválido
        unicode = 0xFFFD;
        longitud = 1;
    }

    *texto += longitud;
    return unicode;
}

// Para imprimir en consola el caracter
int CodificarUTF8(uint32_t unicode, char* salida) {
    if (unicode <= 0x7F) {
        salida[0] = (char)unicode;
        return 1;
    } else if (unicode <= 0x7FF) {
        salida[0] = (char)(0xC0 | ((unicode >> 6) & 0x1F));
        salida[1] = (char)(0x80 | (unicode & 0x3F));
        return 2;
    } else if (unicode <= 0xFFFF) {
        salida[0] = (char)(0xE0 | ((unicode >> 12) & 0x0F));
        salida[1] = (char)(0x80 | ((unicode >> 6) & 0x3F));
        salida[2] = (char)(0x80 | (unicode & 0x3F));
        return 3;
    } else if (unicode <= 0x10FFFF) {
        salida[0] = (char)(0xF0 | ((unicode >> 18) & 0x07));
        salida[1] = (char)(0x80 | ((unicode >> 12) & 0x3F));
        salida[2] = (char)(0x80 | ((unicode >> 6) & 0x3F));
        salida[3] = (char)(0x80 | (unicode & 0x3F));
        return 4;
    }

    // Carácter inválido
    salida[0] = '?';
    return 1;
}


void ImprimirFrecuencias(FrecuenciaCaracter* tabla, size_t numCaracteres) {
    setlocale(LC_CTYPE, "");  // Activar soporte UTF-8 en consola

    printf("\nFrecuencias de caracteres:\n");
    for (size_t i = 0; i < numCaracteres; ++i) {
        uint32_t c = tabla[i].caracter;

        if (c == '\n') {
            printf("'Salto de línea' : %u veces\n", tabla[i].frecuencia);
        } else if (c == ' ') {
            printf("'Espacio' : %u veces\n", tabla[i].frecuencia);
        } else {
            char utf8[5] = {0}; // espacio para máximo 4 bytes UTF-8 + terminador
            CodificarUTF8(c, utf8);
            printf("'%s' : %u veces\n", utf8, tabla[i].frecuencia);
        }
    }
}
