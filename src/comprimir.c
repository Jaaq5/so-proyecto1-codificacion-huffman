#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "frecuenciaCaracteres.h"
#include "arbolHuffman.h"
#include "comprimir.h"
#include "leerDirectorioInput.h"

void SerializarArbol(NodoHuffman* nodo, FILE* archivo) {
    if (!nodo) return;

    if (!nodo->izquierda && !nodo->derecha) {
        // Es una hoja
        fputc('1', archivo);          // Marca de hoja
        fputc(nodo->caracter, archivo); // El caracter
    } else {
        // Es un nodo interno
        fputc('0', archivo);
        SerializarArbol(nodo->izquierda, archivo);
        SerializarArbol(nodo->derecha, archivo);
    }
}

void ComprimirContenido(char* contenido, char* codigos[256], ArchivoTexto* archivos, int num_archivos, NodoHuffman* arbol) {
    FILE* salida = fopen("output/compressed_output.bin", "wb");
    if (!salida) {
        perror("No se pudo abrir compressed_output.bin para escribir");
        exit(EXIT_FAILURE);
    }

    // 1. Escribir cantidad de archivos
    uint32_t cantidad = (uint32_t) num_archivos;
    fwrite(&cantidad, sizeof(uint32_t), 1, salida);

    // 2. Escribir información de cada archivo
    for (int i = 0; i < num_archivos; ++i) {
        uint32_t longitud_nombre = (uint32_t) strlen(archivos[i].nombre_archivo);
        fwrite(&longitud_nombre, sizeof(uint32_t), 1, salida);
        fwrite(archivos[i].nombre_archivo, sizeof(char), longitud_nombre, salida);
        uint32_t tamaño_archivo = (uint32_t) archivos[i].tamaño;
        fwrite(&tamaño_archivo, sizeof(uint32_t), 1, salida);
    }

    // 3. Serializar el árbol de Huffman
    SerializarArbol(arbol, salida);

    // 4. Comprimir los datos
    uint8_t buffer = 0;
    int bits_en_buffer = 0;

    for (size_t i = 0; contenido[i] != '\0'; ++i) {
        char* codigo = codigos[(unsigned char)contenido[i]];

        for (int j = 0; codigo[j] != '\0'; ++j) {
            buffer <<= 1;
            if (codigo[j] == '1') {
                buffer |= 1;
            }
            bits_en_buffer++;

            if (bits_en_buffer == 8) {
                fwrite(&buffer, sizeof(uint8_t), 1, salida);
                buffer = 0;
                bits_en_buffer = 0;
            }
        }
    }

    // Escribir los bits restantes si quedaron
    if (bits_en_buffer > 0) {
        buffer <<= (8 - bits_en_buffer); // Rellenar con ceros a la derecha
        fwrite(&buffer, sizeof(uint8_t), 1, salida);
    }

    fclose(salida);

    printf("\nArchivo comprimido generado como 'output/compressed_output.bin'\n");
}
