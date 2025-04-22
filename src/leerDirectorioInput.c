#include "leerDirectorioInput.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

ArchivoTexto* LeerDirectorioInput(const char* path, int* num_archivos) {
    DIR* dir;
    struct dirent* entry;
    int capacidad = 10;
    int count = 0;

    ArchivoTexto* archivos = malloc(sizeof(ArchivoTexto) * capacidad);
    if (!archivos) {
        perror("Error al reservar memoria");
        exit(EXIT_FAILURE);
    }

    dir = opendir(path);
    if (!dir) {
        perror("No se pudo abrir el directorio");
        free(archivos);
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        // Solo archivos que terminan en ".txt"
        if (strstr(entry->d_name, ".txt")) {
            char ruta_completa[512];
            snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", path, entry->d_name);

            FILE* archivo = fopen(ruta_completa, "r");
            if (!archivo) {
                perror("Error abriendo un archivo");
                continue;
            }

            // Tamaño del archivo
            fseek(archivo, 0, SEEK_END);
            long tamaño = ftell(archivo);
            fseek(archivo, 0, SEEK_SET);

            // Reservar memoria
            char* contenido = malloc(tamaño + 1);
            if (!contenido) {
                perror("Error reservando memoria para contenido");
                fclose(archivo);
                continue;
            }

            size_t leido = fread(contenido, 1, tamaño, archivo);
            contenido[leido] = '\0';  // Fin de cadena

            fclose(archivo);

            // Guardar datos
            archivos[count].nombre_archivo = strdup(entry->d_name);
            archivos[count].contenido = contenido;
            archivos[count].tamaño = leido;
            count++;

            // Expandir array si hace falta
            if (count >= capacidad) {
                capacidad *= 2;
                archivos = realloc(archivos, sizeof(ArchivoTexto) * capacidad);
                if (!archivos) {
                    perror("Error expandiendo memoria");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    closedir(dir);
    *num_archivos = count;
    return archivos;
}

void LiberarArchivos(ArchivoTexto* archivos, int num_archivos) {
    for (int i = 0; i < num_archivos; ++i) {
        free(archivos[i].nombre_archivo);
        free(archivos[i].contenido);
    }
    free(archivos);
}

char* ConcatenarArchivos(ArchivoTexto* archivos, int num_archivos, size_t* tamaño_total) {
    size_t total = 0;

    for (int i = 0; i < num_archivos; ++i) {
        total += archivos[i].tamaño;
    }

    char* buffer = malloc(total + 1); // +1 para el terminador '\0'
    if (!buffer) {
        perror("Error reservando memoria para el buffer total");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;
    for (int i = 0; i < num_archivos; ++i) {
        memcpy(buffer + offset, archivos[i].contenido, archivos[i].tamaño);
        offset += archivos[i].tamaño;
    }

    buffer[total] = '\0'; // Para que sea un string válido
    if (tamaño_total) {
        *tamaño_total = total;
    }

    return buffer;
}
