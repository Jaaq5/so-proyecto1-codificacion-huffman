#ifndef MEDIR_TIEMPO_H
#define MEDIR_TIEMPO_H

/*
 * Inicia la medición del tiempo.
 * Entradas: ninguna.
 * Salidas: ninguna.
 * Uso: llamar antes del bloque de código que se desea medir.
 */
void iniciarMedicionT();

/*
 * Finaliza la medición del tiempo y muestra el resultado en consola.
 * Entradas: ninguna.
 * Salidas: ninguna.
 * Uso: llamar después del bloque de código que se midió.
 */
void finalizarMedicionT();

/*
 * Obtiene la diferencia de tiempo en nanosegundos entre el inicio y el fin de la medición.
 * Entradas: ninguna.
 * Salidas: long (tiempo transcurrido en nanosegundos).
 */
long obtenerUltimaDiferenciaNS();

/*
 * Obtiene la diferencia de tiempo en segundos (con precisión decimal) entre el inicio y el fin.
 * Entradas: ninguna.
 * Salidas: double (tiempo transcurrido en segundos).
 */
double obtenerUltimaDiferenciaSeg();

#endif  // MEDIR_TIEMPO_H

