#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "frecuenciaCaracteres.h"
#include "generarCodigos.h"

// Función auxiliar recursiva para construir los códigos
static void GenerarCodigosRec(NodoHuffman* nodo, char* camino, int profundidad, char** codigos) {
    if (!nodo) return;

    // Verificar si es una hoja
    if (!nodo->izquierda && !nodo->derecha) {
        camino[profundidad] = '\0'; // Finalizar la cadena
        codigos[nodo->caracter] = strdup(camino);
        return;
    }

    // Bajar a la izquierda: agregar '0'
    camino[profundidad] = '0';
    GenerarCodigosRec(nodo->izquierda, camino, profundidad + 1, codigos);

    // Bajar a la derecha: agregar '1'
    camino[profundidad] = '1';
    GenerarCodigosRec(nodo->derecha, camino, profundidad + 1, codigos);
}

void GenerarCodigos(NodoHuffman* raiz, char** codigos) {
    char camino[256]; // Para guardar el camino temporal
    GenerarCodigosRec(raiz, camino, 0, codigos);
}
