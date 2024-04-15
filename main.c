#include "graph.h"
#include <stdbool.h>
#include <stdio.h>

const char *filename = "kernel.cnf";

int count_first_constraints(graph_t *graph, int node_index) {
  int i, clauses_count = 0;
  for (i = 0; i < graph->N; i++) {
    if (graph->edges[i][node_index] || graph->edges[node_index][i]) {
      clauses_count++;
    }
  }

  return clauses_count;
}

void write_first_constraints(FILE *fp, graph_t *graph, int node_index) {
  int i;
  for (i = 0; i < graph->N; i++) {
    if (graph->edges[i][node_index] || graph->edges[node_index][i]) {
      fprintf(fp, "-%d -%d 0\n", node_index + 1, i + 1);
    }
  }
}

int count_second_constraints(graph_t *graph, int node_index) {
  int i, clauses_count = 0;
  for (i = 0; i < graph->N; i++) {
    if (graph->edges[i][node_index]) {
      clauses_count++;
    }
  }
  return clauses_count;
}

void write_second_constraints(FILE *fp, graph_t *graph, int node_index) {
  bool first_time = true;
  int i;
  for (i = 0; i < graph->N; i++) {
    if (graph->edges[i][node_index]) {
      if (first_time) {
        fprintf(fp, "%d ", node_index + 1);
        first_time = false;
      }
      fprintf(fp, "%d ", i + 1);
    }
  }

  if (!first_time) {
    fprintf(fp, "0\n");
  }
}

void write_clauses(graph_t *graph) {
  int i, clauses_cnt = 0;
  for (i = 0; i < graph->N; i++) {
    clauses_cnt += count_first_constraints(graph, i);
    clauses_cnt += count_second_constraints(graph, i);
  }

  FILE *fp = fopen(filename, "w");
  if (fp == NULL) {
    die("write_clauses: %s", strerror(errno));
  }
  fprintf(fp, "p cnf %d %d\n", graph->N, clauses_cnt);

  for (i = 0; i < graph->N; i++) {
    write_first_constraints(fp, graph, i);
    write_second_constraints(fp, graph, i);
  }

  fclose(fp);
}

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

  write_clauses(&graph);

  deallocate_graph(&graph);
  return 0;
}
