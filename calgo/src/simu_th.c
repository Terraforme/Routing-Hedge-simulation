#include "simu_th.h"

static double alpha = 1./4.;
static double beta  = 2./3.;
static double cst_epsilon = 1e-9;
static double cst_gamma   = 1;

static double epsilon_iter(int n)
{
  return cst_epsilon / pow((double) n + 1, alpha);
}

static double gamma_iter(int n)
{
  return cst_gamma / pow((double) n + 1, beta);
}

void eHedge_SBPlayers(struct graph *g, struct Network *net, int nPlayers,
                      int nIter)
/* Algorithme d'epsilon-Hedge sur nPlayers joueurs, sur nIter itérations */
{
  /* Initialisation */
  struct SBPlayer *players = new_SBPlayers(nPlayers);
  int **vertices = vertices_array(g->n);
  init_SBPlayers(players, g, nPlayers, vertices);
  normalize_SBPlayers(players, nPlayers);

  /* Pour moi : */
  aff_SBPlayers(players, nPlayers, 1);

  /* Itérations */
  for (int iter=0; iter<nIter; iter++)
  {
    reset_masses(net); /* Reset des masses */
    for (int i=0; i<nPlayers; i++) /* Calcul des distributions - MàJ des masses */
    {
      double *distrib = SBPlayer_distrib(i, players, epsilon_iter(iter));
      add_mass_of_player(net, players[i].mass, players[i].paths, distrib);
      free(distrib);
    }

    double **cost_mat = mcost_matrix(net); /* Précalcul de la matrice des coûts */
    for (int i=0; i<nPlayers; i++) /* Calcul des coûts - MàJ des évaluations */
    {
      double *distrib = fast_eval_player(i, players, cost_mat);
      for (int j=0; j<players[i].n; j++)
        players[i].Y_uv[j] += distrib[j] * gamma_iter(iter);
      free(distrib);
    }

    fprintf(stderr, "@%3d : potential = %.4f\n", iter+1, net_potential(net));
    free_cost_matrix(cost_mat, g->n);
  }

  for (int i=0; i<nPlayers; i++) aff_SBPlayer_score(i, players);

  free_SBPlayers(players, nPlayers);
  free_vertices(vertices, g->n);

  return ;
}
