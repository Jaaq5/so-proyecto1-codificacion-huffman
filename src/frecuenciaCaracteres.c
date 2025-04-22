#include "frecuenciaCaracteres.h"
#include <stdio.h>

void ContarFrecuencias(const char* texto, size_t tamaño, FrecuenciaCaracter* tablaFrecuencias, size_t* numCaracteres) {
    *numCaracteres = 0;  // Inicializamos el contador de caracteres únicos

    for (size_t i = 0; i < tamaño; ++i) {
        unsigned char c = (unsigned char) texto[i];

        // Verificamos si el carácter ya está en la tabla de frecuencias
        int encontrado = 0;
        for (size_t j = 0; j < *numCaracteres; ++j) {
            if (tablaFrecuencias[j].caracter == c) {
                tablaFrecuencias[j].frecuencia++;
                encontrado = 1;
                break;
            }
        }

        // Si el carácter no está en la tabla, lo agregamos
        if (!encontrado) {
            tablaFrecuencias[*numCaracteres].caracter = c;
            tablaFrecuencias[*numCaracteres].frecuencia = 1;
            (*numCaracteres)++;
        }
    }
}

void ImprimirFrecuencias(FrecuenciaCaracter* tablaFrecuencias, size_t numCaracteres) {
    printf("\nFrecuencias de caracteres:\n");
    for (size_t i = 0; i < numCaracteres; ++i) {
        // Si el carácter es un salto de línea (ASCII 10)
        if (tablaFrecuencias[i].caracter == 10) {
            printf("'Salto de línea' : %u veces\n", tablaFrecuencias[i].frecuencia);
        }
        // Si el carácter es un espacio (ASCII 32)
        else if (tablaFrecuencias[i].caracter == 32) {
            printf("'Espacio' : %u veces\n", tablaFrecuencias[i].frecuencia);
        }
        // Para los demás caracteres, imprimir normalmente
        else {
            printf("'%c' : %u veces\n", tablaFrecuencias[i].caracter, tablaFrecuencias[i].frecuencia);
        }
    }
}
