#include "graph.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
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

void write_second_constraints(FILE *fp, graph_t *graph, int node_index) {
  fprintf(fp, "%d ", node_index + 1);
  int i;
  for (i = 0; i < graph->N; i++) {
    if (graph->edges[i][node_index]) {
      fprintf(fp, "%d ", i + 1);
    }
  }
  fprintf(fp, "0\n");
}

void write_clauses(graph_t *graph) {
  int i, clauses_cnt = 0;
  // count first constraints is equal to the number of edges
  for (i = 0; i < graph->N; i++) {
    clauses_cnt += count_first_constraints(graph, i);
  }
  // count second constraints is equal to the number of nodes
  clauses_cnt += graph->N;

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

void print_solution(char *solution, int N) {
  char in_kernel[BUF_LEN] = "in kernel: ";
  char not_in_kernel[BUF_LEN] = "not in kernel: ";
  // need to cycle though N vars
  // skip first two bytes "v "
  char *endptr = solution + 2;

  long var;
  int i;
  for (i = 0; i < N; i++) {
    if (endptr[0] == '\n') {
      endptr++;
    }
    if (endptr[0] == 'v') {
      endptr++;
    }
    if (endptr[0] == ' ') {
      endptr++;
    }

    if (isdigit(endptr[0])) {
      var = strtol(endptr, &endptr, 10);
      sprintf(in_kernel + strlen(in_kernel), "%ld ", var - 1);
      continue;
    }

    if (endptr[0] == '-') {
      var = strtol(endptr + 1, &endptr, 10);
      sprintf(not_in_kernel + strlen(not_in_kernel), "%ld ", var - 1);
      continue;
    }
  }

  fprintf(stdout, "%s\n", in_kernel);
  fprintf(stdout, "%s\n", not_in_kernel);
}

void run_satsolver(graph_t *graph) {
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

  if (waitpid(pid, NULL, 0) == -1) {
    die("waitpid: %s", strerror(errno));
  }

  // read after termination, to read whole lingeling output at once
  char buffer[BUF_LEN];
  char *solution = NULL;
  ssize_t bytes_read;

  // read data from the pipe until EOF
  while ((bytes_read = read(fd[0], buffer, sizeof(buffer))) > 0) {
    // fprintf(stdout, "bytes read: %zd\n", bytes_read);
    // fprintf(stdout, "%s", buffer);
    solution = strstr(buffer, "s SATISFIABLE");
  }
  if (bytes_read == -1) {
    die("run_satsolver: %s", strerror(errno));
  }
  // close the read end of the pipe
  close(fd[0]);

  if (solution) {
    solution += 14; // jump 14 bytes to skip "s SATISFIABLE\n"
    strstr(solution, " 0")[0] = '\0';
    fprintf(stdout, "\nkernel problem is satisfiable\n");
    print_solution(solution, graph->N);
    return;
  }

  fprintf(stdout, "\nkernel problem is not satisfiable\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    die("usage: ./a.out argument");
  }
  graph_t graph;

  int argument = atoi(argv[1]);
  if (argument == 0) {
    read_file(&graph);
    fprintf(stdout, "graph read: \n");
    print_graph(&graph);
  } else if (argument == 1) {
    fprintf(stderr, "Provide N and density: ");

    char buffer[BUF_LEN] = "";
    char *endptr;
    fgets(buffer, sizeof(buffer), stdin);
    graph.N = (int32_t)strtol(buffer, &endptr, 10);
    graph.density = strtof(endptr, NULL);

    randomly_initialize_graph(&graph);
    fprintf(stdout, "graph generated: \n");
    print_graph(&graph);
  }
  else {
    die("argument must be of value 0 or 1");
  }

  write_clauses(&graph);
  run_satsolver(&graph);

  deallocate_graph(&graph);
  return 0;
}
