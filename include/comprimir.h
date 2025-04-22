#ifndef COMPRIMIR_H
#define COMPRIMIR_H

#include "arbolHuffman.h"
#include "leerDirectorioInput.h"

void ComprimirContenido(char* contenido, char* codigos[256], ArchivoTexto* archivos, int num_archivos, NodoHuffman* arbol);

#endif
