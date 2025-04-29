/*
 * huffmanPthread.c
 *
 * Implementación paralela (usando pthreads) del algoritmo de Huffman para comprimir y descomprimir
 * todos los archivos de un directorio. Uso desde línea de comandos:
 *
 *   Para comprimir:   ./huffmanParallel -c <directorio_txt> <archivo_salida.huf>
 *   Para descomprimir: ./huffmanParallel -d <archivo_entrada.huf> <directorio_destino>
 *
 * Este ejemplo integra:
 *  - Dos fases en compresión: conteo de frecuencias en paralelo y compresión en paralelo.
 *  - Descompresión paralela (cada archivo se procesa en un hilo).
 *
 * Compilar con: gcc -pthread -o huffmanParallel huffmanParallel.c
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "medirTiempo.h"
#include "arbolHuffman.h"

#define MAGIC "HUF\0"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/*

 // Nodo para el árbol de Huffman
 typedef struct Node {
    uint64_t freq;
    int symbol;
    struct Node *left, *right;
 } Node;

 // Información sobre un archivo a comprimir/descomprimir
 typedef struct {
    char *name;         // Nombre del archivo
    char *path;         // Ruta completa del archivo
    uint64_t orig_size; // Tamaño original en bytes
    uint64_t bit_length;// Cantidad total de bits en el bloque comprimido
 } FileInfo;


 typedef struct {
    Node **data;
    int size;
    int capacity;
} MinHeap;

// -------------------------------
// BitReader (para descompresión)
// -------------------------------
typedef struct {
    uint8_t *data;
    size_t size;
    size_t idx;
    int bit_pos;
} BitReader;


*/


// Arreglo global de códigos Huffman (generado después de construir el árbol)
char *codes_p[256] = {0};

// -------------------------------
// Memoria BitWriter (para compresión en memoria)
// -------------------------------
typedef struct {
    uint8_t *buffer;   // Buffer dinámico donde se acumulan los bytes
    size_t capacity;   // Capacidad actual del buffer
    size_t size;       // Cantidad de bytes escritos
    uint8_t buf;       // Byte actual en construcción
    int bit_pos;       // Posición actual en el byte (0 a 7)
    size_t total_bits; // Total de bits escritos (útil para conocer la longitud precisa)
} MemBitWriter;

void mbw_init(MemBitWriter *w) {
    w->capacity = 1024;
    w->buffer = malloc(w->capacity);
    w->size = 0;
    w->buf = 0;
    w->bit_pos = 0;
    w->total_bits = 0;
}

void mbw_write_bit(MemBitWriter *w, int bit) {
    w->buf |= (bit & 1) << (7 - w->bit_pos);
    w->bit_pos++;
    w->total_bits++;
    if (w->bit_pos == 8) {
        if (w->size + 1 > w->capacity) {
            w->capacity *= 2;
            w->buffer = realloc(w->buffer, w->capacity);
        }
        w->buffer[w->size++] = w->buf;
        w->buf = 0;
        w->bit_pos = 0;
    }
}

void mbw_flush(MemBitWriter *w) {
    if (w->bit_pos > 0) {
        if (w->size + 1 > w->capacity) {
            w->capacity *= 2;
            w->buffer = realloc(w->buffer, w->capacity);
        }
        w->buffer[w->size++] = w->buf;
        w->buf = 0;
        w->bit_pos = 0;
    }
}


void br_init_p(BitReader *r, uint8_t *data, size_t size) {
    r->data = data;
    r->size = size;
    r->idx = 0;
    r->bit_pos = 0;
}

int br_read_bit_p(BitReader *r) {
    if (r->idx >= r->size) return -1;
    int bit = (r->data[r->idx] >> (7 - r->bit_pos)) & 1;
    r->bit_pos++;
    if (r->bit_pos == 8) {
        r->bit_pos = 0;
        r->idx++;
    }
    return bit;
}

// -------------------------------
// Montículo Mínimo para el Árbol de Huffman
// -------------------------------


MinHeap* heap_create_p(int cap) {
    MinHeap *h = malloc(sizeof(MinHeap));
    h->data = malloc(sizeof(Node*) * cap);
    h->size = 0;
    h->capacity = cap;
    return h;
}

void heap_swap_p(Node **a, Node **b) {
    Node *temp = *a;
    *a = *b;
    *b = temp;
}

void heap_push_p(MinHeap *h, Node *n) {
    if (h->size >= h->capacity) {
        h->capacity *= 2;
        h->data = realloc(h->data, sizeof(Node*) * h->capacity);
    }
    int i = h->size++;
    h->data[i] = n;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (h->data[p]->freq <= h->data[i]->freq) break;
        heap_swap_p(&h->data[p], &h->data[i]);
        i = p;
    }
}

Node* heap_pop_p(MinHeap *h) {
    if (h->size == 0) return NULL;
    Node *root = h->data[0];
    h->data[0] = h->data[--h->size];
    int i = 0;
    while (1) {
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        int smallest = i;
        if (l < h->size && h->data[l]->freq < h->data[smallest]->freq)
            smallest = l;
        if (r < h->size && h->data[r]->freq < h->data[smallest]->freq)
            smallest = r;
        if (smallest == i) break;
        heap_swap_p(&h->data[i], &h->data[smallest]);
        i = smallest;
    }
    return root;
}

Node* create_node_p(uint64_t freq, int symbol, Node *l, Node *r) {
    Node *n = malloc(sizeof(Node));
    n->freq = freq;
    n->symbol = symbol;
    n->left = l;
    n->right = r;
    return n;
}

Node* build_huffman_tree_p(uint64_t freq[256]) {
    MinHeap *h = heap_create_p(256);
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            heap_push_p(h, create_node_p(freq[i], i, NULL, NULL));
        }
    }
    if (h->size == 0) { free(h->data); free(h); return NULL; }
    while (h->size > 1) {
        Node *a = heap_pop_p(h);
        Node *b = heap_pop_p(h);
        heap_push_p(h, create_node_p(a->freq + b->freq, -1, a, b));
    }
    Node *root = heap_pop_p(h);
    free(h->data);
    free(h);
    return root;
}

void free_tree_p(Node *node) {
    if (!node) return;
    free_tree_p(node->left);
    free_tree_p(node->right);
    free(node);
}

// -------------------------------
// Generación de Códigos Huffman
// -------------------------------
void generate_codes_p(Node *node, char *buffer, int depth) {
    if (!node) return;
    if (node->symbol != -1) {
        buffer[depth] = '\0';
        codes_p[(unsigned char)node->symbol] = strdup(buffer);
        return;
    }
    buffer[depth] = '0';
    generate_codes_p(node->left, buffer, depth + 1);
    buffer[depth] = '1';
    generate_codes_p(node->right, buffer, depth + 1);
}

// -------------------------------
// Escaneo del Directorio (no recursivo)
// -------------------------------
int scan_directory_p(const char *dirpath, FileInfo **out_files) {
    DIR *dir = opendir(dirpath);
    if (!dir) { perror("opendir"); exit(EXIT_FAILURE); }
    struct dirent *entry;
    FileInfo *files = NULL;
    int count = 0;
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') continue; // ignora . y ..
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

/* ----------------------------------------
 *   FASE 1: CONTEO DE FRECUENCIAS (PARALELO)
 * -----------------------------------------*/

// Argumento para cada hilo: contador de frecuencia de un archivo
typedef struct {
    FileInfo *fi;
    uint64_t freq[256]; // Frecuencias locales
} CompressFreqArg;

void* compression_freq_thread(void *arg) {
    CompressFreqArg *cfa = (CompressFreqArg*)arg;
    FILE *f = fopen(cfa->fi->path, "rb");
    if (!f) { perror(cfa->fi->path); pthread_exit(NULL); }
    int c;
    cfa->fi->orig_size = 0;
    memset(cfa->freq, 0, sizeof(cfa->freq));
    while ((c = fgetc(f)) != EOF) {
        cfa->freq[(unsigned char)c]++;
        cfa->fi->orig_size++;
    }
    fclose(f);
    pthread_exit(NULL);
}

/* ----------------------------------------
 *   FASE 2: COMPRESIÓN DE CADA ARCHIVO (PARALELO)
 * -----------------------------------------*/

// Argumento para cada hilo de compresión de archivo
typedef struct {
    FileInfo *fi;
    uint8_t *compBuffer; // Buffer con datos comprimidos
    size_t compSize;     // Tamaño en bytes del buffer
    size_t totalBits;    // Total de bits escritos (bit_length)
} CompressDataArg;

void* compress_file_thread(void *arg) {
    CompressDataArg *cda = (CompressDataArg*)arg;
    FILE *f = fopen(cda->fi->path, "rb");
    if (!f) { perror(cda->fi->path); pthread_exit(NULL); }
    MemBitWriter mbw;
    mbw_init(&mbw);
    int c;
    while ((c = fgetc(f)) != EOF) {
        char *code = codes_p[(unsigned char)c];
        for (int i = 0; code[i] != '\0'; i++) {
            mbw_write_bit(&mbw, code[i] - '0');
        }
    }
    mbw_flush(&mbw);
    fclose(f);
    cda->compBuffer = mbw.buffer;
    cda->compSize = mbw.size;
    cda->totalBits = mbw.total_bits;
    pthread_exit(NULL);
}

/* ----------------------------------------
 *   DESCOMPRESIÓN DE CADA ARCHIVO (PARALELO)
 * -----------------------------------------*/

// Argumento para cada hilo de descompresión
typedef struct {
    FileInfo fi;
    uint8_t *compBuffer;
    size_t compSize;
    Node *huffman_root;
    char outdir[PATH_MAX];
} DecompressDataArg;

void* decompress_file_thread(void *arg) {
    DecompressDataArg *dda = (DecompressDataArg*)arg;
    char outpath[PATH_MAX];
    snprintf(outpath, sizeof(outpath), "%s/%s", dda->outdir, dda->fi.name);
    FILE *fout = fopen(outpath, "wb");
    if (!fout) { perror(outpath); pthread_exit(NULL); }
    BitReader br;
    br_init_p(&br, dda->compBuffer, dda->compSize);
    Node *curr = dda->huffman_root;
    uint64_t written = 0;
    while (written < dda->fi.orig_size) {
        int bit = br_read_bit_p(&br);
        if (bit < 0) break;
        curr = bit ? curr->right : curr->left;
        if (!curr->left && !curr->right) {
            fputc(curr->symbol, fout);
            written++;
            curr = dda->huffman_root;
        }
    }
    fclose(fout);
    pthread_exit(NULL);
}

/* ----------------------------------------
 *   FUNCIÓN PRINCIPAL DE COMPRESIÓN (PARALELO)
 * -----------------------------------------*/
void compress_directory_parallel(const char *dirpath, const char *outpath) {
    FileInfo *files = NULL;
    int nfiles = scan_directory_p(dirpath, &files);
    if (nfiles == 0) {
        fprintf(stderr, "No se encontraron archivos en '%s'\n", dirpath);
        exit(EXIT_FAILURE);
    }
    // Fase 1: Conteo de frecuencias en paralelo
    pthread_t *freq_threads = malloc(sizeof(pthread_t) * nfiles);
    CompressFreqArg *freq_args = malloc(sizeof(CompressFreqArg) * nfiles);
    for (int i = 0; i < nfiles; i++) {
        freq_args[i].fi = &files[i];
        memset(freq_args[i].freq, 0, sizeof(freq_args[i].freq));
        pthread_create(&freq_threads[i], NULL, compression_freq_thread, &freq_args[i]);
    }
    for (int i = 0; i < nfiles; i++) {
        pthread_join(freq_threads[i], NULL);
    }
    free(freq_threads);

    // Combinar las frecuencias locales en un arreglo global
    uint64_t global_freq[256] = {0};
    for (int i = 0; i < nfiles; i++) {
        for (int j = 0; j < 256; j++) {
            global_freq[j] += freq_args[i].freq[j];
        }
    }
    free(freq_args);

    // Construir el árbol de Huffman y generar los códigos
    Node *root = build_huffman_tree_p(global_freq);
    if (!root) {
        fprintf(stderr, "Error: árbol de Huffman vacío.\n");
        exit(EXIT_FAILURE);
    }
    char codebuffer[256];
    generate_codes_p(root, codebuffer, 0);

    // Fase 2: Compresión de cada archivo en paralelo
    pthread_t *comp_threads = malloc(sizeof(pthread_t) * nfiles);
    CompressDataArg *comp_args = malloc(sizeof(CompressDataArg) * nfiles);
    for (int i = 0; i < nfiles; i++) {
        comp_args[i].fi = &files[i];
        comp_args[i].compBuffer = NULL;
        comp_args[i].compSize = 0;
        comp_args[i].totalBits = 0;
        pthread_create(&comp_threads[i], NULL, compress_file_thread, &comp_args[i]);
    }
    for (int i = 0; i < nfiles; i++) {
        pthread_join(comp_threads[i], NULL);
        files[i].bit_length = comp_args[i].totalBits;
    }
    free(comp_threads);

    // Escribir archivo de salida con encabezado y datos comprimidos
    FILE *out = fopen(outpath, "wb");
    if (!out) { perror(outpath); exit(EXIT_FAILURE); }
    fwrite(MAGIC, 1, 4, out);
    uint16_t unique = 0;
    for (int i = 0; i < 256; i++) {
        if (global_freq[i] > 0) unique++;
    }
    fwrite(&unique, sizeof(unique), 1, out);
    for (int i = 0; i < 256; i++) {
        if (global_freq[i] > 0) {
            uint8_t sym = i;
            fwrite(&sym, 1, 1, out);
            fwrite(&global_freq[i], sizeof(uint64_t), 1, out);
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
    for (int i = 0; i < nfiles; i++) {
        fwrite(comp_args[i].compBuffer, 1, comp_args[i].compSize, out);
        free(comp_args[i].compBuffer);
    }
    free(comp_args);
    fclose(out);
    printf("Compresión completada: %d archivos -> %s\n", nfiles, outpath);

    // Liberar recursos
    free_tree_p(root);
    for (int i = 0; i < nfiles; i++) {
        free(files[i].name);
        free(files[i].path);
    }
    free(files);
    for (int i = 0; i < 256; i++) {
        if (codes_p[i]) { free(codes_p[i]); codes_p[i] = NULL; }
    }
}

/* ----------------------------------------
 *   FUNCIÓN PRINCIPAL DE DESCOMPRESIÓN (PARALELO)
 * -----------------------------------------*/
void decompress_archive_parallel(const char *inpath, const char *outdir) {
    FILE *in = fopen(inpath, "rb");
    if (!in) { perror(inpath); exit(EXIT_FAILURE); }
    char magic[4];
    fread(magic, 1, 4, in);
    if (memcmp(magic, MAGIC, 4) != 0) {
        fprintf(stderr, "Formato inválido: MAGIC no coincide.\n");
        exit(EXIT_FAILURE);
    }
    uint16_t unique;
    fread(&unique, sizeof(unique), 1, in);
    uint64_t global_freq[256] = {0};
    for (int i = 0; i < unique; i++) {
        uint8_t sym;
        fread(&sym, 1, 1, in);
        fread(&global_freq[sym], sizeof(uint64_t), 1, in);
    }
    Node *root = build_huffman_tree_p(global_freq);
    if (!root) {
        fprintf(stderr, "Error: árbol de Huffman vacío en descompresión.\n");
        exit(EXIT_FAILURE);
    }
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
    // Para cada archivo, leer su bloque comprimido y lanzar un hilo
    pthread_t *d_threads = malloc(sizeof(pthread_t) * m);
    typedef struct {
        FileInfo fi;
        uint8_t *compBuffer;
        size_t compSize;
        Node *huffman_root;
        char outdir[PATH_MAX];
    } DecompressDataArg;  // (definido anteriormente)

    DecompressDataArg *d_args = malloc(sizeof(DecompressDataArg) * m);
    for (uint32_t i = 0; i < m; i++) {
        uint64_t bits = files[i].bit_length;
        uint64_t bytes_to_read = (bits + 7) / 8;
        uint8_t *buffer = malloc(bytes_to_read);
        fread(buffer, 1, bytes_to_read, in);
        d_args[i].fi = files[i];  // copia de la estructura
        d_args[i].compBuffer = buffer;
        d_args[i].compSize = bytes_to_read;
        d_args[i].huffman_root = root;
        strncpy(d_args[i].outdir, outdir, PATH_MAX);
        pthread_create(&d_threads[i], NULL, decompress_file_thread, &d_args[i]);
    }
    for (uint32_t i = 0; i < m; i++) {
        pthread_join(d_threads[i], NULL);
        free(d_args[i].compBuffer);
        free(files[i].name);
    }
    free(d_threads);
    free(d_args);
    free(files);
    free_tree_p(root);
    fclose(in);
    printf("Descompresión completada en el directorio: %s\n", outdir);
}

/* ----------------------------------------
 *   FUNCIÓN MAIN: Selección de Modo -c y -d
 * -----------------------------------------*/
void EjecutarHuffmanPthread(int argc, char *argv[]) {
    if (argc != 4 || (strcmp(argv[1], "-c") && strcmp(argv[1], "-d"))) {
        fprintf(stderr, "Uso:\n  Para comprimir:   %s -c <dir_txt> <salida.huf>\n  Para descomprimir: %s -d <entrada.huf> <dir_destino>\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);  // Usa exit en lugar de return
    }

    if (strcmp(argv[1], "-c") == 0) {
        iniciarMedicionT();  // Inicia la medición del tiempo
        compress_directory_parallel(argv[2], argv[3]);  // Llama a la función de compresión
        finalizarMedicionT();  // Finaliza la medición del tiempo
    } else {
        iniciarMedicionT();  // Inicia la medición del tiempo
        decompress_archive_parallel(argv[2], argv[3]);  // Llama a la función de descompresión
        finalizarMedicionT();  // Finaliza la medición del tiempo
    }

    exit(EXIT_SUCCESS);  // Usa exit para indicar que el programa terminó con éxito
}
