#include "graph.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const char *filename = "kernel.cnf";
const char *lingeling = "../lingeling/lingeling";

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
      break; // even if there's n adjacent nodes, we only create one clause
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

void run_satsolver() {
  int fd[2];
  if (pipe(fd) == -1) {
    die("run_satsolver: %s", strerror(errno));
  }

  pid_t pid = fork();
  // fork() for executing the lingeling
  switch (pid) {
  case -1: // error
    die("run_satsolver: %s", strerror(errno));
  case 0:
    // child, lingeling
    close(fd[0]);
    // redirect stdout to pipe
    if (dup2(fd[1], STDOUT_FILENO) == -1) {
      die("dup2: %s", strerror(errno));
    }
    close(fd[1]);

    execl(lingeling, lingeling, filename, NULL);
    die("(run_satsolver: execvp) %s", strerror(errno));
  }

  // parent
  close(fd[1]);

  char buffer[BUF_LEN]; // Buffer to store data read from the pipe
  ssize_t bytes_read;
  bool unsat = true;

  // read data from the pipe until EOF
  while ((bytes_read = read(fd[0], buffer, sizeof(buffer))) > 0) {
    fprintf(stdout, "bytes read: %zd\n", bytes_read);
    if (strstr(buffer, "s SATISFIABLE") != NULL) {
      unsat = false;
    }
  }
  if (bytes_read == -1) {
    die("run_satsolver: %s", strerror(errno));
  }

  if (waitpid(pid, NULL, 0) == -1) {
    die("waitpid: %s", strerror(errno));
  }
  // Close the read end of the pipe
  close(fd[0]);

  if(unsat) {
    fprintf(stdout, "not solved\n");
  }
  else {
    fprintf(stdout, "solved\n");
  }
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
  run_satsolver();

  deallocate_graph(&graph);
  return 0;
}
