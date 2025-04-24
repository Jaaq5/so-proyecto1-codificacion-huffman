#ifndef FRECUENCIA_CARACTERES_H
#define FRECUENCIA_CARACTERES_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t caracter;       // Código Unicode
    unsigned int frecuencia;
} FrecuenciaCaracter;

void ContarFrecuencias(const char* texto,
                       FrecuenciaCaracter** tablaFrecuencias,
                       size_t* numCaracteres, size_t* capacidadTabla);

uint32_t DecodificarUTF8(const char** texto);

int CodificarUTF8(uint32_t unicode, char* salida);

void ImprimirFrecuencias(FrecuenciaCaracter* tablaFrecuencias, size_t numCaracteres);

#endif

