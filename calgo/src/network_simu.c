#include "network_simu.h"


/* *********************** ADMINISTRATION *********************** */

struct SimulatedPlayer *new_SimulatedPlayers(struct graph *g)
{
  int n = g->n;
  struct SimulatedPlayer *vertex = malloc(n * sizeof(struct SimulatedPlayer));
  if (vertex == NULL) { fprintf(stderr, "(malloc) new_SimulatedPlayers\n");
                        exit(EXIT_FAILURE); }

  for (int u=0; u<n; u++)
  {
    vertex[u].d = 0; /* Calcul du degré et des voisins */
    for (int v=0; v<n; v++) vertex[u].d += g->network[u][v];
    vertex[u].neighbours = malloc(vertex[u].d * sizeof(int));
    if (vertex[u].neighbours == NULL) { fprintf(stderr,
                                        "(malloc) new_SimulatedPlayers\n");
                                        exit(EXIT_FAILURE); }

    int k = 0;
    for (int v=0; v<n; v++) if (g->network[u][v])
      vertex[u].neighbours[k++] = v;

    vertex[u].own_c_uv = 0;
    vertex[u].own_c_uv_count = 0;

    vertex[u].W_u  = calloc(n, sizeof(double));
    vertex[u].Y_uv = calloc(vertex[u].d, sizeof(double));
    vertex[u].W_uv = calloc(n, sizeof(double*));
    vertex[u].X_uv = calloc(n, sizeof(double*));

    if (vertex[u].W_u == NULL || vertex[u].Y_uv == NULL
        || vertex[u].W_uv == NULL || vertex[u].X_uv == NULL)
    {
      fprintf(stderr, "(malloc) new_SimulatedPlayers\n");
      exit(EXIT_FAILURE);
    }

    for (int t=0; t<n; t++)
    {
      vertex[u].W_uv[t] = calloc(vertex[u].d, sizeof(double));
      vertex[u].X_uv[t] = calloc(vertex[u].d, sizeof(double));
      if (vertex[u].W_uv[t] == NULL || vertex[u].X_uv[t] == NULL)
      { fprintf(stderr, "(malloc) new_SimulatedPlayers\n"); exit(EXIT_FAILURE);}
    }

    vertex[u].n = 0;
    vertex[u].nIter = 1;
  }

  return vertex;
}

void free_SimulatedPlayers(struct SimulatedPlayer *vertex, int n)
/* n : taille du graphe */
{
  for (int u=0; u<n; u++)
  {
    free(vertex[u].neighbours);
    free(vertex[u].W_u);
    free(vertex[u].Y_uv);
    for (int t=0; t<n; t++) free(vertex[u].W_uv[t]);
    for (int t=0; t<n; t++) free(vertex[u].X_uv[t]);
    free(vertex[u].W_uv);
    free(vertex[u].X_uv);
  }
  return free(vertex);
}

struct SimulatedNetwork *new_SimulatedNetwork(struct graph *g, double E)
{
  struct SimulatedNetwork *snet = malloc(sizeof(struct SimulatedNetwork));
  if (snet == NULL) { fprintf(stderr, "new_SimulatedNetwork\n");
                      exit(EXIT_FAILURE); }

  snet->lambda = malloc(g->n * sizeof(double*));
  if (snet->lambda == NULL) { fprintf(stderr, "new_SimulatedNetwork\n");
                              exit(EXIT_FAILURE); }
  for (int s=0; s<g->n; s++)
  {
    snet->lambda[s] = calloc(g->n, sizeof(double));
    if (snet->lambda[s] == NULL) { fprintf(stderr, "new_SimulatedNetwork\n");
                                   exit(EXIT_FAILURE); }
  }

  snet->E = E; snet->n = g->n;
  snet->qevents = new_EventQueue(1<<20);
  snet->vertex  = new_SimulatedPlayers(g);

  snet->L = calloc(g->n, sizeof(double));
  snet->T = calloc(g->n, sizeof(double));

  if (snet->L == NULL || snet->T == NULL)
  { fprintf(stderr, "new_SimulatedNetwork\n"); exit(EXIT_FAILURE); }

  return snet;
}

void free_SimulatedNetwork(struct SimulatedNetwork *snet)
{
  for (int s=0; s<snet->n; s++) free(snet->lambda[s]);
  free(snet->lambda);
  free_EventQueue(snet->qevents);
  free_SimulatedPlayers(snet->vertex, snet->n);
  free(snet->L);
  free(snet->T);

  return free(snet);
}

/* *********************** UTILITAIRE *********************** */

static double gamma_simu(int n)
{
  return 1. / n;
}

double positive_part(double x)
/* Renvoie 0 si x < 0, x sinon */
{
  return (x > 0) ? x : 0;
}

void reset_c_uv(struct SimulatedPlayer *vertex, int u)
/* Remet les c_uv et count_uv à 0 pour le sommet u*/
{
  vertex[u].own_c_uv = 0;
  vertex[u].own_c_uv_count = 0;

  return;
}

void update_distrib_SimulatedPlayer(struct SimulatedPlayer *vertex, int u, int n,
                                    int nPlayers)
/* Remet à jour la distribution du joueur k, selon une itération n */
{
  //fprintf(stderr, "################# UPDATES %d\n", u);
  int d = vertex[u].d;
  for (int k=0; k<d; k++)
  {
    int v = vertex[u].neighbours[k];
    double c_uv = vertex[v].own_c_uv;
    double c_uv_count = vertex[v].own_c_uv_count;
    vertex[u].Y_uv[k] += gamma_simu(n) * (c_uv + vertex[v].mu) / (c_uv_count + 1);
  }

  for (int t=0; t<nPlayers; t++) for (int k=0; k<d; k++)
  {
    int v = vertex[u].neighbours[k];
    vertex[u].W_uv[t][k]  = vertex[v].W_u[t] - vertex[u].Y_uv[k];
  }

  /* Recalcul de W_u[t] et X_uv[t] */
  for (int t=0; t<nPlayers; t++)
  {
    if (vertex[u].W_u[t] == -INFINITY) continue;
    if (u == t) { vertex[u].W_u[t] = 0; continue; }
    int need_update = 0;
    for (int i=0; i<vertex[u].d; i++) if (vertex[u].W_uv[t][i] != -INFINITY)
      need_update = 1;
    if (!need_update)
    {
      //printf(" · set %d to -INFINITY\n", t);
      vertex[u].W_u[t] = -INFINITY;
      continue;
    }
    /* Calcul de W_u[t] */
    double W_max = max(vertex[u].W_uv[t], d);
    vertex[u].W_u[t] = 0;
    for (int i=0; i<d; i++) vertex[u].W_u[t] += exp(vertex[u].W_uv[t][i] - W_max);

    vertex[u].W_u[t] = W_max + log(vertex[u].W_u[t]);

    /* Calcul de X_uv[t] */
    pos_balanced_logit(vertex[u].X_uv[t], vertex[u].W_uv[t], 0, d);
    //printf(" · (%lf) %d --> %d : ", vertex[u].W_u[t], u, t);
    //print_distrib(vertex[u].X_uv[t], d);
  }

  return ;
}

/* *********************** SIMULATION *********************** */


void event_NEW_PAQUET(struct SimulatedNetwork *snet, struct Event event)
{
  int s = event.u;
  int t = event.sink;

  double delta = rand_exponential(snet->vertex[s].mu);
  snet->L[s] = delta + positive_part(snet->L[s] + snet->T[s] - event.T);
  snet->T[s] = event.T;

  snet->vertex[s].own_c_uv += snet->L[s];
  snet->vertex[s].own_c_uv_count ++;

  double next_T = event.T + rand_exponential(snet->lambda[s][t]);
  struct EventID next_ID = new_EventID(next_T, s, t);
  add_Event(new_Event(next_T, s, NEW_PAQUET, t, next_ID), &snet->qevents);
  add_Event(new_Event(event.T + snet->L[s], s, TREAT_PAQUET, t, event.ID),
            &snet->qevents);

  return ;
}

void event_TREAT_PAQUET(struct SimulatedNetwork *snet, struct Event event)
{
  int u = event.u;
  int t = event.sink;
  int d = snet->vertex[u].d;

  snet->vertex[u].n ++;
  //fprintf(stderr, "@@@ %lf, %d\n", event.T, u);

  if (t == u) /* Le paquet est arrivé à Destination */
  {
    /*fprintf(stderr, "--------------- @%.2f Received (%d): %d %d, %lf\n",
    event.T, u, event.ID.source, event.ID.sink, event.T - event.ID.T);*/
    fprintf(stderr, "%lf %lf\n", event.T, event.T - event.ID.T);
    snet->vertex[u].W_u[t] = 0;
    return;
  }

  if (snet->vertex[u].d == 0)
  {
    /*fprintf(stderr, "--------------- @%.2f Lost     (%d): %d %d, %lf\n",
    event.T, u, event.ID.source, event.ID.sink, event.T - event.ID.T);*/
    fprintf(stderr, "%lf %lf\n", event.T, event.T - event.ID.T);
    snet->vertex[u].W_u[t] = -INFINITY;
    return ;
  }

  /* Sélection de la destination */
  int k = select_on_distrib(snet->vertex[u].X_uv[t], d);
  int v = snet->vertex[u].neighbours[k];
  double delta = rand_exponential(snet->vertex[v].mu);
  snet->L[v] = delta + positive_part(snet->L[v] + snet->T[v] - event.T);
  snet->T[v] = event.T;

  /* MàJ c_uv */
  snet->vertex[v].own_c_uv += snet->L[v];
  snet->vertex[v].own_c_uv_count ++;

  double next_T = event.T + snet->L[v];
  add_Event(new_Event(next_T, v, TREAT_PAQUET, t, event.ID), &snet->qevents);

}

void event_UPDATE_DISTRIB(struct SimulatedNetwork *snet, struct Event event)
{
  int u = event.u;
  int t = event.sink;

  /* On met à jour la distribution de u */
  update_distrib_SimulatedPlayer(snet->vertex, u, snet->vertex[u].nIter, snet->n);
  reset_c_uv(snet->vertex, u); /* On reset ici... FIXME ?? */

  double next_T = event.T + snet->E + rand_exponential(snet->vertex[u].mu);
  struct Event next_update = new_Event(next_T, u, UPDATE_DISTRIB, t, event.ID);
  add_Event(next_update, &snet->qevents);

  snet->vertex[u].nIter ++;
  return;
}

void treat_new_event(struct SimulatedNetwork *snet)
{
  struct Event event;
  if (!next_event(&event, snet->qevents))
  { fprintf(stderr, "No event ! \n"); exit(EXIT_FAILURE); }

  //printf("Extracting : "); print_Event(event);

  if (event.action == NEW_PAQUET) event_NEW_PAQUET(snet, event);
  else if (event.action == TREAT_PAQUET) event_TREAT_PAQUET(snet, event);
  else if (event.action == UPDATE_DISTRIB) event_UPDATE_DISTRIB(snet, event);
  else { fprintf(stderr, "Unexpected event\n"); exit(EXIT_FAILURE); }

  return;
}

/* *********************** UTILITAIRE *********************** */

void spread_Simulated_mass(struct SimulatedNetwork *snet, struct Network *net)
/* Met la masse dans le network 'net' selon les distributions
 * des joueurs simulés. */
{
  /* local_mass[u][t] = masse partant de u en direction de t */
  double **local_mass = malloc(snet->n * sizeof(double*));
  if (local_mass == NULL) { fprintf(stderr, "(malloc) spread_Simulated_mass\n");
                            exit(EXIT_FAILURE); }
  for (int u=0; u<snet->n; u++)
  {
    local_mass[u] = calloc(snet->n, sizeof(double));
    if (local_mass[u] == NULL) { fprintf(stderr, "(malloc) spread_Simulated_mass\n");
                                 exit(EXIT_FAILURE); }
  }

  /* Calcul de la masse dans le graphe */
  for (int u=0; u<snet->n; u++)
  {
    /* Ajoute de la masse partante de u */
    for (int t=0; t<snet->n; t++) local_mass[u][t] += snet->lambda[u][t];

    /* Calcul de la masse sur les arcs uv, màj de la masse des v voisins de u */
    for (int k=0; k<snet->vertex[u].d; k++)
    {
      int v = snet->vertex[u].neighbours[k];
      net->masses[u][v] = 0; /* Reset de la masse */
      for (int t=0; t<snet->n; t++)
      {
        double mass = snet->vertex[u].X_uv[t][k] * local_mass[u][t];
        local_mass[v][t] += mass;
        net->masses[u][v] += mass;
      }
    }
  }

  free_cost_matrix(local_mass, snet->n);
  return ;
}
