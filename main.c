#include "graph.h"

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
