#ifndef LEER_DIRECTORIO_INPUT_H
#define LEER_DIRECTORIO_INPUT_H

#include <stddef.h>  // Para size_t

typedef struct {
    char* nombre_archivo;
    char* contenido;
    size_t tamaño;
} ArchivoTexto;

// Lee todos los archivos .txt de un directorio dado
ArchivoTexto* LeerDirectorioInput(const char* path, int* num_archivos);

// Libera memoria usada por los archivos
void LiberarArchivos(ArchivoTexto* archivos, int num_archivos);

// Concatenar el contenido de todos los archivos en un solo buffer.
// Devuelve el buffer (malloc) y guarda el tamaño total en tamaño_total.
char* ConcatenarArchivos(ArchivoTexto* archivos, int num_archivos, size_t* tamaño_total);

#endif
