/*
 * huffmanFork.c
 *
 * Compresión y descompresión de archivos de texto usando el algoritmo de Huffman.
 *
 * La compresión se paraleliza con fork(): después de contar las frecuencias y
 * generar el árbol global y los códigos, se forkea un proceso por cada archivo que
 * genera su sección comprimida en un archivo temporal. Luego el padre "une" los
 * resultados en el archivo final de salida.
 *
 * La descompresión se realiza usando fork() para cada archivo descomprimido (ya que
 * esto es más sencillo dado que cada bloque se procesa de forma independiente).
 *
 * Uso:
 *   Para comprimir:    ./huffmanFork -c <dir_txt> <salida.huf>
 *   Para descomprimir: ./huffmanFork -d <entrada.huf> <dir_destino>
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#include "arbolHuffman.h"
#include "medirTiempo.h"

#define MAGIC "HUF\0"
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

//----------------- Estructuras y Tipos -----------------

/*
 / /* Nodo del árbol de Huffman
 typedef struct Node {
    uint64_t freq;
    int symbol;  // -1 para nodo interno
    struct Node *left, *right;
 } Node;

 // Información de cada archivo (se usa en compresión y en el encabezado del archivo comprimido)
 typedef struct {
    char *name;           // Nombre del archivo (para el encabezado)
    char *path;           // Ruta completa (para localizar el archivo)
    uint64_t orig_size;   // Tamaño original (en bytes)
    uint64_t bit_length;  // Longitud en bits tras la compresión
 } FileInfo;


 // Montículo mínimo (MinHeap) para construir el árbol de Huffman
 typedef struct {
    Node **data;
    int size;
    int capacity;
 } MinHeap;

 // BitWriter para escribir bits sobre un FILE
 typedef struct {
    FILE *f;
    uint8_t buf;
    int bit_pos;  // [0..7]
 } BitWriter;

 // BitReader para leer bits desde un buffer en memoria
 typedef struct {
    uint8_t *data;
    size_t size;
    size_t idx;
    int bit_pos;
 } BitReader;

 */






//----------------- Prototipos -----------------

// Funciones principales de compresión y descompresión
void compress_directory(const char *dirpath, const char *outpath);
void decompress_archive(const char *inpath, const char *outdir);

// Funciones auxiliares
int scan_directory_f(const char *dirpath, FileInfo **out_files);
Node* build_huffman_tree_f(uint64_t freq[256]);
void generate_codes_f(Node *node, char *buffer, int depth);
void free_tree_f(Node *node);

// Funciones de BitWriter y BitReader
void bw_init_f(BitWriter *w, FILE *f);
void bw_write_bit_f(BitWriter *w, int bit);
void bw_flush_f(BitWriter *w);
void br_init_f(BitReader *r, uint8_t *data, size_t size);
int br_read_bit_f(BitReader *r);

// Funciones para el MinHeap
MinHeap* heap_create_f(int cap);
void heap_push_f(MinHeap *h, Node *n);
Node* heap_pop_f(MinHeap *h);
void heap_swap_f(Node **a, Node **b);
Node* create_node_f(uint64_t freq, int symbol, Node *l, Node *r);

// Array global de códigos Huffman (índice: símbolo)
char *codes_f[256] = {0};

//----------------- Implementación de funciones auxiliares -----------------

// Escanea un directorio (no recursivo) y devuelve un arreglo de FileInfo.
int scan_directory_f(const char *dirpath, FileInfo **out_files) {
    DIR *dir = opendir(dirpath);
    if (!dir) { perror("opendir"); exit(EXIT_FAILURE); }
    struct dirent *entry;
    FileInfo *files = NULL;
    int count = 0;
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') continue;  // Ignorar "." y ".."
        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);
        struct stat st;
        if (stat(fullpath, &st) == 0 && S_ISREG(st.st_mode)) {
            files = realloc(files, sizeof(FileInfo) * (count + 1));
            files[count].name = strdup(entry->d_name);
            files[count].path = strdup(fullpath);
            files[count].orig_size = 0;
            files[count].bit_length = 0;
            count++;
        }
    }
    closedir(dir);
    *out_files = files;
    return count;
}

//----------------- Funciones del MinHeap -----------------

MinHeap* heap_create_f(int cap) {
    MinHeap *h = malloc(sizeof(MinHeap));
    h->data = malloc(sizeof(Node*) * cap);
    h->size = 0;
    h->capacity = cap;
    return h;
}

void heap_swap_f(Node **a, Node **b) {
    Node *t = *a;
    *a = *b;
    *b = t;
}

void heap_push_f(MinHeap *h, Node *n) {
    if (h->size >= h->capacity) {
        h->capacity *= 2;
        h->data = realloc(h->data, sizeof(Node*) * h->capacity);
    }
    int i = h->size++;
    h->data[i] = n;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (h->data[p]->freq <= h->data[i]->freq)
            break;
        heap_swap_f(&h->data[p], &h->data[i]);
        i = p;
    }
}

Node* heap_pop_f(MinHeap *h) {
    if (h->size == 0) return NULL;
    Node *root = h->data[0];
    h->data[0] = h->data[--h->size];
    int i = 0;
    while (1) {
        int l = 2 * i + 1, r = 2 * i + 2;
        int smallest = i;
        if (l < h->size && h->data[l]->freq < h->data[smallest]->freq)
            smallest = l;
        if (r < h->size && h->data[r]->freq < h->data[smallest]->freq)
            smallest = r;
        if (smallest == i)
            break;
        heap_swap_f(&h->data[i], &h->data[smallest]);
        i = smallest;
    }
    return root;
}

Node* create_node_f(uint64_t freq, int symbol, Node *l, Node *r) {
    Node *n = malloc(sizeof(Node));
    n->freq = freq;
    n->symbol = symbol;
    n->left = l;
    n->right = r;
    return n;
}

Node* build_huffman_tree_f(uint64_t freq[256]) {
    MinHeap *h = heap_create_f(256);
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            heap_push_f(h, create_node_f(freq[i], i, NULL, NULL));
        }
    }
    if (h->size == 0) {
        free(h->data);
        free(h);
        return NULL;
    }
    while (h->size > 1) {
        Node *a = heap_pop_f(h);
        Node *b = heap_pop_f(h);
        heap_push_f(h, create_node_f(a->freq + b->freq, -1, a, b));
    }
    Node *root = heap_pop_f(h);
    free(h->data);
    free(h);
    return root;
}

void generate_codes_f(Node *node, char *buffer, int depth) {
    if (!node) return;
    if (node->symbol != -1) {
        buffer[depth] = '\0';
        codes_f[(unsigned char)node->symbol] = strdup(buffer);
        return;
    }
    buffer[depth] = '0';
    generate_codes_f(node->left, buffer, depth + 1);
    buffer[depth] = '1';
    generate_codes_f(node->right, buffer, depth + 1);
}

void free_tree_f(Node *node) {
    if (!node) return;
    free_tree_f(node->left);
    free_tree_f(node->right);
    free(node);
}

//----------------- BitWriter y BitReader -----------------

void bw_init_f(BitWriter *w, FILE *f) {
    w->f = f;
    w->buf = 0;
    w->bit_pos = 0;
}

void bw_write_bit_f(BitWriter *w, int bit) {
    w->buf |= (bit & 1) << (7 - w->bit_pos);
    w->bit_pos++;
    if (w->bit_pos == 8) {
        fwrite(&w->buf, 1, 1, w->f);
        w->buf = 0;
        w->bit_pos = 0;
    }
}

void bw_flush_f(BitWriter *w) {
    if (w->bit_pos > 0) {
        fwrite(&w->buf, 1, 1, w->f);
        w->buf = 0;
        w->bit_pos = 0;
    }
}

void br_init_f(BitReader *r, uint8_t *data, size_t size) {
    r->data = data;
    r->size = size;
    r->idx = 0;
    r->bit_pos = 0;
}

int br_read_bit_f(BitReader *r) {
    if (r->idx >= r->size) return -1;
    int bit = (r->data[r->idx] >> (7 - r->bit_pos)) & 1;
    r->bit_pos++;
    if (r->bit_pos == 8) {
        r->bit_pos = 0;
        r->idx++;
    }
    return bit;
}

//----------------- Función de Compresión (paralelizada) -----------------

void compress_directory_fork(const char *dirpath, const char *outpath) {
    FileInfo *files;
    int nfiles = scan_directory_f(dirpath, &files);
    if (nfiles == 0) {
        fprintf(stderr, "No se encontraron archivos en '%s'\n", dirpath);
        exit(EXIT_FAILURE);
    }

    uint64_t freq[256] = {0};
    // Fase 1: Contar frecuencias globales y obtener el tamaño original de cada archivo.
    for (int i = 0; i < nfiles; i++) {
        FILE *f = fopen(files[i].path, "rb");
        if (!f) { perror(files[i].path); exit(EXIT_FAILURE); }
        int c;
        while ((c = fgetc(f)) != EOF) {
            freq[(unsigned char)c]++;
            files[i].orig_size++;
        }
        fclose(f);
    }

    // Construir árbol de Huffman y generar códigos.
    Node *root = build_huffman_tree_f(freq);
    if (!root) { fprintf(stderr, "Error: árbol vacío.\n"); exit(EXIT_FAILURE); }
    char buffer[256];
    generate_codes_f(root, buffer, 0);

    // Fase 2: Calcular la longitud en bits de la compresión de cada archivo.
    for (int i = 0; i < nfiles; i++) {
        FILE *f = fopen(files[i].path, "rb");
        if (!f) { perror(files[i].path); exit(EXIT_FAILURE); }
        int c;
        while ((c = fgetc(f)) != EOF) {
            files[i].bit_length += strlen(codes_f[(unsigned char)c]);
        }
        fclose(f);
    }

    // Fase 3: Escribir el encabezado global en el archivo de salida.
    FILE *out = fopen(outpath, "wb");
    if (!out) { perror(outpath); exit(EXIT_FAILURE); }
    fwrite(MAGIC, 1, 4, out);
    uint16_t unique = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) unique++;
    }
    fwrite(&unique, sizeof(unique), 1, out);
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            uint8_t sym = i;
            fwrite(&sym, 1, 1, out);
            fwrite(&freq[i], sizeof(uint64_t), 1, out);
        }
    }
    uint32_t m = nfiles;
    fwrite(&m, sizeof(uint32_t), 1, out);
    for (int i = 0; i < nfiles; i++) {
        uint16_t namelen = strlen(files[i].name);
        fwrite(&namelen, sizeof(namelen), 1, out);
        fwrite(files[i].name, 1, namelen, out);
        fwrite(&files[i].orig_size, sizeof(uint64_t), 1, out);
        fwrite(&files[i].bit_length, sizeof(uint64_t), 1, out);
    }
    fclose(out);

    // Fase 4: Forquear un proceso hijo por cada archivo para comprimirlo y escribirlo en un archivo temporal.
    for (int i = 0; i < nfiles; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Proceso hijo: comprimir el archivo i.
            FILE *fin = fopen(files[i].path, "rb");
            if (!fin) { perror(files[i].path); exit(EXIT_FAILURE); }
            char temp_filename[PATH_MAX];
            snprintf(temp_filename, sizeof(temp_filename), "temp_%d.bin", i);
            FILE *fout = fopen(temp_filename, "wb");
            if (!fout) { perror(temp_filename); exit(EXIT_FAILURE); }
            BitWriter bw;
            bw_init_f(&bw, fout);
            int c;
            while ((c = fgetc(fin)) != EOF) {
                char *code = codes_f[(unsigned char)c];
                for (int k = 0; code[k]; k++) {
                    bw_write_bit_f(&bw, code[k] - '0');
                }
            }
            bw_flush_f(&bw);
            fclose(fin);
            fclose(fout);
            // El hijo termina aquí.
            exit(EXIT_SUCCESS);
        }
    }
    // El padre espera a que terminen todos los hijos.
    for (int i = 0; i < nfiles; i++) {
        wait(NULL);
    }

    // Fase 5: Unir los archivos temporales en el archivo final.
    out = fopen(outpath, "ab");
    if (!out) { perror(outpath); exit(EXIT_FAILURE); }
    for (int i = 0; i < nfiles; i++) {
        char temp_filename[PATH_MAX];
        snprintf(temp_filename, sizeof(temp_filename), "temp_%d.bin", i);
        FILE *ftemp = fopen(temp_filename, "rb");
        if (!ftemp) { perror(temp_filename); exit(EXIT_FAILURE); }
        // Leer el contenido del archivo temporal y escribirlo en el archivo final.
        uint8_t buffer[4096];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), ftemp)) > 0) {
            fwrite(buffer, 1, bytes, out);
        }
        fclose(ftemp);
        remove(temp_filename);
    }
    fclose(out);

    // Liberar la memoria.
    for (int i = 0; i < 256; i++) {
        if (codes_f[i]) free(codes_f[i]);
    }
    free_tree_f(root);
    for (int i = 0; i < nfiles; i++) {
        free(files[i].name);
        free(files[i].path);
    }
    free(files);
    printf("Compresión completada: %u archivos -> %s\n", m, outpath);
}

//----------------- Función de Descompresión (con fork por archivo) -----------------

void decompress_archive_fork(const char *inpath, const char *outdir) {
    FILE *in = fopen(inpath, "rb");
    if (!in) { perror(inpath); exit(EXIT_FAILURE); }
    char magic[4];
    if (fread(magic, 1, 4, in) != 4) {
        fprintf(stderr, "Error al leer MAGIC.\n");
        exit(EXIT_FAILURE);
    }
    if (memcmp(magic, MAGIC, 4) != 0) {
        fprintf(stderr, "Formato inválido: MAGIC no coincide.\n");
        exit(EXIT_FAILURE);
    }
    uint16_t unique;
    fread(&unique, sizeof(unique), 1, in);
    uint64_t freq[256] = {0};
    for (int i = 0; i < unique; i++) {
        uint8_t sym;
        fread(&sym, 1, 1, in);
        fread(&freq[sym], sizeof(uint64_t), 1, in);
    }
    Node *root = build_huffman_tree_f(freq);
    if (!root) { fprintf(stderr, "Error: árbol vacío en descompresión.\n"); exit(EXIT_FAILURE); }
    uint32_t m;
    fread(&m, sizeof(uint32_t), 1, in);
    FileInfo *files = malloc(sizeof(FileInfo) * m);
    for (uint32_t i = 0; i < m; i++) {
        uint16_t namelen;
        fread(&namelen, sizeof(namelen), 1, in);
        files[i].name = malloc(namelen + 1);
        fread(files[i].name, 1, namelen, in);
        files[i].name[namelen] = '\0';
        fread(&files[i].orig_size, sizeof(uint64_t), 1, in);
        fread(&files[i].bit_length, sizeof(uint64_t), 1, in);
    }
    // Crear directorio destino si no existe.
    struct stat st = {0};
    if (stat(outdir, &st) == -1) {
        if (mkdir(outdir, 0755) == -1) { perror("mkdir"); exit(EXIT_FAILURE); }
    }
    // Para cada archivo, crear un proceso hijo que descomprima su bloque.
    for (uint32_t i = 0; i < m; i++) {
        uint64_t bits = files[i].bit_length;
        uint64_t bytes_to_read = (bits + 7) / 8;
        uint8_t *buffer = malloc(bytes_to_read);
        if (fread(buffer, 1, bytes_to_read, in) != bytes_to_read) {
            fprintf(stderr, "Error al leer datos comprimidos para %s\n", files[i].name);
            exit(EXIT_FAILURE);
        }
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Proceso hijo: descomprimir este archivo.
            char outpath_file[PATH_MAX];
            snprintf(outpath_file, sizeof(outpath_file), "%s/%s", outdir, files[i].name);
            FILE *fout = fopen(outpath_file, "wb");
            if (!fout) { perror(outpath_file); exit(EXIT_FAILURE); }
            BitReader br;
            br_init_f(&br, buffer, bytes_to_read);
            Node *curr = root;
            uint64_t written = 0;
            while (written < files[i].orig_size) {
                int bit = br_read_bit_f(&br);
                if (bit < 0) break;
                curr = bit ? curr->right : curr->left;
                if (!curr->left && !curr->right) {
                    fputc(curr->symbol, fout);
                    written++;
                    curr = root;
                }
            }
            fclose(fout);
            free(buffer);
            //printf("[PID %d] Descomprimido: %s -> %s (bytes: %lu)\n", getpid(), files[i].name, outpath_file, files[i].orig_size);
            free(files[i].name);
            exit(EXIT_SUCCESS);
        } else {
            free(buffer);
        }
    }
    // Esperar a que todos los hijos terminen.
    for (uint32_t i = 0; i < m; i++) {
        wait(NULL);
    }
    for (uint32_t i = 0; i < m; i++) {
        free(files[i].name);
    }
    free(files);
    free_tree_f(root);
    fclose(in);
    printf("Descompresión completada en directorio: %s\n", outdir);
}

//----------------- Main -----------------

void EjecutarHuffmanFork(int argc, char *argv[]) {
    if (argc != 4 || (strcmp(argv[1], "-c") && strcmp(argv[1], "-d"))) {
        fprintf(stderr, "Uso:\n  Para comprimir:   %s -c <dir_txt> <salida.huf>\n  Para descomprimir: %s -d <entrada.huf> <dir_destino>\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);  // Usa exit en lugar de return
    }

    if (strcmp(argv[1], "-c") == 0) {
        //printf("Si ves esto, se llama al fork");
        iniciarMedicionT();  // Inicia la medición del tiempo
        compress_directory_fork(argv[2], argv[3]);  // Llama a la función de compresión
        finalizarMedicionT();  // Finaliza la medición del tiempo
    } else {
        //printf("Si ves esto, se llama al fork");
        iniciarMedicionT();  // Inicia la medición del tiempo
        decompress_archive_fork(argv[2], argv[3]);  // Llama a la función de descompresión
        finalizarMedicionT();  // Finaliza la medición del tiempo
    }

    exit(EXIT_SUCCESS);  // Usa exit para indicar que el programa terminó con éxito
}
