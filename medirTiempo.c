#define _POSIX_C_SOURCE 199309L  // Necesario para usar clock_gettime con CLOCK_MONOTONIC

#include <stdio.h>
#include <time.h>

#include "medirTiempo.h"

// Variables estáticas internas para almacenar los tiempos de inicio y fin
static struct timespec inicio, fin;

/*
 * Inicia la medición de tiempo utilizando clock_gettime con CLOCK_MONOTONIC.
 * Entradas: ninguna.
 * Salidas: ninguna.
 * Uso: llamar esta función justo antes de la sección de código que se desea medir.
 */
void iniciarMedicionT() {
    clock_gettime(CLOCK_MONOTONIC, &inicio);
}

/*
 * Finaliza la medición de tiempo, calcula y muestra el tiempo transcurrido.
 * Entradas: ninguna.
 * Salidas: ninguna (imprime el resultado por consola).
 * Uso: llamar esta función inmediatamente después de la sección de código que se midió.
 */
void finalizarMedicionT() {
    clock_gettime(CLOCK_MONOTONIC, &fin);
    long ns = obtenerUltimaDiferenciaNS();
    double seg = obtenerUltimaDiferenciaSeg();
    printf("Tiempo transcurrido: %ld ns (%.9f segundos)\n", ns, seg);
}

/*
 * Calcula y devuelve la diferencia de tiempo entre el inicio y el fin en nanosegundos.
 * Entradas: ninguna.
 * Salidas: long (diferencia de tiempo en nanosegundos).
 */
long obtenerUltimaDiferenciaNS() {
    return (fin.tv_sec - inicio.tv_sec) * 1000000000L +
    (fin.tv_nsec - inicio.tv_nsec);
}

/*
 * Calcula y devuelve la diferencia de tiempo entre el inicio y el fin en segundos (con precisión decimal).
 * Entradas: ninguna.
 * Salidas: double (diferencia de tiempo en segundos).
 */
double obtenerUltimaDiferenciaSeg() {
    return obtenerUltimaDiferenciaNS() / 1e9;
}

/*
 * Ejemplo de uso en main.c:
 *
 * #include "medirTiempo.h"
 *
 * int main() {
 *     iniciarMedicionT();
 *
 *     funcionPorMedirT();  // Código cuya ejecución se desea medir
 *
 *     finalizarMedicionT();
 *
 *     long tiempoNS = obtenerUltimaDiferenciaNS();
 *     double tiempoSeg = obtenerUltimaDiferenciaSeg();
 *
 *     // Mostrar tiempo medido
 *     printf("Tiempo transcurrido: %ld ns (%.9f segundos)\n", tiempoNS, tiempoSeg);
 *
 *     return 0;
 * }
 */
