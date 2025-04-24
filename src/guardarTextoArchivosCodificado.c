#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "guardarTextoArchivosCodificado.h"

// Función que recibe el texto codificado y lo guarda en un archivo .bin
void guardarTextoArchivosCodificado(const char* textoArchivosCodificado, const char* rutaArchivo) {
    // Abrir el archivo para escritura binaria
    FILE* archivo = fopen(rutaArchivo, "wb");

    // Verificar si hubo un error al abrir el archivo
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    // Escribir el texto codificado en el archivo
    size_t longitudTexto = strlen(textoArchivosCodificado);
    size_t bytesEscritos = fwrite(textoArchivosCodificado, sizeof(char), longitudTexto, archivo);

    // Verificar si la escritura fue exitosa
    if (bytesEscritos != longitudTexto) {
        perror("Error al escribir en el archivo");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    // Cerrar el archivo después de escribir
    fclose(archivo);
    printf("Texto codificado guardado exitosamente en %s\n", rutaArchivo);
}
