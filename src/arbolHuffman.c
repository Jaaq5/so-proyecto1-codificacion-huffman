#include <stdio.h>
#include <stdlib.h>
#include "frecuenciaCaracteres.h"
#include "arbolHuffman.h"

typedef struct ListaNodos {
    NodoHuffman* nodo;
    struct ListaNodos* siguiente;
} ListaNodos;

// Función auxiliar para crear un nodo simple
NodoHuffman* CrearNodo(unsigned char caracter, unsigned int frecuencia) {
    NodoHuffman* nuevo = (NodoHuffman*)malloc(sizeof(NodoHuffman));
    if (!nuevo) {
        perror("Error al asignar memoria para NodoHuffman");
        exit(EXIT_FAILURE);
    }
    nuevo->caracter = caracter;
    nuevo->frecuencia = frecuencia;
    nuevo->izquierda = NULL;
    nuevo->derecha = NULL;
    return nuevo;
}

// Insertar un nodo en la lista ordenada de menor a mayor frecuencia
static ListaNodos* InsertarOrdenado(ListaNodos* lista, NodoHuffman* nodo) {
    ListaNodos* nuevo = (ListaNodos*)malloc(sizeof(ListaNodos));
    if (!nuevo) {
        perror("Error al asignar memoria para ListaNodos");
        exit(EXIT_FAILURE);
    }
    nuevo->nodo = nodo;
    nuevo->siguiente = NULL;

    if (!lista || nodo->frecuencia < lista->nodo->frecuencia) {
        nuevo->siguiente = lista;
        return nuevo;
    }

    ListaNodos* actual = lista;
    while (actual->siguiente && actual->siguiente->nodo->frecuencia <= nodo->frecuencia) {
        actual = actual->siguiente;
    }

    nuevo->siguiente = actual->siguiente;
    actual->siguiente = nuevo;
    return lista;
}


NodoHuffman* ConstruirArbolHuffman(FrecuenciaCaracter* tablaFrecuencias, size_t numCaracteres) {
    ListaNodos* lista = NULL;

    // Crear un nodo para cada caracter con frecuencia > 0
    for (size_t i = 0; i < numCaracteres; ++i) {
        NodoHuffman* nodo = CrearNodo(tablaFrecuencias[i].caracter, tablaFrecuencias[i].frecuencia);
        lista = InsertarOrdenado(lista, nodo);
    }

    // Combinar nodos hasta que quede uno solo
    while (lista && lista->siguiente) {
        ListaNodos* primero = lista;
        ListaNodos* segundo = lista->siguiente;

        NodoHuffman* combinado = CrearNodo(0, primero->nodo->frecuencia + segundo->nodo->frecuencia);
        combinado->izquierda = primero->nodo;
        combinado->derecha = segundo->nodo;

        lista = segundo->siguiente;

        // Insertar el nuevo combinado otra vez ordenadamente
        lista = InsertarOrdenado(lista, combinado);

        // Liberar las estructuras de lista (no los nodos)
        free(primero);
        free(segundo);
    }

    NodoHuffman* raiz = NULL;
    if (lista) {
        raiz = lista->nodo;
        free(lista);
    }

    return raiz;
}


/*
// Construye el árbol de Huffman completo
NodoHuffman* ConstruirArbolHuffman(unsigned int* frecuencias) {
    ListaNodos* lista = NULL;

    // Crear un nodo para cada caracter con frecuencia > 0
    for (int i = 0; i < 256; ++i) {
        if (frecuencias[i] > 0) {
            NodoHuffman* nodo = CrearNodo((unsigned char)i, frecuencias[i]);
            lista = InsertarOrdenado(lista, nodo);
        }
    }

    // Combinar nodos hasta que quede uno solo
    while (lista && lista->siguiente) {
        ListaNodos* primero = lista;
        ListaNodos* segundo = lista->siguiente;

        NodoHuffman* combinado = CrearNodo(0, primero->nodo->frecuencia + segundo->nodo->frecuencia);
        combinado->izquierda = primero->nodo;
        combinado->derecha = segundo->nodo;

        lista = segundo->siguiente;

        // Insertar el nuevo combinado otra vez ordenadamente
        lista = InsertarOrdenado(lista, combinado);

        // Liberar las estructuras de lista (no los nodos)
        free(primero);
        free(segundo);
    }

    NodoHuffman* raiz = NULL;
    if (lista) {
        raiz = lista->nodo;
        free(lista);
    }

    return raiz;
}
*/

// Liberar memoria del árbol completo
void LiberarArbol(NodoHuffman* raiz) {
    if (!raiz) return;
    LiberarArbol(raiz->izquierda);
    LiberarArbol(raiz->derecha);
    free(raiz);
}

// Imprimir el árbol de Huffman en preorden (raíz -> izquierda -> derecha)
void ImprimirArbolHuffman(NodoHuffman* raiz, int nivel) {
    if (raiz == NULL) {
        return;
    }

    // Imprimir el nodo actual
    for (int i = 0; i < nivel; ++i) {
        printf("  ");  // Indentación para representar los niveles del árbol
    }
    if (raiz->caracter == 0) {
        printf("Nodo interno: Frecuencia %u\n", raiz->frecuencia); // Nodo combinado
    } else {
        printf("Carácter: '%c', Frecuencia: %u\n", raiz->caracter, raiz->frecuencia); // Nodo hoja
    }

    // Recursión para los nodos izquierdo y derecho
    ImprimirArbolHuffman(raiz->izquierda, nivel + 1);
    ImprimirArbolHuffman(raiz->derecha, nivel + 1);
}

