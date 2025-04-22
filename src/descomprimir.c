#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "frecuenciaCaracteres.h"
#include "arbolHuffman.h"
#include "descomprimir.h"


// Función para reconstruir árbol de Huffman desde frecuencias
NodoHuffman* ReconstruirArbolDesdeFrecuencias(unsigned int frecuencias[256]) {
    return ConstruirArbolHuffman(frecuencias);
}

void DescomprimirArchivo(const char* ruta_comprimido) {
    FILE* f = fopen(ruta_comprimido, "rb");
    if (!f) {
        perror("Error abriendo archivo comprimido");
        return;
    }

    // Asegurar que la carpeta de recuperación exista
    mkdir("output/recuperado", 0777);

    unsigned int frecuencias[256];
    fread(frecuencias, sizeof(unsigned int), 256, f);

    NodoHuffman* arbol = ReconstruirArbolDesdeFrecuencias(frecuencias);

    int num_archivos = 0;
    fread(&num_archivos, sizeof(int), 1, f);

    // Estructura para guardar información de los archivos
    typedef struct {
        char nombre[256];
        size_t tamaño;
    } InfoArchivo;

    InfoArchivo* archivos = (InfoArchivo*)malloc(num_archivos * sizeof(InfoArchivo));
    if (!archivos) {
        perror("Error al reservar memoria para archivos");
        fclose(f);
        return;
    }

    for (int i = 0; i < num_archivos; ++i) {
        int len_nombre = 0;
        fread(&len_nombre, sizeof(int), 1, f);
        fread(archivos[i].nombre, sizeof(char), len_nombre, f);
        archivos[i].nombre[len_nombre] = '\0';
        fread(&archivos[i].tamaño, sizeof(size_t), 1, f);
    }

    // Ahora, a leer y decodificar los datos comprimidos
    for (int i = 0; i < num_archivos; ++i) {
        // Cambiado aquí: guardar en output/recuperado/
        char ruta_archivo[512];
        snprintf(ruta_archivo, sizeof(ruta_archivo), "output/recuperado/%s", archivos[i].nombre);

        FILE* salida = fopen(ruta_archivo, "wb");
        if (!salida) {
            perror("Error creando archivo de salida");
            continue;
        }

        size_t bytes_escritos = 0;
        NodoHuffman* actual = arbol;
        int byte_actual = 0;
        unsigned char mascara = 0;

        while (bytes_escritos < archivos[i].tamaño) {
            if (mascara == 0) {
                byte_actual = fgetc(f);
                mascara = 0x80; // 1000 0000
            }

            if (byte_actual & mascara)
                actual = actual->derecha;
            else
                actual = actual->izquierda;

            mascara >>= 1;

            if (actual->izquierda == NULL && actual->derecha == NULL) {
                fputc(actual->caracter, salida);
                actual = arbol;
                bytes_escritos++;
            }
        }

        fclose(salida);
        printf("Archivo recuperado: %s\n", ruta_archivo);
    }

    free(archivos);
    LiberarArbol(arbol);
    fclose(f);
}
