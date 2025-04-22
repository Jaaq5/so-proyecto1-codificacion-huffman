#ifndef ARBOL_HUFFMAN_H
#define ARBOL_HUFFMAN_H

// Estructura del nodo de Huffman
typedef struct NodoHuffman {
    unsigned char caracter;      // Carácter o valor que representa el nodo
    unsigned int frecuencia;     // Frecuencia del carácter
    struct NodoHuffman* izquierda; // Hijo izquierdo
    struct NodoHuffman* derecha;  // Hijo derecho
} NodoHuffman;

// Crear un nodo de Huffman
NodoHuffman* CrearNodo(unsigned char caracter, unsigned int frecuencia);

// Construir el árbol de Huffman
// Cambié el tipo de argumento a FrecuenciaCaracter* y añadí el parámetro de número de caracteres
NodoHuffman* ConstruirArbolHuffman(FrecuenciaCaracter* tablaFrecuencias, size_t numCaracteres);

// Liberar memoria de un árbol de Huffman
void LiberarArbol(NodoHuffman* raiz);

// Imprimir el árbol de Huffman (preorden)
void ImprimirArbolHuffman(NodoHuffman* raiz, int nivel);

#endif
