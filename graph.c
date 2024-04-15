#include "graph.h"

void allocate_graph(graph_t *graph) {
  if (graph->N < 1 || (graph->density < 0.0f || graph->density > 1.0f)) {
    die("allocate_graph: invalid arguments");
  }

  int i;
  graph->edges = malloc(sizeof(u_int8_t *) * graph->N);
  if (graph->edges == NULL) {
    die("allocate_graph: %s", strerror(errno));
  }

  for (i = 0; i < graph->N; i++) {
    graph->edges[i] = malloc(sizeof(u_int8_t) * graph->N);
    if (graph->edges[i] == NULL) {
      die("allocate_graph: %s", strerror(errno));
    }
  }
}

void deallocate_graph(graph_t *graph) {
  int i;
  for (i = 0; i < graph->N; i++) {
    free(graph->edges[i]);
  }
  free(graph->edges);
}

void randomly_initialize_graph(graph_t *graph) {
  allocate_graph(graph);

  srand(time(NULL));
  int i, j;
  for (i = 0; i < graph->N; i++) {
    for (j = 0; j < graph->N; j++) {
      graph->edges[i][j] =
          (((float)rand() / (float)RAND_MAX) > graph->density) ? 1 : 0;
    }
  }

  for (i = 0; i < graph->N; i++) {
    graph->edges[i][i] = 0;
  }
}

void read_file(graph_t *graph) {
  char *filename = "./graph.txt";
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    die("read_file: %s", strerror(errno));
  }

  char buffer[BUF_LEN] = "";
  if (fgets(buffer, sizeof(buffer), fp) == NULL) {
    die("read_file: unable to read fist line");
  }
  graph->N = atoi(buffer);

  if (fgets(buffer, sizeof(buffer), fp) == NULL) {
    die("read_file: unable to read second line");
  }
  graph->density = strtof(buffer, NULL);

  allocate_graph(graph);

  int i, j;
  char *endptr;
  for (i = 0; i < graph->N; i++) {
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
      die("read_file: unable to read 2D array");
    }

    endptr = buffer;
    for (j = 0; j < graph->N; j++) {
      graph->edges[i][j] = (u_int8_t)strtol(endptr, &endptr, 10);
    }
  }

  fclose(fp);
}

void print_graph(graph_t *graph) {
  fprintf(stdout, "N: %d\ndensity: %.2f\n\n", graph->N, graph->density);

  int i, j;
  for (i = 0; i < graph->N; i++) {
    for (j = 0; j < graph->N; j++) {
      fprintf(stdout, "[%u] ", graph->edges[i][j]);
    }
    fprintf(stdout, "\n");
  }
}

#ifdef DEBUG
int main(int argc, char **argv) {
  if (argc != 2) {
    die("usage: ./a.out argument");
  }
  graph_t graph;

  if (atoi(argv[1]) == 0) {
    read_file(&graph);
  } else {
    fprintf(stderr, "Provide N and density: ");

    char buffer[BUF_LEN] = "";
    char *endptr;
    fgets(buffer, sizeof(buffer), stdin);
    graph.N = (int32_t)strtol(buffer, &endptr, 10);
    graph.density = strtof(endptr, NULL);

    randomly_initialize_graph(&graph);
  }

  print_graph(&graph);
  deallocate_graph(&graph);
  return 0;
}
#endif
