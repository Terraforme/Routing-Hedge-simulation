#include "ui.h"

#define handle_error(s) do {fprintf(stderr, #s "\n"); exit(EXIT_FAILURE); } while(0);



/* ************* FONCTIONS ADMINISTRATIVES ************* */

struct SBPlayer *new_SBPlayers(int n)
/* Renvoie un pointeur vers un tableau de n nouveaux joueurs (non initalisés) */
{
  struct SBPlayer *players = malloc(n * sizeof(struct SBPlayer));
  if (players == NULL) handle_error("(malloc) new_SBPlayers");

  return players;
}

void init_SBPlayer(int i, struct SBPlayer *players, struct graph *g,
                   int **vertices)
/* Initialise le i-ième joueur (couple ss, masse, chemins, évaluations) */
{
  int N = g->n, a, b;

  /* Couple ss */
  do {
    a = random() % N;
    b = random() % N;
    while (b == a) b = random() % N;
    players[i].source = (a > b) ? b : a;
    players[i].sink   = (a > b) ? a : b;
  } while (!connected(players[i].source, players[i].sink, g));

  /* Masse et chemins */
  players[i].mass  = 1; /* FIXME : pas d'aléatoire */
  players[i].paths = path_from_to(players[i].source, players[i].sink, g, vertices);
  players[i].n     = len(players[i].paths);
  players[i].Y_uv = calloc(players[i].n, sizeof(double));

  return ;
}

void init_SBPlayers(struct SBPlayer *players, struct graph *g, int n,
                    int **vertices)
/* Initialise les n premiers joueurs
 * Remarque : les joueurs sont indépendants. */
{
  for (int i=0; i<n; i++) init_SBPlayer(i, players, g, vertices);
  return ;
}

void set_SBPlayer(int i, struct SBPlayer *players, int source, int sink,
                  double mass, struct graph *g, int **vertices)
/* Initialise le joueur 'i' à (source, sink, mass) */
{
  players[i].mass = mass;
  players[i].source = source;
  players[i].sink   = sink;
  players[i].paths  = path_from_to(source, sink, g, vertices);
  players[i].n      = len(players[i].paths);
  players[i].Y_uv   = calloc(players[i].n, sizeof(double));

  if (players[i].Y_uv == NULL) { fprintf(stderr, "(calloc) set_SBPlayer\n");
                                 exit(EXIT_FAILURE); }

  return;
}

void normalize_SBPlayers(struct SBPlayer *players, int n)
/* Normalise les masses des n premiers joueurs de SBP */
{
  double s = 0;
  for (int i=0; i<n; i++) s += players[i].mass;
  for (int i=0; i<n; i++) players[i].mass /= s;
  return ;
}

void free_SBPlayers(struct SBPlayer *players, int n)
/* Libère les n premiers joueurs de players, et libère le pointeur 'players' */
{
  for (int i=0; i<n; i++)
  {
    free(players[i].Y_uv);
    free_paths(players[i].paths);
  }
  free(players);
  return ;
}

void reset_SBPlayers(struct SBPlayer *players, int n)
/* Remet les évaluation des n premiers joueurs à 0 */
{
  for (int i=0; i<n; i++)
  {
    free(players[i].Y_uv);
    players[i].Y_uv = new_distrib(players[i].n);
  }
  return ;
}

/* ******************** FONCTIONS DE JEU ******************** */

double* SBPlayer_distrib(int i, struct SBPlayer *players,
                         double e)
/* Calcule la distribution du joueur i qu'il renvoie sous la forme d'un tableau.
 * Méthode du semi-bandit : celui-là a relevé des coûts modifiés */
{
  double *distrib = new_distrib(players[i].n);
  balanced_logit(distrib, players[i].Y_uv, e, players[i].n);
  /*printf("@@@@@@@  ");
  for (int j=0; j<players[i].n; j++) printf("%.3f (%d), ", distrib[j], j);
  printf("\n");*/
  return distrib;
}


double* fast_eval_player(int i, struct SBPlayer *players, double **cost_mat)
/* Renvoie la matrice des coûts du joueur i, en fonction de la matrice des
 * coûts en paramètre. Si celle-si est la matrice des coûts purs, alors
 * on a le cas bandit ; si c'est la matrice des coûts modifiés, alors on a
 * le cas semi-bandit. */
{
  int k = 0;
  struct List *paths = players[i].paths;
  struct List *path  = NULL;
  double *distrib = new_distrib(players[i].n);
  while (!is_empty(paths))
  {
    path = paths->head;
    distrib[k++] = fast_path_cost(path, cost_mat);
    paths = paths->tail;
  }
  return distrib;
}

double **paths_mass_spread(int p, struct SBPlayer *players, int n)
/* Renvoie la matrice des masses du joueur p */
{
  struct Network net;
  net.masses = malloc(n * sizeof(double*));
  if (net.masses == NULL) { fprintf(stderr, "(malloc) paths_mass_spread\n");
                            exit(EXIT_FAILURE); }

  for (int i=0; i<n; i++)
  {
    net.masses[i] = calloc(n, sizeof(double));
    if (net.masses[i] == NULL) { fprintf(stderr, "(malloc) paths_mass_spread\n");
                                 exit(EXIT_FAILURE); }
  }

  double *distrib = SBPlayer_distrib(p, players, 0);
  add_mass_of_player(&net, 1, players[p].paths, distrib);
  free(distrib);
  return net.masses;
}

/* ******************* FONCTIONS USER INTERFACE ******************* */

void aff_SBPlayers(struct SBPlayer *players, int n, int verbative)
/* Affiche les n premiers joueurs de 'players' */
{
  for (int i=0; i<n; i++)
  {
    printf("Player #%d : \n", i);
    printf("\tSource & sink : %d & %d\n", players[i].source, players[i].sink);
    printf("\tMass        : %g\n", players[i].mass);
    printf("\tPaths       : %d\n", len(players[i].paths));
    if (verbative)
    {
      printf("\t@");
      aff_paths(players[i].paths);
      printf("\n");
    }
    printf("\n");
  }
  return ;
}

void aff_SBPlayer_score(int i, struct SBPlayer *players)
/* Affiche la liste des correspondances chemin/masse accordée */
{
  printf("Player #%d (%d - %d): \n", i, players[i].source, players[i].sink);
  struct List *paths = players[i].paths;
  double *distrib = SBPlayer_distrib(i, players, 0);

  int k = 0;
  while (!is_empty(paths))
  {
    printf("\t%.3f {%.3f}: ", distrib[k], players[i].Y_uv[k]);
    k++;
    aff_List_int(paths->head);
    printf("\n");
    paths = paths->tail;
  }

  free(distrib);
  return ;
}


/* *********************** CAS D'UN JOUEUR PAR NOEUD *********************** */
/* Plaçons-nous maintenant dans le cas d'un joueur par nœud */


void init_VertexPlayer(struct VertexPlayer *players, struct graph *g, int id)
/* Initialise le players d'id ID */
{
  /* Quantités triviales */
  players[id].id = id;
  players[id].W_u = 0;

  players[id].d = 0; /* Calcul du degré */
  for (int j=0; j<g->n; j++) players[id].d += g->network[id][j];

  /* Calcul des voisins */
  players[id].neighbours = malloc(players[id].d * sizeof(int));
  if (players[id].neighbours == NULL) handle_error("(malloc) init_VertexPlayer");
  int k = 0;
  for (int j=0; j<g->n; j++)
  if (g->network[id][j]) players[id].neighbours[k++] = j;

  /* Initialisation des degrés */
  players[id].Y_uv = calloc(players[id].d, sizeof(double));
  /* Initialisation des w */
  players[id].W_uv = calloc(players[id].d, sizeof(double));

  return;
}

void free_VertexPlayer(struct VertexPlayer *players, int id)
/* Libère la mémoire dédiée au joueur d'id 'id' */
{
  free(players[id].neighbours);
  free(players[id].Y_uv);
  free(players[id].W_uv);
  return ;
}

struct VPPopulation *new_population_set(struct graph *g, int k)
/* Renvoie un tableau de k nouvelles populations en adéquation avec le graphe G */
{
  struct VPPopulation *pop = malloc(k * sizeof(struct VPPopulation));
  if (pop == NULL) handle_error("(malloc) new_population");

  for (int p=0; p<k; p++)
  {
    pop[p].players = malloc(g->n * sizeof(struct VertexPlayer));
    if (pop[p].players == NULL) handle_error("(malloc) new_population");

    /* Masse */
    pop[p].mass = 1./k;

    /* Couples source/destination */
    int a, b;
    do {
      b = random() % g->n;
      a = random() % g->n;
      while (b == a) b = random() % g->n;
      pop[p].source = (a > b) ? b : a;
      pop[p].sink   = (a > b) ? a : b;
    } while (!connected(pop[p].source, pop[p].sink, g));

    pop[p].n = g->n;
    /* Joueurs */
    for (int i=0; i<g->n; i++) init_VertexPlayer(pop[p].players, g, i);
    for (int i=0; i<g->n; i++) if (i<pop[p].source || i>pop[p].sink)
      pop[p].players[i].W_u = -INFINITY;
  }

  return pop;
}

void free_VPPopulation_set(struct VPPopulation *pop, int k)
/* Libère la mémoire dédiée à k populations */
{
  for (int p=0; p<k; p++)
  {
    for (int i=0; i<pop[p].n; i++) free_VertexPlayer(pop[p].players, i);
    free(pop[p].players);
  }

  return free(pop);
}

void reset_VPPopulation_set(struct VPPopulation *pop, int k)
/* Remet à 0 les évaluation d'un ensemble de populations */
{
  for (int p=0; p<k; p++)
  {
    for (int i=0; i<pop[p].n; i++)
    {
      free(pop[p].players[i].Y_uv);
      pop[p].players[i].Y_uv = new_distrib(pop[p].players[i].d);
      free(pop[p].players[i].W_uv);
      pop[p].players[i].W_uv = new_distrib(pop[p].players[i].d);

      if (i>pop[p].sink || i<pop[p].source) pop[p].players[i].W_u = -INFINITY;
      else pop[p].players[i].W_u = 0;
    }
  }
  return ;
}


/* ******************** FONCTIONS USUELLES ******************** */

void normalize_VPPopulation_set(struct VPPopulation *pop, int k)
/* Normalise, ensembles, les k premières populations */
{
  double smass = 0;
  for (int i=0; i<k; i++) smass += pop[i].mass;

  for (int i=0; i<k; i++) pop[i].mass /= smass;
  return ;
}

void set_VPPopulation_mass(double mass, struct VPPopulation *pop, int i)
/* Met la masse d'une population donnée à la quantité demandée */
/* Ne renormalise pas */
{
  pop[i].mass = mass;
  return;
}

void convexity_fix_VPPopulation(struct VPPopulation *pop, struct graph *g, int n)
/* Sous optimal - met des coûts infinis pour éviter la perte de paquets */
{
  for (int p=0; p<n; p++)
  for (int u=g->n-1; u>=0; u--)
  {
    if (u>pop[p].sink || u<pop[p].source) pop[p].players[u].W_u = -INFINITY;
    else if (!connected(u, pop[p].sink, g)) pop[p].players[u].W_u = -INFINITY;
    else for (int i=0; i<pop[p].players[u].d; i++)
    {
      int v = pop[p].players[u].neighbours[i];
      pop[p].players[u].W_uv[i] = pop[p].players[v].W_u;
    }
  }
  return;
}

/* ******************** FONCTIONS D'ÉVALUATION ******************** */

double *VertexPlayer_distrib(int i, struct VertexPlayer *players, double e)
/* Calcule la distribution du joueur i */
{
  double *distrib = new_distrib(players[i].d);
  pos_balanced_logit(distrib, players[i].W_uv, e, players[i].d); /* NEW : POS */
  /*printf("@@@@@@(%d) : ", i);
  for (int j=0; j<players[i].d; j++)
  printf("%.3f(%d)[%g]{%g} ", distrib[j], players[i].neighbours[j],
                          players[i].W_uv[j], players[i].Y_uv[j]);
  printf("\n");*/
  return distrib;
}

void update_eval_VertexPlayer(int i, struct VertexPlayer *players, double *eval,
                              int sink)
/* Met à jour les évaluations du joueur i étant données les coûts qu'il vient de
 * mesurer - on assume que les joueurs topologiquement supérieurs à i on
 * déjà actualisé leur score. */
{
  /* Version avec coûts modifiés */
  /* eval représente les évaluations fraîches sur les arêtes, pondérées par gamma*/

  if (i > sink) players[i].W_u = -INFINITY;
  else if (i == sink) players[i].W_u = 0;

  else /* cas général */
  {
    /* Calcul des w_uv = -Y_uv + w_v */
    for (int j=0; j<players[i].d; j++) players[i].Y_uv[j] += eval[j]; /* Y_uv */
    for (int j=0; j<players[i].d; j++)
    {
      int l = players[i].neighbours[j];
      if (players[l].W_u == -INFINITY) players[i].W_uv[j] = -INFINITY;
      else players[i].W_uv[j] = - players[i].Y_uv[j] /* W_uv : attention - !*/
                              + players[l].W_u;
      //printf("W_%d.%d = %.3f\n", i, l, players[i].W_uv[j]);
    }

    /* Actualisation de w */
    double w_max = -INFINITY;
    for (int j=0; j<players[i].d; j++)
    {
      double n_score = players[i].W_uv[j];
      if (n_score != -INFINITY) w_max = (w_max > n_score) ? w_max : n_score;
    }
    if (w_max == -INFINITY) { players[i].W_u = -INFINITY; return; }

    players[i].W_u = 0;
    for (int j=0; j<players[i].d; j++)
    if (players[i].W_uv[j] != -INFINITY)
      players[i].W_u += exp(players[i].W_uv[j] - w_max);

    players[i].W_u = w_max + log(players[i].W_u);
    //if (players[i].W_u == -INFINITY) players[i].W_u = +INFINITY;
  }
  //printf("##%d W_u : %f\n", i, players[i].W_u);
  return ;
}


double **mass_spread(int p, struct VPPopulation *pop, double e)
/* Renvoie la matrice de la répartition de masse faite par la population i */
{
  int n = pop[p].n;
  double **mass = malloc(n * sizeof(double*));
  double  *local_mass = calloc(n, sizeof(double));
  for (int i=0; i<n; i++) mass[i] = calloc(n, sizeof(double));

  local_mass[pop[p].source] = pop[p].mass;

  /* La masse passant en un nœud i est la somme mass[.<i][i] */
  for (int u=pop[p].source; u<pop[p].sink; u++)
  if (pop[p].players[u].W_u != -INFINITY)
  {
    double *distrib = VertexPlayer_distrib(u, pop[p].players, e); /* distribution */
    /*if (i == pop[p].source)
    {
      printf("---- POP %d :", p);
      for (int j=0; j<pop[p].players[i].d; j++)
      printf("(%d)%.3f, ", pop[p].players[i].neighbours[j], distrib[j]);
    } printf("\n");*/

    /* Calcul de la masse locale */

    /* Propagation de la masse */
    for (int k=0; k<pop[p].players[u].d; k++)
    {
      int v = pop[p].players[u].neighbours[k];
      mass[u][v] = local_mass[u] * distrib[k];
      local_mass[v] += mass[u][v];
    }


    free(distrib);
  }

  free(local_mass);

  return mass;
}

/* #################### ADMINISTRATION #################### */

void init_VertexBandit(struct VertexBandit *bandits, struct graph *g, int u)
/* Initialise le bandit sur le sommet u */
{
  bandits[u].W_u = 0; bandits[u].d = 0; /* Calcul du degré */
  for (int v=0; v<g->n; v++) bandits[u].d += g->network[u][v];

  int d = bandits[u].d;
  bandits[u].neighbours = malloc(d * sizeof(int));
  bandits[u].Y_uv  = calloc(d, sizeof(double));
  bandits[u].W_uv  = calloc(d, sizeof(double));
  bandits[u].noise = calloc(d, sizeof(double));
  bandits[u].costs = calloc(d, sizeof(double));
  bandits[u].noisy_costs = calloc(d, sizeof(double));

  if (bandits[u].neighbours == NULL || bandits[u].Y_uv  == NULL
      || bandits[u].W_uv    == NULL || bandits[u].noise == NULL
      || bandits[u].costs   == NULL || bandits[u].noisy_costs == NULL)
  {
    fprintf(stderr, "(malloc) init_VertexBandit\n");
    exit(EXIT_FAILURE);
  }

  int k = 0;
  for (int v=0; v<g->n; v++) /* Calcul des voisins */
  if (g->network[u][v]) bandits[u].neighbours[k++] = v;

  return;
}

void free_VertexBandit(struct VertexBandit *bandits, int u)
/* Libère la mémoire dédiée au bandit sur le sommet u */
{
  free(bandits[u].neighbours);
  free(bandits[u].Y_uv);
  free(bandits[u].W_uv);
  free(bandits[u].noise);
  free(bandits[u].costs);
  return free(bandits[u].noisy_costs);
}

struct VBPopulation *new_VBPopulation_set(struct graph *g, int k)
/* Renvoie un tableau de k nouvelles populations en adéquation avec le graphe G */
{
  struct VBPopulation *pop = malloc(k * sizeof(struct VBPopulation));
  if (pop == NULL) handle_error("(malloc) new_VBPopulation_set");

  for (int p=0; p<k; p++)
  {
    pop[p].bandits = malloc(g->n * sizeof(struct VertexPlayer));
    if (pop[p].bandits == NULL) handle_error("(malloc) new_VBPopulation_set");

    /* Masse */
    pop[p].mass = 1./k;

    /* Couples source/destination */
    int a, b;
    do {
      b = random() % g->n;
      a = random() % g->n;
      while (b == a) b = random() % g->n;
      pop[p].source = (a > b) ? b : a;
      pop[p].sink   = (a > b) ? a : b;
    } while (!connected(pop[p].source, pop[p].sink, g));

    pop[p].n = g->n;
    /* Joueurs */
    for (int u=0; u<g->n; u++) init_VertexBandit(pop[p].bandits, g, u);
    for (int u=0; u<g->n; u++) if (u<pop[p].source || u>pop[p].sink)
      pop[p].bandits[u].W_u = -INFINITY;
  }

  return pop;
}

struct VBPopulation *VBPopulation_from_VPPopulation(struct VPPopulation *pop, int k)
/* Renvoie un tableau de k nouvelles populations en adéquation
 * avec la population semi-bandit donnée en argument */
{
  struct VBPopulation *pop_bandits = malloc(k * sizeof(struct VBPopulation));
  if (pop_bandits == NULL) handle_error("(malloc) VBPopulation_from_VPPopulation");

  for (int p=0; p<k; p++)
  {
    pop_bandits[p].n      = pop[p].n;
    pop_bandits[p].mass   = pop[p].mass;
    pop_bandits[p].sink   = pop[p].sink;
    pop_bandits[p].source = pop[p].source;

    pop_bandits[p].bandits = malloc(pop_bandits[p].n * sizeof(struct VertexBandit));
    if (pop_bandits[p].bandits == NULL)
      handle_error("(malloc) VBPopulation_from_VPPopulation");
    for (int u=0; u<pop_bandits[p].n; u++)
    {
      pop_bandits[p].bandits[u].d = pop[p].players[u].d;
      pop_bandits[p].bandits[u].W_u  = pop[p].players[u].W_u;
      pop_bandits[p].bandits[u].W_uv = pop[p].players[u].W_uv;
      pop_bandits[p].bandits[u].Y_uv = pop[p].players[u].Y_uv;
      pop_bandits[p].bandits[u].neighbours = pop[p].players[u].neighbours;

      int d = pop_bandits[p].bandits[u].d;
      pop_bandits[p].bandits[u].noise = calloc(d, sizeof(double));
      pop_bandits[p].bandits[u].costs = calloc(d, sizeof(double));
      pop_bandits[p].bandits[u].noisy_costs = calloc(d, sizeof(double));
    }
  }

  return pop_bandits;
}

void free_VBPopulation_set(struct VBPopulation *pop, int k)
/* Libère la mémoire dédiée à k populations */
{
  for (int p=0; p<k; p++)
  {
    for (int u=0; u<pop[p].n; u++) free_VertexBandit(pop[p].bandits, u);
    free(pop[p].bandits);
  }
  return free(pop);
}

void partial_free_VBPopulation_set(struct VBPopulation *pop, int k)
/* Ne libère que les champs noise, costs, et noisy_costs */
{
  for (int p=0; p<k; p++)
  {
    for (int u=0; u<pop[p].n; u++)
    {
      free(pop[p].bandits[u].noise);
      free(pop[p].bandits[u].costs);
      free(pop[p].bandits[u].noisy_costs);
    }
    free(pop[p].bandits);
  }
  return free(pop);
}

void reset_VBPopulation_set(struct VBPopulation *pop, int k)
/* Remet à 0 les évaluations d'un ensemble de populations */
{
  for (int p=0; p<k; p++)
  for (int u=0; u<pop[p].n; u++)
  for (int v=0; v<pop[p].bandits[u].d; v++)
  {
    pop[p].bandits[u].Y_uv[v] = 0;
    pop[p].bandits[u].W_uv[v] = 0;
    /* Devrait suffire ... qed de noise cost et noisy_costs ? */
  }
  return ;
}

/* #################### FONCTIONS USUELLES #################### */

void normalize_VBPopulation_set(struct VBPopulation *pop, int k)
/* Normalise, ensembles, les k premières populations */
{
  double tot_mass = 0;
  for (int p=0; p<k; p++) tot_mass += pop[p].mass;
  for (int p=0; p<k; p++) pop[p].mass /= tot_mass;
  return;
}

void set_VBPopulation_mass(double mass, struct VBPopulation *pop, int i)
/* Met la masse d'une population donnée à la quantité demandée */
/* Ne renormalise pas */
{
  pop[i].mass = mass;
  return;
}

void reset_VBPopulation_noisy_costs(struct VBPopulation *pop, int k)
/* Remet à 0 les coûts bruités de tous les individus */
{
  for (int p=0; p<k; p++)
  for (int u=0; u<pop[p].n; u++)
  for (int v=0; v<pop[p].bandits[u].d; v++)
    pop[p].bandits[u].noisy_costs[v] = 0;
  return ;
}


/* #################### FONCTIONS D'ÉVALUATION #################### */

double *VertexBandit_distrib(int u, struct VertexBandit *bandits, double e,
                             int noise)
/* Calcule la distribution du bandit u */
{
  double *distrib = new_distrib(bandits[u].d);
  pos_balanced_logit(distrib, bandits[u].W_uv, e, bandits[u].d);

  if (noise == WITH_NOISE)
  {
    /* Génération d'un bruit z dans la sphère de dimension d */
    /* Puis X_u <- X_u + e.z_u. Ce bruit est sauvegardé dans bandit[u].noise */
    double *noise = spherical_noise(bandits[u].d);
    for (int i=0; i<bandits[u].d; i++) bandits[u].noise[i] = noise[i];
    for (int i=0; i<bandits[u].d; i++) distrib[i] += e * noise[i];
    free(noise);
  }

  return distrib;
}



double **bandit_mass_spread(int p, struct VBPopulation *pop, double e, int noise)
/* Renvoie la matrice de la répartition de masse faite par la population i */
/* Spécifier NO_NOISE pour ne pas rajouter de bruit, WITH_NOISE sinon. */
{
  int n = pop[p].n;

  double **mass = malloc(n * sizeof(double*));
  double  *local_mass = calloc(n, sizeof(double));
  if (mass == NULL || local_mass == NULL) handle_error("(malloc) bandit_mass_spread");
  for (int u=0; u<n; u++) mass[u] = calloc(n, sizeof(double));

  local_mass[pop[p].source] = pop[p].mass;

  /* La masse passant en un nœud i est la somme mass[.<i][i] */
  for (int u=pop[p].source; u<pop[p].sink; u++)
  if (pop[p].bandits[u].W_u != -INFINITY)
  {
    /* On calcule la distribution, puis la masse locale */
    double *distrib = VertexBandit_distrib(u, pop[p].bandits, e, noise);

    /* Propagation de la masse */
    for (int k=0; k<pop[p].bandits[u].d; k++)
    {
      int v = pop[p].bandits[u].neighbours[k];
      mass[u][v] = local_mass[u] * distrib[k];
      local_mass[v] += mass[u][v];
    }

    free(distrib);
  }

  free(local_mass);

  return mass;
}

void bandit_measure_costs(struct VBPopulation *pop, int k,
                          struct Network *net, double epsilon)
/* Mesure les coûts purs pour une population dans un réseau */
{
  reset_masses(net); /* Recalcul de la masse dans le graphe */
  for (int p=0; p<k; p++)
  {
    double **pop_mass = bandit_mass_spread(p, pop, epsilon, NO_NOISE);
    for (int u=0; u<net->n; u++)
    for (int v=0; v<net->n; v++)
      net->masses[u][v] += pop_mass[u][v];

    free_cost_matrix(pop_mass, net->n);
  }

  double **costs = cost_matrix(net);
  for (int p=0; p<k; p++)
  for (int u=pop[p].source; u<pop[p].sink; u++)
  for (int i=0; i<pop[p].bandits[u].d; i++)
  {
    int v = pop[p].bandits[u].neighbours[i];
    pop[p].bandits[u].costs[i] = costs[u][v];
  }

  return free_cost_matrix(costs, net->n);
}

void bandit_add_noisy_measure(struct VBPopulation *pop, int k,
                              struct Network *net, double epsilon)
/* Rajoute une mesure bruitée (pour l'approximation du gradient */
{
  /* On calcule un bruit pour tous les joueurs */
  for (int p=0; p<k; p++)
  for (int u=pop[p].source; u<pop[p].sink; u++)
  {
    int d = pop[p].bandits[u].d;
    double *z = spherical_noise(d);
    for (int i=0; i<d; i++)
    {
      int v = pop[p].bandits[u].neighbours[i];
      double x_uv = net->masses[u][v];
      dtod_t c_uv = net->cost[u][v];
      /*pop[p].bandits[u].noisy_costs[i] += z[i] * (c_uv(x_uv + epsilon * z[i])
                                                 -c_uv(x_uv - epsilon * z[i]));*/
      /*double noisy_x_uv = x_uv + epsilon * z[i];*/
      pop[p].bandits[u].noisy_costs[i] +=
        z[i] * (c_uv(x_uv + epsilon * z[i])   * (x_uv + epsilon * z[i])
                - c_uv(x_uv - epsilon * z[i]) * (x_uv - epsilon * z[i]));
    }
    free(z);
  }

  return ;
}

void bandit_update_scores(struct VBPopulation *pop, int pop_size, struct Network *net,
                          double gamma, double epsilon, int k)
/* Actualise les Y_uv, W_uv, W_u de tous les joueurs */
{
  net = net; // lol
  for (int p=0; p<pop_size; p++) /* On respecte l'ordre topologique */
  for (int u=pop[p].n-1; u>=0; u--)
  {
    if (u == pop[p].sink)
    {
      pop[p].bandits[u].W_u = 0;
      continue;
    }
    if (u > pop[p].sink || u < pop[p].source)
    {
      pop[p].bandits[u].W_u = -INFINITY;
      continue;
    }

    int d = pop[p].bandits[u].d;
    for (int i=0; i<d; i++)
    /* MàJ des Y_uv et des W_uv */
    {
      int v = pop[p].bandits[u].neighbours[i];
      /*double raw_cost = pop[p].bandits[u].costs[i];
      double grad_estimate = d * pop[p].bandits[u].noisy_costs[i]
                             / (2 * epsilon * k);
      pop[p].bandits[u].Y_uv[i] += gamma *
                                   (raw_cost + net->masses[u][v] * grad_estimate);*/
      double grad_estimate = d * pop[p].bandits[u].noisy_costs[i] / (epsilon * 2 * k);
      pop[p].bandits[u].Y_uv[i] += gamma * grad_estimate;
      pop[p].bandits[u].W_uv[i] = pop[p].bandits[v].W_u - pop[p].bandits[u].Y_uv[i];
    }

    /* Calcul de W_u */
    double W_max = max(pop[p].bandits[u].W_uv, d);
    if (W_max == -INFINITY) /* Cas d'un nœud qui n'atteint pas 'sink' */
    {
      pop[p].bandits[u].W_u = -INFINITY;
      continue;
    }

    pop[p].bandits[u].W_u = 0;
    for (int i=0; i<d; i++)
      pop[p].bandits[u].W_u += exp(pop[p].bandits[u].W_uv[i] - W_max);
    pop[p].bandits[u].W_u = W_max + log(pop[p].bandits[u].W_u);
  }

  return ;
}

/* ************************ DEBUG : AFFICHAGE ************************ */

void print_VertexBandit(struct VertexBandit *bandits, int u, double e)
/* Affiche le u-ième bandit */
{
  double *distrib = VertexBandit_distrib(u, bandits, e, NO_NOISE);

  int d = bandits[u].d;
  printf("@@@@@(%d) : ", u);
  for (int i=0; i<d; i++)
    printf("%.3f(%d)[%.3f]{%.3f} ", distrib[i], bandits[u].neighbours[i],
           bandits[u].W_uv[i], bandits[u].Y_uv[i]);
  printf("\n");

  free(distrib);
  return ;
}

void print_VBPopulation(struct VBPopulation *pop, int k, double e)
{
  for (int p=0; p<k; p++)
  {
    printf("==== POP %d\n", p);
    for (int u=pop[p].source; u<pop[p].sink; u++)
      print_VertexBandit(pop[p].bandits, u, e);
  }
  return ;
}
