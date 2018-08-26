#ifndef simu_th_h
#define simu_th_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "network_th.h"
#include "list.h"
#include "distrib.h"
#include "graph.h"
#include "ui.h"

void eHedge_SBPlayers(struct graph *g, struct Network *net, int nPlayers,
                      int nIter);
/* Algorithme d'epsilon-Hedge sur nPlayers joueurs, sur nIter itérations */

#endif
