#ifndef FRECUENCIA_CARACTERES_H
#define FRECUENCIA_CARACTERES_H

#include <stddef.h>

// Estructura para almacenar un carácter y su frecuencia
typedef struct {
    char caracter;
    unsigned int frecuencia;
} FrecuenciaCaracter;

void ContarFrecuencias(const char* texto, size_t tamaño, FrecuenciaCaracter* tablaFrecuencias, size_t* numCaracteres);
void ImprimirFrecuencias(FrecuenciaCaracter* tablaFrecuencias, size_t numCaracteres);

#endif

