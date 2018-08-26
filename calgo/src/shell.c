#include "shell.h"
#include <time.h>

#define handle_error(s) do {fprintf(stderr, #s "\n"); exit(EXIT_FAILURE); } while(0);



/* **************** CONSTANTES DE SIMULTATION **************** */

static double alpha = 1./3.;//1./4.;
static double beta  = 1;//0.52;//1.;
static double cst_epsilon = 1e-5;//1e-9;
static double cst_gamma   = 1;

static double epsilon_iter(int n)
{
  return cst_epsilon / pow((double) n + 1, alpha);
}

static double gamma_iter(int n)
{
  return cst_gamma / pow((double) n + 1, beta);
}

struct Shell *new_Shell(void)
{
  struct Shell *sh = malloc(sizeof (struct Shell));
  if (sh == NULL) handle_error("(malloc) new_Shell");

  sh->nPlayers = sh->nGraph = sh->exists_token = FALSE;
  sh->initialized_network = sh->initialized_players = FALSE;
  sh->g   = NULL;
  sh->net = NULL;
  sh->players = NULL;
  sh->exec_mode = MODE_PATHS;

  return sh;
}

void free_Shell(struct Shell *sh)
{
  if (sh->g != NULL)       free_graph(sh->g);
  if (sh->net != NULL)     free_Network(sh->net);
  if (sh->players != NULL) free(sh->players);

  return free(sh);
}

int next_token(struct Shell *sh)
{
  char c; int k = 0;
  while ((c = getchar()) != EOF && k < 1023)
  {
    //printf("\'%c\'(%d)\n", c, c);
    if (c == ' ' && k)  break;
    if (c == ' ' && !k) continue;
    if (c == '\n') break;
    sh->token[k++] = c;
  }
  sh->token[k] = '\0';
  sh->exists_token = !(!k || c == EOF || c == '\n');
  return (!k || c == EOF || c == '\n');
}

void read_cmd(struct Shell *sh)
{
  printf("[%d]> ", sh->nGraph);
  while(1)
  {
    int c = next_token(sh);
    if (!c) printf("Token \"%s\"\n", sh->token);
    if (c)  printf("Last Token : \"%s\"\n", sh->token);
    if (c) break;
  }
  sh->nGraph++;
  return;
}

/* ************************ MANIPULATION DES TOKENS ************************ */

int empty_token(const char *token)
/* Renvoie 1 si sh->token est le token vide. Renvoie 0 sinon */
{
  return (*token == '\0');
}

int cmp_token(const char *token1, const char *token2)
/* Compare sh->token avec la chaîne de caractères en argument
 * Renvoie 1 si elles sont égales, 0 sinon */
{
  /*if (!strcmp(token1, token2)) printf("\"%s\" == \"%s\"\n", token1, token2);
  else printf("\"%s\" != \"%s\"\n", token1, token2);*/
  return ! strcmp(token1, token2);
}

void flush_tokens(struct Shell *sh)
/* Vide stdin */
{
  while (sh->exists_token) next_token(sh);
}

/* ************************ MANIPULATION DES COMMANDES ************************ */

int treat_cmd(struct Shell *sh)
/* *** FONCTION PRINCIPALE *** */
{
  int ret_value;
  printf("[%d]> ", sh->nGraph++);
  next_token(sh);

  /* Reconnaissance du token */
  if (empty_token(sh->token)) return EMPTY;
  else if (cmp_token(sh->token, "//")) ret_value = 0;
  else if (cmp_token(sh->token, "quit") || cmp_token(sh->token, "q")) exit(quit(sh));
  else if (cmp_token(sh->token, "new")) ret_value = new_smg(sh);
  else if (cmp_token(sh->token, "run")) ret_value = run(sh);
  else if (cmp_token(sh->token, "print")) ret_value = print(sh);
  else if (cmp_token(sh->token, "set"))   ret_value = set(sh);
  else if (cmp_token(sh->token, "unset")) ret_value = 0;
  else if (cmp_token(sh->token, "mode"))  ret_value = change_mode(sh);
  else if (cmp_token(sh->token, "memcheck"))  ret_value = 0;
  else if (cmp_token(sh->token, "connected")) ret_value = 0;
  else if (cmp_token(sh->token, "path")) ret_value = 0;
  else if (cmp_token(sh->token, "help")) ret_value = 0;
  else ret_value = unknown(sh);

  flush_tokens(sh);

  return ret_value;
}

int quit(struct Shell *sh)
{
  fprintf(stdout, "Exiting...\n");
  free_Shell(sh);
  return NORMAL;
}

int unknown(struct Shell *sh)
{
  fprintf(stderr, "Unknown token : \"%s\"\n", sh->token);
  return UNKNOWN;
}

/* Création */
int new_smg(struct Shell *sh)
/* New something : fonction qui repère ce que l'on crée */
{
  if (sh->exists_token) next_token(sh);
  else                  return NOTOKEN;

  if (cmp_token(sh->token, "graph"))   return shell_new_graph(sh);
  if (cmp_token(sh->token, "network")) return shell_new_network(sh);
  if (cmp_token(sh->token, "players")) return shell_new_players(sh);

  return unknown(sh);
}

int shell_new_graph(struct Shell *sh)
/* Va lire deux arguments : un entier et un flottant */
/* Possiblement d'autre paramètres sur la nature du graphe */
{
  int n;
  double p;
  if (sh->exists_token) next_token(sh);
  else { fprintf(stderr, "Expected int\n"); return NOTOKEN; }
  n = atoi(sh->token); /* Taille du graphe */

  if (sh->exists_token)
  /* Probabilités sur les arcs */
  {
    next_token(sh);
    p = atof(sh->token);
  }
  else p = 0.5;

  sh->initialized_players = FALSE;
  sh->initialized_network = FALSE;

  sh->g = new_graph(n);
  set_randDAG(sh->g, p);
  return NORMAL;
}

int shell_new_network(struct Shell *sh)
{
  if (sh->g == NULL) { fprintf(stderr, "No graph.\n"); return MISSING; }

  /* Potentielle libération */
  if (sh->net != NULL) free_Network(sh->net);

  sh->net = new_Network(sh->g->n); sh->initialized_network = FALSE;
  network_get_graph(sh->net, sh->g);
  return NORMAL;
}

int shell_new_players(struct Shell *sh)
/* Nouveaux joueurs [nombre] */
{
  int n;
  if (sh->g == NULL) { fprintf(stderr, "No graph yet. Please give one\n");
                       return MISSING; }

  if (sh->exists_token) next_token(sh);
  else { fprintf(stderr, "Expected int\n"); return NOTOKEN; }
  n = atoi(sh->token);

  sh->initialized_players = TRUE;
  sh->nPlayers = n;
  sh->players  = malloc(n * sizeof(struct ShellPlayer));
  if (sh->players == NULL) { fprintf(stderr, "(malloc) shell_new_players\n");
                             exit(EXIT_FAILURE); }

  for (int i=0; i<n; i++)
  {
    struct Couple ss = connected_couple_DAG(sh->g);
    sh->players[i].source = ss.left;
    sh->players[i].sink   = ss.right;
    sh->players[i].mass   = 1.;
  }

  return NORMAL;
}

/* ************************** SETTINGS ************************** */

int set(struct Shell *sh)
/* Fonction maîtresse */
/* C'est là qu'on va pouvoir initialiser donner des valeurs etc. */
{
  if (sh->exists_token) next_token(sh);
  else { fprintf(stderr, "Expected token.\n"); return NOTOKEN; }

  if (cmp_token(sh->token, "network")) set_network(sh);
  else if (cmp_token(sh->token, "player")) set_player(sh);
  else if (cmp_token(sh->token, "graph"))  set_graph(sh);
  else if (cmp_token(sh->token, "mass")) set_mass(sh);
  else if (cmp_token(sh->token, "beta")) set_beta(sh);
  else if (cmp_token(sh->token, "cst_gamma")) set_cst_gamma(sh);
  else unknown(sh);

  return NORMAL;
}

int set_network(struct Shell *sh)
/* Paramétrage du réseau (fonctions de coût) */
{
  if (sh->net == NULL) { fprintf(stderr, "No network\n"); return NOOBJECT; }

  if (sh->exists_token) next_token(sh);
  else return NORMAL;

  if (cmp_token(sh->token, "constant"))
  {
    sh->initialized_network = 1;
    set_allfun (sh->net, fun_cst);
    set_alldfun(sh->net, fun_dcst);
    set_alld2fun(sh->net, fun_d2cst);
    return NORMAL;
  }
  else if (cmp_token(sh->token, "linear"))
  {
    sh->initialized_network = 1;
    set_allfun (sh->net, fun_lin);
    set_alldfun(sh->net, fun_dlin);
    set_alld2fun(sh->net, fun_d2lin);
    return NORMAL;
  }
  else if (cmp_token(sh->token, "inverse"))
  {
    sh->initialized_network = 1;
    set_allfun (sh->net, fun_inv);
    set_alldfun(sh->net, fun_dinv);
    return NORMAL;
  }
  else if (cmp_token(sh->token, "inverse2"))
  {
    sh->initialized_network = 1;
    set_allfun (sh->net, fun_inv2);
    set_alldfun(sh->net, fun_dinv2);
    return NORMAL;
  }
  else if (cmp_token(sh->token, "affine"))
  {
    sh->initialized_network = 1;
    set_allfun (sh->net, fun_aff);
    set_alldfun(sh->net, fun_daff);
    set_alld2fun(sh->net, fun_d2aff);
    return NORMAL;
  }
  else if (cmp_token(sh->token, "poly3"))
  {
    sh->initialized_network = 1;
    set_allfun (sh->net, fun_deg3);
    set_alldfun(sh->net, fun_ddeg3);
    set_alld2fun(sh->net, fun_d2deg3);
    return NORMAL;
  }
  else return UNKNOWN;
}

int set_player(struct Shell *sh)
{
  if (!sh->initialized_players) { fprintf(stderr, "No players.\n"); return MISSING; }

  int i, source, sink;

  if (sh->exists_token) next_token(sh);
  else { fprintf(stderr, "Expected int.\n"); return NOTOKEN; }
  i = atoi(sh->token); /* Numéro du joueur */
  if (i >= sh->nPlayers) { fprintf(stderr, "%d out of length.\n", i); return NORMAL;}

  /* Lecture du couple source / destination */
  if (sh->exists_token) next_token(sh);
  else { fprintf(stderr, "Expected source.\n"); return NOTOKEN; }
  source = atoi(sh->token);

  if (sh->exists_token) next_token(sh);
  else { fprintf(stderr, "Expected sink.\n"); return NOTOKEN; }
  sink = atoi(sh->token);

  sh->players[i].sink = sink;
  sh->players[i].source = source;

  return NORMAL;
}

int set_graph(struct Shell *sh)
{
  if (sh->exists_token) next_token(sh);
  else { fprintf(stderr, "Expected int.\n"); return NOTOKEN; }
  int n = atoi(sh->token);

  /* Libération potentielle */
  if (sh->g != NULL)    free_graph(sh->g);

  sh->g = new_graph(n);
  for (int i=0; i<n; i++)
  for (int j=0; j<n; j++)
  {
    int coef;
    if (!scanf("%d", &coef)) { fprintf(stderr, "Scanf"); return MISSING; };
    sh->g->network[i][j] = coef;
  }

  sh->initialized_players = FALSE;
  free(sh->players);
  sh->players = NULL;

  return NORMAL;
}

int set_mass(struct Shell *sh)
{
  if (sh->exists_token) next_token(sh);
  else { fprintf(stderr, "Expected float"); return NOTOKEN; }

  double mass = atof(sh->token);
  for (int p=0; p<sh->nPlayers; p++) sh->players[p].mass= mass;

  return NORMAL;
}

int set_beta(struct Shell *sh)
{
  if (sh->exists_token) next_token(sh);
  else { fprintf(stderr, "Expected float\n"); return NOTOKEN; }

  beta = atof(sh->token);

  return NORMAL;
}

int set_cst_gamma(struct Shell *sh)
{
  if (sh->exists_token) next_token(sh);
  else { fprintf(stderr, "Expected float\n"); return NOTOKEN; }

  cst_gamma = atof(sh->token);

  return NORMAL;
}

/* ************************** SIMULATION ************************** */

/* Conversion utiles pour les simulations */
struct SBPlayer *ShellPlayers_to_SBPlayers(struct Shell *sh, int n,
                                           int **vertices)
/* Étant donnés 'n' joueurs type shell, renvoie une structure équivalente
 * de 'n' joueurs type sb et les initialise pour se préparer à une simulation.
 * A besoin de structures auxiliaires. */
{
  struct SBPlayer *sbplayers = new_SBPlayers(n);

  for (int i=0; i<n; i++) set_SBPlayer(i, sbplayers, sh->players[i].source,
                                       sh->players[i].sink, sh->players[i].mass,
                                       sh->g, vertices);

  return sbplayers;
}

struct VPPopulation *ShellPlayers_to_VPPopulation(struct Shell *sh, int n)
/* Étant donnés 'n' joueurs type shell, renvoie une structure équivalente
 * de 'n' popuations type vertex et les initialise pour se préparer à une
 * simulation.
 * A besoin de structures auxiliaires. */
{
  struct VPPopulation *pop = malloc(n * sizeof(struct VPPopulation));
  if (pop == NULL) { fprintf(stderr, "(malloc) ShellPlayers_to_VPPopulation\n");
                     exit(EXIT_FAILURE); }

  for (int p=0; p<n; p++)
  {
    pop[p].players = calloc(sh->g->n, sizeof(struct VertexPlayer));
    if (pop[p].players == NULL)
    {
      fprintf(stderr, "(calloc) ShellPlayers_to_VPPopulation\n");
      exit(EXIT_FAILURE);
    }

    /* Informations sur la population */
    pop[p].mass   = sh->players[p].mass;
    pop[p].source = sh->players[p].source;
    pop[p].sink   = sh->players[p].sink;
    pop[p].n      = sh->g->n;

    /* Joueurs */
    for (int i=0; i<sh->g->n; i++) init_VertexPlayer(pop[p].players, sh->g, i);
    for (int i=0; i<sh->g->n; i++) if (i<pop[p].source || i>pop[p].sink)
      pop[p].players[i].W_u = -INFINITY;
  }

  return pop;
}

struct VBPopulation *ShellPlayers_to_VBPopulation(struct Shell *sh, int n)
/* Étant donnés 'n' joueurs type shell, renvoie une structure équivalente
 * de 'n' popuations type BANDIT et les initialise pour se préparer à une
 * simulation.
 * A besoin de structures auxiliaires. */
{
  struct VBPopulation *pop = malloc(n * sizeof(struct VBPopulation));
  if (pop == NULL) { fprintf(stderr, "(malloc) ShellPlayers_to_VBPopulation\n");
                     exit(EXIT_FAILURE); }

  for (int p=0; p<n; p++)
  {
    pop[p].bandits = calloc(sh->g->n, sizeof(struct VertexBandit));
    if (pop[p].bandits == NULL)
    {
      fprintf(stderr, "(calloc) ShellPlayers_to_VBPopulation\n");
      exit(EXIT_FAILURE);
    }

    /* Initialisation : masse, source/destination, nombres de joueurs */
    pop[p].mass   = sh->players[p].mass;
    pop[p].source = sh->players[p].source;
    pop[p].sink   = sh->players[p].sink;
    pop[p].n = sh->g->n;

    /* Joueurs */
    for (int u=0; u<sh->g->n; u++) init_VertexBandit(pop[p].bandits, sh->g, u);
    for (int u=0; u<sh->g->n; u++) if (u<pop[p].source || u>pop[p].sink)
      pop[p].bandits[u].W_u = -INFINITY;
  }

  return pop;
}

/* *************** FONCTIONS AUXILIARES DE SIMULATION *************** */

static int has_converged(struct Shell *sh, double epsilon, void *players,
                         double **cost_mat)
/* Regarde si l'état courant est un epsilon-équilibre.
 * Renvoie 1 si c'est le cas, 0 sinon. */
{
  for (int i=0; i<sh->nPlayers; i++)
  {
    if (sh->exec_mode & MODE_VERTEX)
    {
      struct VPPopulation *pop = (struct VPPopulation *) players;
      double **mass = mass_spread(i, pop, 0);
      if (!convergence_on(pop[i].source, pop[i].sink, mass, epsilon,
                          cost_mat, sh->g))
      {
        free_cost_matrix(mass, sh->g->n);
        return 0;
      }
      free_cost_matrix(mass, sh->g->n);
    }
    else if (sh->exec_mode & MODE_PATHS)
    {
      struct SBPlayer *pop = (struct SBPlayer *) players;
      double **mass = paths_mass_spread(i, pop, sh->g->n);
      if (!convergence_on(pop[i].source, pop[i].sink, mass, epsilon,
                          cost_mat, sh->g))
      {
        free_cost_matrix(mass, sh->g->n);
        return 0;
      }
      free_cost_matrix(mass, sh->g->n);
    }
    else if (sh->exec_mode & MODE_BANDIT)
    {
      struct VBPopulation *pop = (struct VBPopulation *) players;
      double **mass = bandit_mass_spread(i, pop, 0, NO_NOISE);
      if (!convergence_on(pop[i].source, pop[i].sink, mass, epsilon,
                          cost_mat, sh->g))
      {
        free_cost_matrix(mass, sh->g->n);
        return 0;
      }
      free_cost_matrix(mass, sh->g->n);
    }
  }
  return 1;
}

/* *************** SIMULATION PATHS *************** */

static int shell_simu_sb(struct Shell *sh)
/* Fait la simulation */
{
  if (sh->g == NULL)   { fprintf(stderr, "No graph.\n");   return MISSING; }
  if (sh->net == NULL) { fprintf(stderr, "No network.\n"); return MISSING; }
  if (!sh->initialized_network) { fprintf(stderr, "Uninitialized network.\n");
                                  return MISSING; }
  if (sh->players == NULL) { fprintf(stderr, "No players.\n");  return MISSING; }
  if (!sh->initialized_players) { fprintf(stderr, "Uninitialized players.\n");
                                  return MISSING; }

  /* Remarque : si les joueurs sont non-initialisés, se préparer à une explosion
   * de même que si le network n'est pas initalisé... */

  int **vertices = vertices_array(sh->g->n);
  struct SBPlayer *sb_players = ShellPlayers_to_SBPlayers(sh, sh->nPlayers,
                                                          vertices);

  clock_t t0 = clock();

  for (int iter=0; sh->exec_mode & STOP || iter<sh->nIter; iter++)
  /* Boucle principale */
  {

    //aff_SBPlayer_score(0, sb_players);

    reset_masses(sh->net); /* Reset des masses */
    for (int i=0; i<sh->nPlayers; i++) /* Calcul des distributions - MàJ des masses */
    {
      double *distrib = SBPlayer_distrib(i, sb_players, 0);
      add_mass_of_player(sh->net, sb_players[i].mass,
                         sb_players[i].paths, distrib);
      free(distrib);
    }
    double **cost_mat = mcost_matrix(sh->net); /* Précalcul de la matrice des coûts */
    if (iter && sh->exec_mode & STOP && has_converged(sh, sh->precision,
                                                      sb_players, cost_mat))
    {
      fprintf(stderr, "\x1b[1K\rConverged with %d steps.\n", iter + 1);
      free_cost_matrix(cost_mat, sh->g->n);
      break;
    }
    else if (sh->exec_mode & STOP && !(sh->exec_mode & SILENT))
      fprintf(stderr, "\x1b[1K\rDid not converged with %d steps", iter + 1);

    for (int i=0; i<sh->nPlayers; i++) /* Calcul des coûts - MàJ des évaluations */
    {
      double *distrib = fast_eval_player(i, sb_players, cost_mat);
      for (int j=0; j<sb_players[i].n; j++)
        sb_players[i].Y_uv[j] += distrib[j] * gamma_iter(iter);
      free(distrib);
    }
    if (sh->exec_mode & POTENTIAL) fprintf(stderr, "@%3d : potential = %.4f\n",
                         iter+1, net_potential(sh->net));


    free_cost_matrix(cost_mat, sh->g->n);
  }

  if (sh->exec_mode & POTENTIAL)
    for (int i=0; i<sh->nPlayers; i++) aff_SBPlayer_score(i, sb_players);

  if (sh->exec_mode & TIME)
  {
    clock_t t1 = clock();
    fprintf(stderr, "Time used : %lf\n", (t1 - t0) / (CLOCKS_PER_SEC * 1.));
  }


  free_vertices(vertices, sh->g->n);
  free_SBPlayers(sb_players, sh->nPlayers);
  return NORMAL;
}

/* *************** SIMULATION VERTEX *************** */
static int shell_simu_vertex(struct Shell *sh)
{
  /* Vérifications préliminaires pour éviter une explosion en vol */
  if (sh->g == NULL)   { fprintf(stderr, "No graph.\n"); return MISSING; }
  if (sh->net == NULL) { fprintf(stderr, "No network.\n"); return MISSING; }
  if (!sh->initialized_network) { fprintf(stderr, "Uninitialized network.\n");
                                  return MISSING; }
  if (sh->players == NULL) { fprintf(stderr, "No players.\n");  return MISSING; }
  if (!sh->initialized_players) { fprintf(stderr, "Uninitialized players.\n");
                                  return MISSING; }

  struct VPPopulation *v_players = ShellPlayers_to_VPPopulation(sh, sh->nPlayers);

  double t0 = clock();
  double previous_cc = 0;

  for (int iter=0; sh->exec_mode & (STOP | STOP_CCC) || iter<sh->nIter; iter++)
  /* Boucle principale */
  {
    reset_masses(sh->net);
    /* Calcul de la masse */
    for (int p=0; p<sh->nPlayers; p++)
    /* CALCUL DE LA MASSE & DISTRIBUTIONS : Parcourss de toutes les populations */
    {
      double **mass = mass_spread(p, v_players, 0);

      for (int i=0; i<sh->g->n; i++) for (int j=0; j<sh->g->n; j++)
        sh->net->masses[i][j] += mass[i][j];

      for (int i=0; i<sh->g->n; i++) free(mass[i]); /* Libération de mass */
      free(mass);
    }

    double **cost_mat = mcost_matrix(sh->net); /* Précalcul de la matrice des coûts */
    /* Ajustement de Gamma - seulement à la première itération */
    if (!iter && sh->exec_mode & GAMMA_CORRECTION)
    {
      cst_gamma = 1 / net_d2potential(sh->net);
      printf("Cst Gamma : %lf\n", cst_gamma);
    }
    else if (!iter)
    {
      printf("Cst Gamma : %lf\n", cst_gamma);
    }

    /* Convergence en distribution */
    if (iter && sh->exec_mode & STOP && has_converged(sh, sh->precision,
                                                      v_players, cost_mat))
    {
      if (!(sh->exec_mode & SILENT))
        fprintf(stderr, "\x1b[1K\rConverged with %d steps.\n", iter + 1);
      else fprintf(stderr, "Converged with %d steps.\n", iter + 1);
      free_cost_matrix(cost_mat, sh->g->n);
      break;
    }
    else if (sh->exec_mode & STOP && !(sh->exec_mode & SILENT))
      fprintf(stderr, "\x1b[1K\rDid not converged with %d steps", iter + 1);

    for (int p=0; p<sh->nPlayers; p++)
    for (int i=v_players[p].sink-1; i>=v_players[p].source; i--)
    /* CALCUL DES CoÜTS - MàJ des ÉVALUATIONS */
    /* Respecter l'ordre topologique ! */
    {
      int deg = v_players[p].players[i].d;
      double *costs = new_distrib(deg);
      for (int j=0; j<deg; j++)
      {
        int v = v_players[p].players[i].neighbours[j];
        costs[j] = gamma_iter(iter) * cost_mat[i][v];
      }
      update_eval_VertexPlayer(i, v_players[p].players, costs,
                               v_players[p].sink);
      free(costs);
    }

    /* AFFICHAGE DU POTENTIEL */
    if (sh->exec_mode & POTENTIAL) fprintf(stderr, "%d %f\n",
                         iter+1, net_potential(sh->net));

    /* Convergence en coût cumulé */
    if (sh->exec_mode & STOP_CCC)
    {
      double current_cc = net_potential(sh->net);
      double ccc = 100 * iter * (previous_cc - current_cc) / current_cc;
      //printf("%lf\n", ccc);
      if (iter > 1 && ccc >= 0 && ccc <= sh->precision)
      {
       if (!(sh->exec_mode & SILENT))
        fprintf(stderr, "\x1b[1K\rConverged with %d steps.\n", iter + 1);
       else fprintf(stderr, "Converged with %d steps.\n", iter + 1);
       free_cost_matrix(cost_mat, sh->g->n);
       break;
      }
      else if (!(sh->exec_mode & SILENT))
       fprintf(stderr, "\x1b[1K\rDid not converged with %d steps", iter + 1);
      previous_cc = current_cc;
    }

    free_cost_matrix(cost_mat, sh->g->n);


  }

  if (sh->exec_mode & TIME)
  {
    clock_t t1 = clock();
    fprintf(stderr, "Time used : %lf\n", (t1 - t0) / (CLOCKS_PER_SEC * 1.));
  }

  /* FIN : Libération & co */

  free_VPPopulation_set(v_players, sh->nPlayers);
  return NORMAL;
}


static int shell_simu_bandit(struct Shell *sh, int k)
/* La simulation du cas bandit */
{
  /* Vérifications préliminaires pour éviter une explosion en vol */
  if (sh->g == NULL)   { fprintf(stderr, "No graph.\n"); return MISSING; }
  if (sh->net == NULL) { fprintf(stderr, "No network.\n"); return MISSING; }
  if (!sh->initialized_network) { fprintf(stderr, "Uninitialized network.\n");
                                  return MISSING; }
  if (sh->players == NULL) { fprintf(stderr, "No players.\n");  return MISSING; }
  if (!sh->initialized_players) { fprintf(stderr, "Uninitialized players.\n");
                                  return MISSING; }

  struct VBPopulation *pop =
    ShellPlayers_to_VBPopulation(sh, sh->nPlayers);

  for (int iter=0; sh->exec_mode & STOP || iter<sh->nIter; iter++)
  /* Boucle principale */
  {
    if (sh->exec_mode & POTENTIAL)
    {
      bandit_measure_costs(pop, sh->nPlayers, sh->net, 0);
      fprintf(stderr, "%d %f\n", iter+1, net_potential(sh->net));
    }

    double **cost_mat = mcost_matrix(sh->net); /* Précalcul de la matrice des coûts */
    if (iter && sh->exec_mode & STOP && has_converged(sh, sh->precision,
                                                      pop, cost_mat))
    {
      fprintf(stderr, "\x1b[1K\rConverged with %d steps.\n", iter + 1);
      free_cost_matrix(cost_mat, sh->g->n);
      break;
    }
    else if (sh->exec_mode & STOP && !(sh->exec_mode & SILENT))
      fprintf(stderr, "\x1b[1K\rDid not converged with %d steps", iter + 1);
    free_cost_matrix(cost_mat, sh->g->n);

    bandit_measure_costs(pop, sh->nPlayers, sh->net, 0);
    if (isnan(net_potential(sh->net))) return NORMAL;

    reset_VBPopulation_noisy_costs(pop, sh->nPlayers);
    for (int i=0; i<k; i++)
      bandit_add_noisy_measure(pop, sh->nPlayers, sh->net, epsilon_iter(iter));


    bandit_update_scores(pop, sh->nPlayers, sh->net, gamma_iter(iter),
                         epsilon_iter(iter), k);

  }

  free_VBPopulation_set(pop, sh->nPlayers);

  return NORMAL;
}

static int shell_simu_queues(struct Shell *sh)
{
  if (sh->g == NULL)   { fprintf(stderr, "No graph.\n"); return MISSING; }
  if (sh->net == NULL) { fprintf(stderr, "No network.\n"); return MISSING; }
  if (!sh->initialized_network) { fprintf(stderr, "Uninitialized network.\n");
                                  return MISSING; }
  if (sh->players == NULL) { fprintf(stderr, "No players.\n");  return MISSING; }
  if (!sh->initialized_players) { fprintf(stderr, "Uninitialized players.\n");
                                  return MISSING; }

  struct SimulatedNetwork *snet = new_SimulatedNetwork(sh->g, sh->precision);

  /* Initialisation des flux */
  for (int p=0; p<sh->nPlayers; p++)
  {
    struct ShellPlayer player = sh->players[p];
    snet->lambda[player.source][player.sink] = player.mass;
  }

  /* Initialisation des nœuds */
  for (int u=0; u<snet->n; u++)
  {
    snet->vertex[u].mu = 1; /* FIXME : peut être variable */
    snet->vertex[u].n  = 1;
    /*update_distrib_SimulatedPlayer(snet->vertex, u, snet->vertex[u].n,
                                   sh->g->n);*/
    //print_distrib(snet->vertex[u].X_uv[sh->players[0].sink], snet->vertex[u].d);
  }

  /* Initialisation des événements */
  for (int p=0; p<sh->nPlayers; p++)
  {
    struct ShellPlayer player = sh->players[p];
    struct EventID ID  = new_EventID(0, player.source, player.sink);
    struct Event event = new_Event(0, player.source, NEW_PAQUET, player.sink, ID);
    add_Event(event, &snet->qevents);
  }

  for (int u=0; u<sh->g->n; u++)
  {
    struct EventID ID  = new_EventID(0, 0, 0);
    //printf("Adding event : UPDATE %d\n", u);
    struct Event upd_event = new_Event(0, u, UPDATE_DISTRIB, u, ID);
    add_Event(upd_event, &snet->qevents);
  }

  for (int i=0; i<snet->qevents->n; i++) print_Event(snet->qevents->events[i]);

  /* Simulation */
  for (int iter=0; iter<sh->nIter; iter++) treat_new_event(snet);

  spread_Simulated_mass(snet, sh->net);

  /*for (int p=0; p<sh->nPlayers; p++)
  {
    int s = sh->players[p].source;
    int t = sh->players[p].sink;
    for (int k=0; k<snet->vertex[s].d; k++)
    {
      int v = snet->vertex[s].neighbours[k];
      printf("---- Edge Y_%d-%d : %lf\n", s, v, snet->vertex[s].Y_uv[k]);
    }
    for (int u=sh->players[p].source; u<sh->g->n; u++)
      printf("W_%d^(%d) : %lf\n", u, t, snet->vertex[u].W_u[t]);
  }*/

  long long int worst = -1;
  int overloaded_node = -1;
  for (int u=0; u<snet->n; u++)
  {
    if (snet->vertex[u].n > worst)
    {
      worst = snet->vertex[u].n;
      overloaded_node = u;
    }
  }
  printf("Most loaded node : %d\n", overloaded_node);


  free_SimulatedNetwork(snet);
  sh->precision = 0.01;
  return NORMAL;
}

int run(struct Shell *sh)
{
  sh->exec_mode &= 0xf;
  sh->nIter = 100; sh->precision = 1e-2;
  // cst_gamma = 1;

  while (sh->exists_token)
  {
    next_token(sh);
    if (cmp_token(sh->token, "paths"))
      sh->exec_mode = MODE_PATHS  | (sh->exec_mode ^ (sh->exec_mode & 0xf));
    else if (cmp_token(sh->token, "vertex"))
      sh->exec_mode = MODE_VERTEX | (sh->exec_mode ^ (sh->exec_mode & 0xf));
    else if (cmp_token(sh->token, "bandit"))
      sh->exec_mode = MODE_BANDIT | (sh->exec_mode ^ (sh->exec_mode & 0xf));
    else if (cmp_token(sh->token, "simulation"))
    {
      sh->exec_mode = MODE_SIMU  | (sh->exec_mode ^ (sh->exec_mode & 0xf));
      sh->precision = 100;
    }
    else if (cmp_token(sh->token, "corrected"))
      sh->exec_mode |= GAMMA_CORRECTION;
    else if (cmp_token(sh->token, "silent"))
      sh->exec_mode = SILENT | sh->exec_mode;
    else if (cmp_token(sh->token, "potential")) sh->exec_mode |= POTENTIAL;
    else if (cmp_token(sh->token, "time")) sh->exec_mode |= TIME;
    else if (cmp_token(sh->token, "with"))
    {
      if (sh->exists_token) next_token(sh);
      else { fprintf(stderr, "Expected float\n"); return NOTOKEN; }

      sh->precision = atof(sh->token);
      sh->exec_mode |= STOP;
    }
    else if (cmp_token(sh->token, "ccc"))
    {
      if (sh->exists_token) next_token(sh);
      else { fprintf(stderr, "Expected float\n"); return NOTOKEN; }

      sh->precision = atof(sh->token);
      sh->exec_mode |= STOP_CCC;
    }
    else if (cmp_token(sh->token, "for"))
    {
      if (sh->exists_token) next_token(sh);
      else { fprintf(stderr, "Expected int\n"); return NOTOKEN; }

      sh->nIter = atoi(sh->token);
    }
  }

  if (sh->exec_mode & MODE_PATHS) shell_simu_sb(sh);
  else if (sh->exec_mode & MODE_VERTEX) shell_simu_vertex(sh);
  else if (sh->exec_mode & MODE_BANDIT) shell_simu_bandit(sh, 1);
  else if (sh->exec_mode & MODE_SIMU)   shell_simu_queues(sh);

  return NORMAL;
}

/* Affichage */
int print(struct Shell *sh)
{
  if (sh->exists_token) next_token(sh);
  else                  return NOTOKEN;

  if (cmp_token(sh->token, "graph"))     return shell_print_graph(sh);
  if (cmp_token(sh->token, "network"))   return shell_print_network(sh);
  if (cmp_token(sh->token, "players"))   return shell_print_players(sh);
  if (cmp_token(sh->token, "player"))    return shell_print_player (sh);
  if (cmp_token(sh->token, "potential")) return shell_print_potential(sh);
  if (cmp_token(sh->token, "score"))    return shell_print_scores (sh);
  if (cmp_token(sh->token, "mass"))      return shell_print_masses (sh);
  if (cmp_token(sh->token, "graphviz"))  return shell_graphviz(sh);
  if (cmp_token(sh->token, "mark")) { fprintf(stderr, "#\n"); return NORMAL; }

  return unknown(sh);
}

int shell_print_graph(struct Shell *sh)
{
  if (sh->g == NULL)
  {
    fprintf(stderr, "Unexisting graph\n");
    return NOOBJECT;
  }

  if (sh->exists_token) next_token(sh);
  if (cmp_token(sh->token, "links")) list_links(sh->g);
  else  aff_graph(sh->g, 0);

  return NORMAL;
}

int shell_print_network(struct Shell *sh)
{
  if (sh->net == NULL)
  {
    fprintf(stderr, "Unexisting network\n");
    return NOOBJECT;
  }

  fprintf(stderr, "Unexisting feature\n");
  return NORMAL;
}

int shell_print_players(struct Shell *sh)
/* Affichage de la liste des joueurs */
{
  if (!sh->initialized_players) { fprintf(stderr, "No players\n"); return NOOBJECT; }

  for (int i=0; i<sh->nPlayers; i++)
    printf("Player %d :\tSource: %d, Sink: %d, Mass:%lf\n",
           i, sh->players[i].source, sh->players[i].sink, sh->players[i].mass);

  return NORMAL;
}

int shell_print_player(struct Shell *sh)
/* Affichage d'un joueur */
{
  sh = sh;
  printf("TODO.\n");
  return NORMAL;
}

int shell_print_scores(struct Shell *sh)
/* Affiche les scores des joueurs */
{
  sh = sh;
  printf("TODO.\n");
  return NORMAL;
}

int shell_print_masses(struct Shell *sh)
/* Affiche les masses dans le graphe */
{
  if (sh->net == NULL) { fprintf(stderr, "No network.\n"); return MISSING; }

  aff_masses(sh->net);
  return NORMAL;
}

int shell_graphviz(struct Shell *sh)
{
  int light = FALSE;
  if (sh->exists_token)
  {
    next_token(sh);
    if (cmp_token(sh->token, "light")) light = TRUE;
  }


  printf("digraph G {\n  rankdir=LR\n");


  printf("  node [shape=doublecircle]; ");
  for (int i=0; i<sh->nPlayers; i++)
    printf("%d ", sh->players[i].source);

  printf(";\n  node [shape=doublecircle, color=red]; ");
  for (int i=0; i<sh->nPlayers; i++)
    printf("%d ", sh->players[i].sink);

  printf(";\n  node [shape = circle, color = black];\n");

  int m = 0;
  for (int u=0; u<sh->g->n; u++)
  for (int v=0; v<sh->g->n; v++)
  if (sh->g->network[u][v]) m++;

  double min_mass = sh->precision / m;

  double max_mass = 0;
  for (int i=0; i<sh->g->n; i++) for (int j=0; j<sh->g->n; j++)
  if (sh->net->masses[i][j] > max_mass) max_mass = sh->net->masses[i][j];
  if (max_mass - min_mass < 1e-3) min_mass = max_mass / 2;

  for (int i=0; i<sh->g->n; i++) for (int j=0; j<sh->g->n; j++)
  if (sh->net->masses[i][j] > 0)
  {
    double mass = sh->net->masses[i][j];
    int r = 230 - 230 * (mass - min_mass) / (max_mass - min_mass);
    int g = r;
    int b = r;
    if (mass/max_mass < 0.05 || r > 225)
    {
      if (light) continue;
      printf("  %d -> %d [style=dashed, color=\"#%2x%2x%2x\"]\n", i, j, 225, 225, 225);
      continue;
    }
    printf("  %d -> %d [label=\"%.3f\" color=\"#%2x%2x%2x\"]\n", i, j, mass, r, g, b);
  }
  printf("}\n");

  return NORMAL;
}

int shell_print_potential(struct Shell *sh)
{
  printf("\n---------------------Potential : %g\n", net_potential(sh->net));
  return NORMAL;
}

/* **** MODES **** */

int change_mode(struct Shell *sh)
/* change le mode du shell (un joueur par sommet par exemple) */
{
  if (sh->exists_token) next_token(sh);
  else if (sh->exec_mode & MODE_PATHS)  { printf("Mode: paths.\n");  return NORMAL; }
  else if (sh->exec_mode & MODE_VERTEX) { printf("Mode: vertex.\n"); return NORMAL; }

  if (cmp_token(sh->token, "vertex"))
  {
    sh->exec_mode = MODE_VERTEX;
    return NORMAL;
  }
  else if (cmp_token(sh->token, "paths"))
  {
    sh->exec_mode = MODE_PATHS;
    return NORMAL;
  }
  else return unknown(sh);

  return NORMAL;
}
