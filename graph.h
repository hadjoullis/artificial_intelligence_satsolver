#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define die(...)                                                               \
  do {                                                                         \
    fprintf(stderr, __VA_ARGS__);                                              \
    fputc('\n', stderr);                                                       \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define BUF_LEN 1024

typedef struct {
  int32_t N;
  float density;
  u_int8_t **edges;
} graph_t;

void allocate_graph(graph_t *graph);
void deallocate_graph(graph_t *graph);
void randomly_initialize_graph(graph_t *graph);
void print_graph(graph_t *graph);
void read_file(graph_t *graph);
