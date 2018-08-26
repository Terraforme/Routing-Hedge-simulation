#include "network_th.h"

#define handle_error(s) do {fprintf(stderr, #s "\n"); exit(EXIT_FAILURE); } while(0);


/* ***************** Fonctions administratives ***************** */

/* Quelques fonctions pour encapsuler le code */

static int** malloc_int_matrix(int n)
/* Renvoie une matrice n × n d'entiers franchement allouée */
{
  int **res = malloc(sizeof(int*) * n);
  if (res == NULL) handle_error("malloc_int_matrix");
  for (int i=0; i<n; i++)
  {
    res[i] = malloc(sizeof(int) * n);
    if (res[i] == NULL) handle_error("malloc_int_matrix");
  }

  return res;
}

static double** malloc_double_matrix(int n)
/* Renvoie une matrice n × n de 'double' franchement allouée */
{
  double **res = malloc(sizeof(double*) * n);
  if (res == NULL) handle_error("malloc_double_matrix");
  for (int i=0; i<n; i++)
  {
    res[i] = malloc(sizeof(double) * n);
    if (res[i] == NULL) handle_error("malloc_double_matrix");
  }

  return res;
}

static dtod_t **malloc_functions_matrix(int n)
{
  double (***res)(double) = malloc(n * sizeof(dtod_t*));
  if (res == NULL) handle_error("malloc_functions_matrix");
  for (int i=0; i<n; i++)
  {
    res[i] = malloc(n * sizeof(dtod_t));
    if (res[i] == NULL) handle_error("malloc_functions_matrix");
  }
  return res;
}


/* Fonctions utiles */

struct Network *new_Network(int n)
/* Renvoie un nouveau réseau vide */
/* Cette fonction est vachement longue */
{
  struct Network *net = malloc(sizeof(struct Network));
  if (net == NULL) handle_error("(malloc) new_Network");

  /* Graphe && masses */
  net->graph = malloc_int_matrix(n);
  net->masses = malloc_double_matrix(n);

  /* Fonctions */
  net->cost  = malloc_functions_matrix(n);
  net->dcost = malloc_functions_matrix(n);
  net->d2cost = malloc_functions_matrix(n);

  /* Taille du graphe */
  net->n = n;

  return net;
}

void free_Network(struct Network *net)
/* Libère la mémoire dédiée à un réseau */
{
  for (int i=0; i<net->n; i++)
  {
    free(net->graph[i]);
    free(net->masses[i]);
    free(net->cost[i]);
    free(net->dcost[i]);
    free(net->d2cost[i]);
  }
  free(net->graph);
  free(net->masses);
  free(net->cost);
  free(net->dcost);
  free(net->d2cost);

  return free(net);
}

void set_netfun (int i, int j, struct Network *net, dtod_t fun)
/* met la fonction i,j à fun */
{
  net->cost[i][j] = fun;
  return ;
}

void set_netdfun(int i, int j, struct Network *net, dtod_t fun)
/* met la dérivée  i,j à fun */
{
  net->dcost[i][j] = fun;
}

void network_get_graph(struct Network *net, struct graph *g)
/* Copie le graphe dans le réseau - pas forcément utile, mais ... */
{
  if (g->n != net->n) handle_error("network_get_graph : non-matching sizes");
  int n = g->n;

  for (int i=0; i<n; i++)
  for (int j=0; j<n; j++) net->graph[i][j] = g->network[i][j];

  return ;
}

/* ***************** Fonctions Basiques ***************** */

void reset_network(struct Network *net)
/* Efface toutes les masses, état du graphe, et pointeurs de fonctions */
{
  int n = net->n;
  for (int i=0; i<n; i++)
  for (int j=0; j<n; j++)
  {
    net->graph[i][j] = net->masses[i][j] = 0;
    net->cost[i][j] = net->dcost[i][j] = NULL;
  }
  return ;
}

void reset_masses (struct Network *net)
/* Remet toutes les masses à zéro */
{
  for (int i=0; i<net->n; i++)
  for (int j=0; j<net->n; j++)
  {
    net->masses[i][j] = 0;
  }
  return ;
}

void add_mass_over(struct Network *net, double mass, struct List *path)
/* Distribue la masse 'mass' sur le chemin en argument */
{
  if (is_empty(path) || is_empty(path->tail)) return ;

  int u = *((int*) path->head);
  int v;
  path = path->tail;
  while (!is_empty(path))
  {
    v = *((int*) path->head);
    net->masses[u][v] += mass;
    u = v;
    path = path->tail;
  }

  return ;
}

void add_mass_of_player(struct Network *net, double pmass,
                        struct List *paths, double *distrib)
/* Étant donné tous les chemins possibles d'un joueur, la demande de ce joueur
 * et la manière dont il distribue la masse sur ses chemins possibles,
 * met à jours la masse dans le réseau */
{
  struct List *path;
  int i = 0;
  while (!is_empty(paths))
  {
    path = paths->head;
    add_mass_over(net, pmass * distrib[i], path);
    i ++;
    paths = paths->tail;
  }
  return ;
}

void add_mass_on_links(struct Network *net, double pmass,
                       int origin, int *neighbours, int n,
                       double *distrib)
/* Étant donnés les liens origin -> neighbours[.], ajoute de la masse
 * selon la distribution donnée */
{
  for (int i=0; i<n; i++) net->masses[origin][neighbours[i]] += distrib[i] * pmass;
  return;
}

/* ***************** Fonctions de calcul des coûts ***************** */

double compute_path_cost(struct Network *net, struct List *path)
/* Calcule \sum_{e \in path} c(x_e) */
{
  if (is_empty(path) || is_empty(path->tail)) return 0;
  double c = 0;
  int u = *((int*) path->head); int v;
  path = path->tail;

  while(!is_empty(path))
  {
    v = *((int*) path->head);
    c += net->cost[u][v](net->masses[u][v]);
    path = path->tail;
    u = v;
  }

  return c;
}

double compute_modified_path_cost(struct Network *net, struct List *path)
/* Calcule \sum_{e \in path} x_e c'(x_e) + c(x_e) */
{
  if (is_empty(path) || is_empty(path->tail)) return 0;
  double c = 0;
  int u = *((int*) path->head); int v;
  path = path->tail;

  while(!is_empty(path))
  {
    v = *((int*) path->head);
    double x_uv = net->masses[u][v];
    c += x_uv * net->dcost[u][v](x_uv) + net->cost[u][v](x_uv);
    path = path->tail;
    u = v;
  }

  return c;
}

double  **cost_matrix(struct Network *net)
/* renvoie la matrice cost[i][j] */
{
  int n = net->n;
  double **cost_mat = malloc_double_matrix(n);
  for (int i=0; i<n; i++) for (int j=0; j<n; j++)
    cost_mat[i][j] = net->cost[i][j](net->masses[i][j]);

  return cost_mat;
}

double **mcost_matrix(struct Network *net)
/* renvoie la matrice des coûts modifiés */
{
  int n = net->n;
  double **mcost_mat = malloc_double_matrix(n);
  for (int i=0; i<n; i++) for (int j=0; j<n; j++)
  {
    double x_ij = net->masses[i][j];
    mcost_mat[i][j] = x_ij * net->dcost[i][j](x_ij) + net->cost[i][j](x_ij);
  }

  return mcost_mat;
}

void free_cost_matrix(double **cost_mat, int n)
/* Libère une matrice de coût */
{
  for (int i=0; i<n; i++) free(cost_mat[i]);
  return free(cost_mat);
}

double fast_path_cost( struct List *path, double **cost_mat)
/* Fait la même chose que compute_path_cost mais utilise une matrice
 * précalculée des coûts */
{
  if (is_empty(path) || is_empty(path->tail)) return 0;
  double c = 0;
  int u = *((int*) path->head); int v;
  path = path->tail;

  while(!is_empty(path))
  {
    v = *((int*) path->head);
    c += cost_mat[u][v];
    path = path->tail;
    u = v;
  }

  return c;
}

double fast_modified_path_cost(struct List *path,
                               double **mcost_mat)
/* Fait la même chose que compute_modified_path_cost mais utilise une matrice
 * précalculée des coûts modifiés  */
{
  if (is_empty(path) || is_empty(path->tail)) return 0;
  double c = 0;
  int u = *((int*) path->head); int v;
  path = path->tail;

  while(!is_empty(path))
  {
    v = *((int*) path->head);
    c += mcost_mat[u][v];
    path = path->tail;
    u = v;
  }

  return c;
}

/* ***************** CALCUL DU POTENTIEL ***************** */

double net_potential(struct Network *net)
/* Renvoie le potentiel du réseau */
{
  double potential = 0;
  for (int i=0; i<net->n; i++)
  for (int j=0; j<net->n; j++)
    if (net->masses[i][j])
      potential += net->masses[i][j] * net->cost[i][j](net->masses[i][j]);
  return potential;
}

double net_d2potential(struct Network *net)
{
  double potential = 0;
  for (int i=0; i<net->n; i++)
  for (int j=0; j<net->n; j++)
    if (net->masses[i][j])
    {
      double x_ij = net->masses[i][j];
      potential += x_ij * net->d2cost[i][j](x_ij) + net->dcost[i][j](x_ij)
                   + net->cost[i][j](x_ij);
    }
  return potential;
}

/* ***************** CALCUL DE CONVERGENCE ***************** */

double DAG_worst_used_Path(int s, int t, double **mass,
                      double **cost_mat, double min_mass, struct graph *g)
/* Renvoie le coût du pire chemin de s à t, de masse non nulle */
{
  double *d = calloc(g->n, sizeof(double));
  if (d == NULL) { fprintf(stderr, "(calloc) DAH_shortest_Path\n");
                   exit(EXIT_FAILURE); }

  /* On multiplie les coûts par -1 */
  for (int u=s+1; u<=t; u++) d[u] = +INFINITY;

  for (int u=s; u<t; u++)
  for (int v=u+1; v<=t; v++)
  if (g->network[u][v]
      &&  !isnan(mass[u][v]) && mass[u][v] > min_mass
      && d[v] > d[u] - cost_mat[u][v])
    d[v] = d[u] - cost_mat[u][v];

  double res = -d[t];
  free(d);
  return res;
}

double DAG_shortest_Path(int s, int t, double **mass,
                         double **cost_mat, struct graph *g)
/* Renvoie le coût du meilleur chemin de s à t (toutes masses confondues) */
{
  double *d = calloc(g->n, sizeof(double));
  if (d == NULL) { fprintf(stderr, "(calloc) DAH_shortest_Path\n");
                   exit(EXIT_FAILURE); }

  for (int u=s+1; u<=t; u++) d[u] = +INFINITY;

  for (int u=s; u<t; u++)
  for (int v=u+1; v<=t; v++)
  if (mass[u][v] && d[v] > d[u] + cost_mat[u][v])
    d[v] = d[u] + cost_mat[u][v];

  double res = d[t];
  free(d);
  return res;
}

int convergence_on(int s, int t, double **mass, double epsilon,
                   double **cost_mat, struct graph *g)
/* Renvoie 1 si c(p') <= c(p) + e pour tous p,p' chemins s --> t
 * avec p un chemin utilisé (i.e de masse non nulle).
 * Renvoie 0 sinon */
{
  int m = 0;
  for (int u=0; u<g->n; u++)
  for (int v=0; v<g->n; v++)
  if (mass[u][v]) m++;

  double min_mass = epsilon / m;

  double best_path  = DAG_shortest_Path(s,   t, mass, cost_mat, g);
  double worst_path = DAG_worst_used_Path(s, t, mass, cost_mat, min_mass, g);

  /*printf("\x1b[1K\r(%d/%d) best : %lf, worst : %lf (wmm = %lf)\n",  s, t, best_path, worst_path
         ,min_mass);*/
  return worst_path <= best_path + epsilon;
}

/* ***************** Fonctions d'initialisation ***************** */

void set_allfun (struct Network *net, dtod_t fun)
/* met toutes les fonctions à fun */
{
  int n = net->n;
  for (int i=0; i<n; i++)
  for (int j=0; j<n; j++) net->cost[i][j] = fun;

  return ;
}
void set_alldfun(struct Network *net, dtod_t fun)
/* met toutes les dérivées à fun */
{
  int n = net->n;
  for (int i=0; i<n; i++)
  for (int j=0; j<n; j++) net->dcost[i][j] = fun;

  return ;
}

void set_alld2fun(struct Network *net, dtod_t fun)
{
  int n = net->n;
  for (int i=0; i<n; i++)
  for (int j=0; j<n; j++) net->d2cost[i][j] = fun;

  return ;
}

/* ***************** AFFICHAGE ***************** */

void aff_masses(struct Network *net)
{
  for (int i=0; i<net->n; i++)
  {
    for (int j=0; j<net->n; j++) printf("%.3f ", net->masses[i][j]);
    printf("\n");
  }
  return ;
}
