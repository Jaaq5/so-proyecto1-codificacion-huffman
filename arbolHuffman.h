#ifndef ARBOLHUFFMAN_H
#define ARBOLHUFFMAN_H

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

/*
 * Nodo del árbol de Huffman.
 * Cada nodo representa un símbolo (hoja) o una combinación de símbolos (nodo interno).
 * - freq: frecuencia del símbolo.
 * - symbol: valor del símbolo (0-255 para hojas, -1 para nodos internos).
 * - left, right: punteros a los nodos hijos izquierdo y derecho.
 */
typedef struct Node {
    uint64_t freq;
    int symbol;
    struct Node *left;
    struct Node *right;
} Node;

/*
 * Estructura para almacenar información de un archivo.
 * Utilizada tanto durante la compresión como para generar el encabezado del archivo comprimido.
 * - name: nombre del archivo.
 * - path: ruta completa del archivo.
 * - orig_size: tamaño original del archivo en bytes.
 * - bit_length: tamaño en bits después de la compresión.
 */
typedef struct {
    char *name;
    char *path;
    uint64_t orig_size;
    uint64_t bit_length;
} FileInfo;

/*
 * Montículo mínimo (MinHeap) usado para construir el árbol de Huffman.
 * - data: arreglo dinámico de punteros a Node.
 * - size: cantidad actual de elementos.
 * - capacity: capacidad máxima del arreglo.
 */
typedef struct {
    Node **data;
    int size;
    int capacity;
} MinHeap;

/*
 * Escritor de bits (BitWriter) para escribir bits en un archivo FILE.
 * - f: puntero al archivo donde se escriben los bits.
 * - buf: buffer de 8 bits donde se acumulan los bits a escribir.
 * - bit_pos: posición actual en el buffer (de 0 a 7).
 */
typedef struct {
    FILE *f;
    uint8_t buf;
    int bit_pos;  // [0..7]
} BitWriter;

/*
 * Lector de bits (BitReader) para leer bits desde un buffer en memoria.
 * - data: puntero al buffer de datos.
 * - size: tamaño del buffer en bytes.
 * - idx: índice actual de lectura dentro del buffer.
 * - bit_pos: posición actual del bit en el byte (de 0 a 7).
 */
typedef struct {
    uint8_t *data;
    size_t size;
    size_t idx;
    int bit_pos;
} BitReader;

#endif

