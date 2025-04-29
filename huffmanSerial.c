// huffmanerial.c
// Implementación serial en C del algoritmo de Huffman para comprimir y descomprimir todos los archivos de texto en un directorio.
// Uso:
//   Para comprimir:   ./huffman -c <directorio_txt> <archivo_salida.huf>
//   Para descomprimir: ./huffman -d <archivo_entrada.huf> <directorio_destino>

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

#include "arbolHuffman.h"
#include "medirTiempo.h"

#define MAGIC "HUF\0"
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// ----------------------- Estructuras para Huffman -----------------------

/*

typedef struct Node {
    uint64_t freq;
    int symbol;          // 0-255 para hoja, -1 para nodo interno
    struct Node *left;
    struct Node *right;
} Node;

// Información de cada archivo
typedef struct {
    char *name;
    char *path;
    uint64_t orig_size;
    uint64_t bit_length;
} FileInfo;

// Montículo mínimo simple
typedef struct {
    Node **data;
    int size;
    int capacity;
} MinHeap;

// Escribe bits en un buffer de salida
typedef struct {
    FILE *f;
    uint8_t buf;
    int bit_pos;  // [0..7]
} BitWriter;

// Lee bits desde un buffer en memoria
typedef struct {
    uint8_t *data;
    size_t size;
    size_t idx;
    int bit_pos;
} BitReader;


*/


// Código Huffman por símbolo
char *codes[256];

// Prototipos
MinHeap* heap_create(int cap);
void heap_push(MinHeap *h, Node *n);
Node* heap_pop(MinHeap *h);
Node* build_huffman_tree(uint64_t freq[256]);
void generate_codes(Node *node, char *buffer, int depth);
void free_tree(Node *node);

// Lee todos los archivos regulares en un directorio (no recursivo)
int scan_directory(const char *dirpath, FileInfo **out_files) {
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
            files = realloc(files, sizeof(FileInfo) * (count+1));
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

// Montículo mínimo de nodos
MinHeap* heap_create(int cap) {
    MinHeap *h = malloc(sizeof(MinHeap));
    h->data = malloc(sizeof(Node*) * cap);
    h->size = 0;
    h->capacity = cap;
    return h;
}

void heap_swap(Node **a, Node **b) { Node *t = *a; *a = *b; *b = t; }

void heap_push(MinHeap *h, Node *n) {
    if (h->size >= h->capacity) {
        h->capacity *= 2;
        h->data = realloc(h->data, sizeof(Node*) * h->capacity);
    }
    int i = h->size++;
    h->data[i] = n;
    // sift up
    while (i > 0) {
        int p = (i - 1) / 2;
        if (h->data[p]->freq <= h->data[i]->freq) break;
        heap_swap(&h->data[p], &h->data[i]);
        i = p;
    }
}

Node* heap_pop(MinHeap *h) {
    if (h->size == 0) return NULL;
    Node *root = h->data[0];
    h->data[0] = h->data[--h->size];
    // sift down
    int i = 0;
    while (1) {
        int l = 2*i + 1;
        int r = 2*i + 2;
        int smallest = i;
        if (l < h->size && h->data[l]->freq < h->data[smallest]->freq)
            smallest = l;
        if (r < h->size && h->data[r]->freq < h->data[smallest]->freq)
            smallest = r;
        if (smallest == i) break;
        heap_swap(&h->data[i], &h->data[smallest]);
        i = smallest;
    }
    return root;
}

Node* create_node(uint64_t freq, int symbol, Node *l, Node *r) {
    Node *n = malloc(sizeof(Node));
    n->freq = freq;
    n->symbol = symbol;
    n->left = l;
    n->right = r;
    return n;
}

Node* build_huffman_tree(uint64_t freq[256]) {
    MinHeap *h = heap_create(256);
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            heap_push(h, create_node(freq[i], i, NULL, NULL));
        }
    }
    if (h->size == 0) return NULL;
    while (h->size > 1) {
        Node *a = heap_pop(h);
        Node *b = heap_pop(h);
        heap_push(h, create_node(a->freq + b->freq, -1, a, b));
    }
    Node *root = heap_pop(h);
    free(h->data);
    free(h);
    return root;
}

void generate_codes(Node *node, char *buffer, int depth) {
    if (!node) return;
    if (node->symbol != -1) {
        buffer[depth] = '\0';
        codes[(unsigned char)node->symbol] = strdup(buffer);
        return;
    }
    // izquierda = '0'
    buffer[depth] = '0';
    generate_codes(node->left, buffer, depth+1);
    // derecha = '1'
    buffer[depth] = '1';
    generate_codes(node->right, buffer, depth+1);
}

void free_tree(Node *node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}



void bw_init(BitWriter *w, FILE *f) {
    w->f = f;
    w->buf = 0;
    w->bit_pos = 0;
}

void bw_write_bit(BitWriter *w, int bit) {
    w->buf |= (bit & 1) << (7 - w->bit_pos);
    w->bit_pos++;
    if (w->bit_pos == 8) {
        fwrite(&w->buf, 1, 1, w->f);
        w->buf = 0;
        w->bit_pos = 0;
    }
}

void bw_flush(BitWriter *w) {
    if (w->bit_pos > 0) {
        fwrite(&w->buf, 1, 1, w->f);
        w->buf = 0;
        w->bit_pos = 0;
    }
}


void br_init(BitReader *r, uint8_t *data, size_t size) {
    r->data = data;
    r->size = size;
    r->idx = 0;
    r->bit_pos = 0;
}

int br_read_bit(BitReader *r) {
    if (r->idx >= r->size) return -1;
    int bit = (r->data[r->idx] >> (7 - r->bit_pos)) & 1;
    r->bit_pos++;
    if (r->bit_pos == 8) {
        r->bit_pos = 0;
        r->idx++;
    }
    return bit;
}

// FUNCIONES PRINCIPALES: compresión y descompresión

void compress_directory(const char *dirpath, const char *outpath) {
    FileInfo *files;
    int nfiles = scan_directory(dirpath, &files);
    if (nfiles == 0) {
        fprintf(stderr, "No se encontraron archivos en '%s'\n", dirpath);
        exit(EXIT_FAILURE);
    }
    // 1) Conteo de frecuencias y tamaños
    uint64_t freq[256] = {0};
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
    // 2) Construir árbol y códigos
    Node *root = build_huffman_tree(freq);
    if (!root) { fprintf(stderr, "Error: árbol vacío.\n"); exit(EXIT_FAILURE); }
    char buffer[256];
    generate_codes(root, buffer, 0);
    // 3) Calcular longitudes de bits por archivo
    for (int i = 0; i < nfiles; i++) {
        FILE *f = fopen(files[i].path, "rb");
        if (!f) { perror(files[i].path); exit(EXIT_FAILURE); }
        int c;
        while ((c = fgetc(f)) != EOF) {
            files[i].bit_length += strlen(codes[(unsigned char)c]);
        }
        fclose(f);
    }
    // 4) Escribir archivo de salida
    FILE *out = fopen(outpath, "wb");
    if (!out) { perror(outpath); exit(EXIT_FAILURE); }
    // Encabezado: MAGIC
    fwrite(MAGIC, 1, 4, out);
    // Símbolos no nulos
    uint16_t unique = 0;
    for (int i = 0; i < 256; i++) if (freq[i] > 0) unique++;
    fwrite(&unique, sizeof(unique), 1, out);
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            uint8_t sym = i;
            fwrite(&sym, 1, 1, out);
            fwrite(&freq[i], sizeof(uint64_t), 1, out);
        }
    }
    // Número de archivos
    uint32_t m = nfiles;
    fwrite(&m, sizeof(uint32_t), 1, out);
    // Encabezado por archivo
    for (int i = 0; i < nfiles; i++) {
        uint16_t namelen = strlen(files[i].name);
        fwrite(&namelen, sizeof(namelen), 1, out);
        fwrite(files[i].name, 1, namelen, out);
        fwrite(&files[i].orig_size, sizeof(uint64_t), 1, out);
        fwrite(&files[i].bit_length, sizeof(uint64_t), 1, out);
    }
    // Datos comprimidos por archivo
    BitWriter bw;
    bw_init(&bw, out);
    for (int i = 0; i < nfiles; i++) {
        FILE *f = fopen(files[i].path, "rb");
        if (!f) { perror(files[i].path); exit(EXIT_FAILURE); }
        int c;
        while ((c = fgetc(f)) != EOF) {
            char *code = codes[(unsigned char)c];
            for (int k = 0; code[k]; k++) {
                bw_write_bit(&bw, code[k] - '0');
            }
        }
        bw_flush(&bw);
        fclose(f);
    }
    fclose(out);
    // Liberar memoria
    for (int i = 0; i < 256; i++) if (codes[i]) free(codes[i]);
    free_tree(root);
    for (int i = 0; i < nfiles; i++) {
        free(files[i].name);
        free(files[i].path);
    }
    free(files);
    printf("Compresión completada: %u archivos -> %s\n", m, outpath);
}

void decompress_archive(const char *inpath, const char *outdir) {
    FILE *in = fopen(inpath, "rb");
    if (!in) { perror(inpath); exit(EXIT_FAILURE); }
    // Leer y validar MAGIC
    char magic[4];
    fread(magic,1,4,in);
    if (memcmp(magic, MAGIC, 4) != 0) {
        fprintf(stderr, "Formato inválido: MAGIC no coincide.\n"); exit(EXIT_FAILURE);
    }
    // Leer símbolos y frecuencias
    uint16_t unique;
    fread(&unique, sizeof(unique), 1, in);
    uint64_t freq[256] = {0};
    for (int i = 0; i < unique; i++) {
        uint8_t sym;
        fread(&sym,1,1,in);
        fread(&freq[sym], sizeof(uint64_t), 1, in);
    }
    // Reconstruir árbol
    Node *root = build_huffman_tree(freq);
    if (!root) { fprintf(stderr, "Error: árbol vacío en descompresión.\n"); exit(EXIT_FAILURE); }
    // Leer número de archivos
    uint32_t m;
    fread(&m, sizeof(uint32_t), 1, in);
    FileInfo *files = malloc(sizeof(FileInfo) * m);
    for (uint32_t i = 0; i < m; i++) {
        uint16_t namelen;
        fread(&namelen, sizeof(namelen), 1, in);
        files[i].name = malloc(namelen+1);
        fread(files[i].name,1,namelen,in);
        files[i].name[namelen] = '\0';
        fread(&files[i].orig_size, sizeof(uint64_t), 1, in);
        fread(&files[i].bit_length, sizeof(uint64_t), 1, in);
    }
    // Crear directorio destino si no existe
    struct stat st = {0};
    if (stat(outdir, &st) == -1) {
        if (mkdir(outdir, 0755) == -1) { perror("mkdir"); exit(EXIT_FAILURE); }
    }
    // Descomprimir archivos
    for (uint32_t i = 0; i < m; i++) {
        uint64_t bits = files[i].bit_length;
        uint64_t bytes_to_read = (bits + 7) / 8;
        uint8_t *buffer = malloc(bytes_to_read);
        fread(buffer,1,bytes_to_read,in);
        BitReader br;
        br_init(&br, buffer, bytes_to_read);
        // Abrir archivo de salida
        char outpath[PATH_MAX];
        snprintf(outpath, sizeof(outpath), "%s/%s", outdir, files[i].name);
        FILE *fout = fopen(outpath, "wb");
        if (!fout) { perror(outpath); exit(EXIT_FAILURE); }
        // Decodificar
        Node *curr = root;
        uint64_t written = 0;
        while (written < files[i].orig_size) {
            int bit = br_read_bit(&br);
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
        free(files[i].name);
        //printf("Descomprimido: %s -> %s (bytes: %lu)\n", files[i].name, outpath, files[i].orig_size);
    }
    // Limpieza
    free(files);
    free_tree(root);
    fclose(in);
    printf("Descompresión completada en directorio: %s\n", outdir);
}

void EjecutarHuffmanSerial(int argc, char *argv[]) {
    if (argc != 4 || (strcmp(argv[1], "-c") && strcmp(argv[1], "-d"))) {
        fprintf(stderr, "Uso:\n  Para comprimir:   %s -c <dir_txt> <salida.huf>\n  Para descomprimir: %s -d <entrada.huf> <dir_destino>\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);  // Usa exit en lugar de return
    }

    if (strcmp(argv[1], "-c") == 0) {
        iniciarMedicionT();  // Inicia la medición del tiempo
        compress_directory(argv[2], argv[3]);  // Llama a la función de compresión
        finalizarMedicionT();  // Finaliza la medición del tiempo
    } else {
        iniciarMedicionT();  // Inicia la medición del tiempo
        decompress_archive(argv[2], argv[3]);  // Llama a la función de descompresión
        finalizarMedicionT();  // Finaliza la medición del tiempo
    }

    exit(EXIT_SUCCESS);  // Usa exit para indicar que el programa terminó con éxito
}

