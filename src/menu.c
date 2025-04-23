// Bibliotecas
#include <stdio.h>   // Para printf(), scanf()
#include <stdlib.h>  // Para system()

// Encabezados
#include "menu.h"

/*
 * Muestra el menú principal al usuario, solicita una opción válida (1-4) y la devuelve.
 * Entradas: ninguna.
 * Salidas: int opcion (valor entre 1 y 4 que representa la elección del usuario).
 * Ejemplo: opcionMenuPrincipal = MenuPrincipal();
 */
int MenuPrincipal() {
    int opcion = 0;

    // Limpia la pantalla de la consola
    system("clear");

    do {

        printf("\nBienvenido al programa de codificacion de Huffman\n");
        printf("1. Codificacion de Huffman serial\n");
        printf("2. Codificacion de Huffman paralela (fork)\n");
        printf("3. Codificacion de Huffman concurrente (pthread)\n");
        printf("4. Salir\n");
        printf("Seleccione una opcion (1-4): ");

        // Lee la entrada del usuario
        if (scanf("%d", &opcion) != 1) {
            // Si la entrada no es numérica, limpia el buffer
            while (getchar() != '\n');
            // Reinicia opción
            opcion = 0;
        }

        // Verifica si la opción está fuera del rango válido
        if (opcion < 1 || opcion > 4) {
            printf("Entrada invalida, intente de nuevo (1-4).\n");
        }

    // Repite mientras la opción no sea válida
    } while (opcion < 1 || opcion > 4);

    return opcion;
}


/*
 * Muestra el menú secundario de acciones, solicita una opción válida (1-3) y la devuelve.
 * Entradas: int opcionPrincipal (tipo de version: 1=serial, 2=paralela/fork, 3=concurrente/pthread).
 * Salidas: int opcion (valor entre 1 y 3 que representa la acción (comprimir/descomprimir) elegida).
 * Ejemplo: opcionMenuSecundario = MenuSecundario(1);
 */
int MenuSecundario(int opcionPrincipal) {
    int opcion = 0;
    const char* versionAlgoritmo;

    // Asigna el nombre de la versión del algoritmo según la opción principal
    switch (opcionPrincipal) {
        case 1: versionAlgoritmo = "serial"; break;
        case 2: versionAlgoritmo = "paralela (fork)"; break;
        case 3: versionAlgoritmo = "concurrente (pthread)"; break;
    }

    do {
        printf("\n1. Comprimir (%s)\n", versionAlgoritmo);
        printf("2. Descomprimir (%s)\n", versionAlgoritmo);
        printf("3. Atras\n");
        printf("Seleccione una opcion (1-3): ");

        // Lee la entrada del usuario
        if (scanf("%d", &opcion) != 1) {
            // Si la entrada no es numérica, limpia el buffer
            while (getchar() != '\n');
            opcion = 0;
        }

        // Verifica si la opción está fuera del rango válido
        if (opcion < 1 || opcion > 3) {
            printf("Entrada invalida, intente de nuevo (1-3).\n");
        }

    // Repite mientras la opción no sea válida
    } while (opcion < 1 || opcion > 3);

    return opcion;
}
