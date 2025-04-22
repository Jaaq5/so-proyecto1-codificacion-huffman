#include <stdio.h>
#include <stdlib.h>
#include "menu.h"

int MostrarMenuPrincipal() {
    int opcion = 0;
    do {
        // Limpia la pantalla
        system("clear");

        printf("\nBienvenido al programa de codificacion de Huffman\n");
        printf("1. Codificacion de Huffman serial\n");
        printf("2. Codificacion de Huffman paralela (fork)\n");
        printf("3. Codificacion de Huffman concurrente (pthread)\n");
        printf("4. Salir\n");
        printf("Seleccione una opcion (1-4): ");

        if (scanf("%d", &opcion) != 1) {
            while (getchar() != '\n'); // limpia buffer
            opcion = 0;
        }

        if (opcion < 1 || opcion > 4) {
            printf("Entrada invalida, intente de nuevo (1-4).\n");
        }

    } while (opcion < 1 || opcion > 4);

    return opcion;
}

int MostrarMenuAcciones(int version) {
    int opcion = 0;
    const char* versionAlgoritmo;

    // Definimos nombre para mostrar ejemplos más claros
    switch (version) {
        case 1: versionAlgoritmo = "serial"; break;
        case 2: versionAlgoritmo = "paralela (fork)"; break;
        case 3: versionAlgoritmo = "concurrente (pthread)"; break;
        default: versionAlgoritmo = "desconocido"; break;
    }

    do {
        printf("\n1. Comprimir (%s)\n", versionAlgoritmo);
        printf("2. Descomprimir (%s)\n", versionAlgoritmo);
        printf("3. Atras\n");
        printf("Seleccione una opcion (1-3): ");

        if (scanf("%d", &opcion) != 1) {
            while (getchar() != '\n');
            opcion = 0;
        }

        if (opcion < 1 || opcion > 3) {
            printf("Entrada invalida, intente de nuevo (1-3).\n");
        }

    } while (opcion < 1 || opcion > 3);

    return opcion;
}
