# Makefile examples
# https://makefiletutorial.com
# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor

# Compilador y flags
# -Wall para advertencias
# -Iinclude para cabeceras
CC = gcc
CFLAGS = -Wall -Iinclude

# Archivos fuente
SRC = \
    src/main.c \
    src/menu.c \
    src/leerDirectorioInput.c \
    src/frecuenciaCaracteres.c \
    src/arbolHuffman.c \
    src/generarCodigos.c \
    src/guardarTextoArchivosCodificado.c \
    src/comprimir.c \
    #src/descomprimir.c \

# Archivos objeto
OBJ = \
    build/main.o \
    build/menu.o \
    build/leerDirectorioInput.o \
    build/frecuenciaCaracteres.o \
    build/arbolHuffman.o \
    build/generarCodigos.o \
    build/guardarTextoArchivosCodificado.o \
    build/comprimir.o \
    #build/descomprimir.o

# Ejecutable
# "./bin/proyecto1" en terminal
BIN = bin/proyecto1

# Regla principal
all: crear_carpetas $(BIN)

# Crear carpetas necesarias
crear_carpetas:
	mkdir -p build bin output

# Compilar archivos .c en .o
build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Construir el ejecutable
$(BIN): $(OBJ)
	$(CC) -o $@ $^

# Limpiar carpetas autogeneradas ("make clean" en terminal)
clean:
	rm -rf build bin output
