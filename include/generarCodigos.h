#ifndef GENERAR_CODIGOS_H
#define GENERAR_CODIGOS_H

#include "arbolHuffman.h"

char* GenerarCodigo(NodoHuffman* raiz, const char* texto);
void ImprimirTablaCodigos(NodoHuffman* raiz);
void ImprimirTextoArchivosCodificado(const char* codigo);

#endif

