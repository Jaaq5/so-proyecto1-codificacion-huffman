#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "menu.h"
#include "leerDirectorioInput.h"
#include "frecuenciaCaracteres.h"
#include "arbolHuffman.h"
#include "generarCodigos.h"
#include "comprimir.h"
#include "descomprimir.h"




int main() {

    int opcion_principal;
    int opcion_accion;

    do {
        opcion_principal = MostrarMenuPrincipal();

        if (opcion_principal == 4) {
            printf("Saliendo del programa...\n");
            break;
        }

        // Imprimimos qué tipo seleccionó
        switch (opcion_principal) {
            case 1:
                printf("Selecciono: Codificacion de Huffman serial\n");
                break;
            case 2:
                printf("Selecciono: Codificacion de Huffman paralela (fork)\n");
                break;
            case 3:
                printf("Selecciono: Codificacion de Huffman concurrente (pthread)\n");
                break;
        }

        do {
            opcion_accion = MostrarMenuAcciones(opcion_principal);

            if (opcion_accion == 3) {
                printf("Volviendo al menu principal...\n");
                break;
            }

            if (opcion_accion == 1) {
                switch (opcion_principal) {
                    case 1:
                        //printf("Comprimir (serial): aqui llamarías a tu funcion de compresion serial\n");
                        int num_archivos = 0;
                        char directorio_usuario[256];
                        char path[300];
                        // Bucle hasta que el usuario ingrese un directorio válido
                        while (1) {
                            printf("Ingrese el nombre del directorio: ");
                            scanf("%255s", directorio_usuario);

                            snprintf(path, sizeof(path), "%s", directorio_usuario);

                            struct stat st;
                            if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
                                break; // El directorio existe y es válido
                            } else {
                                printf("El directorio '%s' no existe. Intente nuevamente.\n", path);
                            }
                        }

                        ArchivoTexto* archivos = LeerDirectorioInput(path, &num_archivos);
                        printf("Se leyeron %d archivos:\n", num_archivos);
                        for (int i = 0; i < num_archivos; ++i) {
                            printf("Archivo: %s (tamaño: %zu bytes)\n", archivos[i].nombre_archivo, archivos[i].tamaño);
                        }


                        // Concatenar todo el contenido en un solo buffer
                        size_t tamaño_total = 0;
                        char* todo_contenido = ConcatenarArchivos(archivos, num_archivos, &tamaño_total);

                        tamaño_total = strlen(todo_contenido);
                        printf("\nContenido total concatenado (%zu bytes).\n", tamaño_total);

                        // Inicializar la tabla de frecuencias
                        FrecuenciaCaracter tablaFrecuencias[256];
                        size_t numCaracteres = 0;

                        // Contar las frecuencias de los caracteres
                        ContarFrecuencias(todo_contenido, tamaño_total, tablaFrecuencias, &numCaracteres);

                        // Imprimir las frecuencias
                        ImprimirFrecuencias(tablaFrecuencias, numCaracteres);

                        // Construir el árbol de Huffman
                        NodoHuffman* arbol = ConstruirArbolHuffman(tablaFrecuencias, numCaracteres);

                        // Imprimir el árbol de Huffman (Recorrido Preorden)
                        printf("\nÁrbol de Huffman (Recorrido Preorden):\n");
                        ImprimirArbolHuffman(arbol, 0);  // Llamar con nivel 0 para imprimir

                        // Liberar la memoria del árbol
                        LiberarArbol(arbol);




                        break;
                    case 2:
                        printf("Comprimir (paralelo/fork): aqui llamarías a tu funcion de compresion usando fork\n");
                        break;
                    case 3:
                        printf("Comprimir (concurrente/pthread): aqui llamarías a tu funcion de compresion usando pthread\n");
                        break;
                }
            } else if (opcion_accion == 2) {
                switch (opcion_principal) {
                    case 1:
                        printf("Descomprimir (serial): aqui llamarías a tu funcion de descompresion serial\n");
                        break;
                    case 2:
                        printf("Descomprimir (paralelo/fork): aqui llamarías a tu funcion de descompresion usando fork\n");
                        break;
                    case 3:
                        printf("Descomprimir (concurrente/pthread): aqui llamarías a tu funcion de descompresion usando pthread\n");
                        break;
                }
            }

        } while (opcion_accion != 3);

    } while (opcion_principal != 4);


/*

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    int num_archivos = 0;
    char directorio_usuario[256];
    char path[300];
    // Bucle hasta que el usuario ingrese un directorio válido
    while (1) {
        printf("Ingrese el nombre del directorio: ");
        scanf("%255s", directorio_usuario);

        snprintf(path, sizeof(path), "%s", directorio_usuario);

        struct stat st;
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
            break; // El directorio existe y es válido
        } else {
            printf("El directorio '%s' no existe. Intente nuevamente.\n", path);
        }
    }
    ArchivoTexto* archivos = LeerDirectorioInput(path, &num_archivos);
    printf("Se leyeron %d archivos:\n", num_archivos);

    for (int i = 0; i < num_archivos; ++i) {
        printf("Archivo: %s (tamaño: %zu bytes)\n", archivos[i].nombre_archivo, archivos[i].tamaño);
    }


    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Concatenar todo el contenido en un solo buffer
    size_t tamaño_total = 0;
    char* todo_contenido = ConcatenarArchivos(archivos, num_archivos, &tamaño_total);

    // Contar frecuencias
    unsigned int frecuencias[256] = {0};
    ContarFrecuencias(todo_contenido, tamaño_total, frecuencias);

    // Mostrar caracteres con frecuencia > 0
    printf("\nFrecuencias de caracteres:\n");
    for (int i = 0; i < 256; ++i) {
        if (frecuencias[i] > 0) {
            if (i == 10) {
                printf("'Salto de línea' (%d) : %u veces\n", i, frecuencias[i]);
            } else if (i == 32) {
                printf("'Espacio' (%d) : %u veces\n", i, frecuencias[i]);
            } else if (i >= 32 && i <= 126) { // Caracteres imprimibles normales
                printf("'%c' (%d) : %u veces\n", i, i, frecuencias[i]);
            } else {
                printf("'(no imprimible)' (%d) : %u veces\n", i, frecuencias[i]);
            }
        }
    }


    printf("\nContenido total concatenado (%zu bytes).\n", tamaño_total);


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Construir el árbol de Huffman
    NodoHuffman* arbol = ConstruirArbolHuffman(frecuencias);

    printf("\nÁrbol de Huffman construido.\n");

    // Generar los códigos de Huffman
    char* codigos[256] = {NULL};
    GenerarCodigos(arbol, codigos);

    printf("\nCódigos de Huffman generados:\n");
    for (int i = 0; i < 256; ++i) {
        if (codigos[i]) {
            if (i == 10)
                printf("'Salto de línea' (%d) : %s\n", i, codigos[i]);
            else if (i == 32)
                printf("'Espacio' (%d) : %s\n", i, codigos[i]);
            else if (i >= 32 && i <= 126)
                printf("'%c' (%d) : %s\n", i, i, codigos[i]);
            else
                printf("'(no imprimible)' (%d) : %s\n", i, codigos[i]);
        }
    }

    // Comprimir contenido en el archivo de salida "output.bin"
    ComprimirContenido(todo_contenido, codigos, archivos, num_archivos, arbol);

    DescomprimirArchivo("output/compressed_output.bin");

    // Liberar códigos
    for (int i = 0; i < 256; ++i) {
        free(codigos[i]);
    }


    // Luego liberar la memoria
    LiberarArbol(arbol);

    // (Opcional) Puedes mostrar el contenido:
    // printf("%s\n", todo_contenido);

    // Liberar memoria
    free(todo_contenido);
    LiberarArchivos(archivos, num_archivos);

*/

    return 0;
}
