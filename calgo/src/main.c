#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "graph.h"
#include "list.h"
#include "network_th.h"
#include "distrib.h"
#include "simu_th.h"
#include "ui.h"
#include "shell.h"
#include "event.h"

#define N 10
#define NPLAYERS 15
#define NITER    100

int main(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  long int seed = tv.tv_sec * 1000000 + tv.tv_usec;
  srand48(seed);
  srand(seed);

  /*
  struct graph *g = new_graph(N);
  set_randDAG(g, 0.5);
  aff_graph(g, 1);
  list_links(g);

  struct Network *net = new_Network(g->n);
  network_get_graph(net, g);
  set_allfun(net, fun_const);
  set_alldfun(net, fun_zero);

  eHedge_SBPlayers(g, net, NPLAYERS, NITER);

  free_graph(g);
  free_Network(net);
  */
  struct Shell *sh = new_Shell();
  while(1) treat_cmd(sh);
  free_Shell(sh);
  printf("Hello World!\n");
  return 0;
}
