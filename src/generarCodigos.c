#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "frecuenciaCaracteres.h"
#include "generarCodigos.h"

// Estructura para mapa de codificación de cada carácter
typedef struct {
    unsigned char caracter;
    char* codigo;
} CodigoCaracter;

// Función recursiva para llenar el mapa de códigos
static void ConstruirTablaCodigos(NodoHuffman* raiz, CodigoCaracter* tabla, char* buffer, int profundidad, int* indice) {
    if (!raiz) return;

    if (raiz->izquierda == NULL && raiz->derecha == NULL) {
        buffer[profundidad] = '\0';
        tabla[*indice].caracter = raiz->caracter;
        tabla[*indice].codigo = strdup(buffer);
        (*indice)++;
        return;
    }

    // Izquierda = '0'
    buffer[profundidad] = '0';
    ConstruirTablaCodigos(raiz->izquierda, tabla, buffer, profundidad + 1, indice);

    // Derecha = '1'
    buffer[profundidad] = '1';
    ConstruirTablaCodigos(raiz->derecha, tabla, buffer, profundidad + 1, indice);
}

// Buscar el código en la tabla
static const char* ObtenerCodigo(CodigoCaracter* tabla, int tamaño, unsigned char c) {
    for (int i = 0; i < tamaño; ++i) {
        if (tabla[i].caracter == c) return tabla[i].codigo;
    }
    return NULL;
}

char* GenerarCodigo(NodoHuffman* raiz, const char* texto) {
    if (!raiz || !texto) return NULL;

    CodigoCaracter tabla[256];
    char buffer[256];
    int indice = 0;

    // Construir tabla de codificación
    ConstruirTablaCodigos(raiz, tabla, buffer, 0, &indice);

    // Estimar tamaño del texto codificado (más que suficiente)
    size_t longitudEstimado = strlen(texto) * 16;
    char* textoCodificado = (char*)malloc(longitudEstimado);
    if (!textoCodificado) {
        perror("Error al asignar memoria para texto codificado");
        exit(EXIT_FAILURE);
    }
    textoCodificado[0] = '\0';

    // Codificar texto
    for (size_t i = 0; texto[i] != '\0'; ++i) {
        const char* codigo = ObtenerCodigo(tabla, indice, (unsigned char)texto[i]);
        if (codigo) {
            strcat(textoCodificado, codigo);
        }
    }

    return textoCodificado;
}

void ImprimirTablaCodigos(NodoHuffman* raiz) {
    CodigoCaracter tabla[256];
    char buffer[256];
    int indice = 0;

    // Construimos la tabla con los códigos de cada carácter
    ConstruirTablaCodigos(raiz, tabla, buffer, 0, &indice);

    printf("\nCódigos Huffman por carácter:\n");
    for (int i = 0; i < indice; ++i) {
        unsigned char c = tabla[i].caracter;
        if (c == 10) {
            printf("'Salto de línea' : %s\n", tabla[i].codigo);
        } else if (c == 32) {
            printf("'Espacio' : %s\n", tabla[i].codigo);
        } else {
            printf("'%c' : %s\n", c, tabla[i].codigo);
        }
        free(tabla[i].codigo); // Liberamos strdup
    }
}

void ImprimirTextoArchivosCodificado(const char* codigo) {
    printf("\nTexto codificado:\n%s\n", codigo);
}
