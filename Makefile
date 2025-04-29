# Compilador a utilizar
CC = gcc

# Opciones de compilación: usar estándar C11 y optimización nivel 2
CFLAGS = -std=c11 -O2

# Opciones de enlace: enlazar la librería pthread
LDFLAGS = -lpthread

# Lista de archivos objeto
OBJETOS = main.o arbolHuffman.o huffmanSerial.o huffmanPthread.o huffmanFork.o medirTiempo.o

# Nombre del ejecutable final
EJECUTABLE = proyecto1

# Regla principal: construir el ejecutable
all: $(EJECUTABLE)

# Cómo construir el ejecutable a partir de los objetos
$(EJECUTABLE): $(OBJETOS)
	$(CC) $(CFLAGS) -o $@ $(OBJETOS) $(LDFLAGS)

# Reglas para compilar cada archivo fuente a objeto

# Compilar main.c
main.o: main.c medirTiempo.h
	$(CC) $(CFLAGS) -c main.c

# Compilar medirTiempo.c
medirTiempo.o: medirTiempo.c medirTiempo.h
	$(CC) $(CFLAGS) -c medirTiempo.c

# Compilar arbolHuffman.c
arbolHuffman.o: arbolHuffman.c arbolHuffman.h
	$(CC) $(CFLAGS) -c arbolHuffman.c

# Compilar huffmanSerial.c
huffmanSerial.o: huffmanSerial.c medirTiempo.h
	$(CC) $(CFLAGS) -c huffmanSerial.c

# Compilar huffmanPthread.c
huffmanPthread.o: huffmanPthread.c medirTiempo.h
	$(CC) $(CFLAGS) -c huffmanPthread.c

# Compilar huffmanFork.c
huffmanFork.o: huffmanFork.c medirTiempo.h
	$(CC) $(CFLAGS) -c huffmanFork.c

# Comando para limpiar los archivos generados
clean:
	rm -f $(OBJETOS) $(EJECUTABLE)



