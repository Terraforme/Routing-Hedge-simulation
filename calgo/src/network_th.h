#ifndef network_th_h
#define network_th_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "graph.h"
#include "list.h"


/* On définit ici une manière de simuler les réseaux */
/* ICI, on se contente de l'approche THÉORIQUE, i.e un réseau sera un graphe
 * parcouru par des masses et doté de fonctions pour chaque arête. */

/* On donne quelques fonctions usuelles pour pouvoir jouer avec le réseau
 * sans se casser la tête */

typedef double (*dtod_t) (double); /* Type d'un pointeur d'une fct double -> double */

struct Network
{
  int **graph;     /* Le graphe est représenté par une matrice d'adjacence */
  double **masses; /* matrice des masses */
  dtod_t ** cost;  /* Matrice des fonctions de coût */
  dtod_t **dcost;  /* Matrices des dérivées des fonctions de coûts */
  dtod_t **d2cost; /* Matrices des dérivées secondes */
  int mode;
  int n; /* taille du graphe */
};

/* ***************** Fonctions administratives ***************** */

struct Network *new_Network(int n); /* Renvoie un nouveau réseau vide */
void free_Network(struct Network *net); /* Libère la mémoire dédiée à un réseau */

void set_netfun (int i, int j, struct Network *net, dtod_t fun);
/* met le fonction i,j à fun */
void set_netdfun(int i, int j, struct Network *net, dtod_t fun);
/* met la dérivée  i,j à fun */
void network_get_graph(struct Network *net, struct graph *g);
/* Copie le graphe dans le réseau - pas forcément utile, mais ... */

/* ***************** Fonctions Basiques ***************** */

void reset_network(struct Network *net);
/* Efface toutes les masses, état du graphe, et pointeurs de fonctions */
void reset_masses (struct Network *net);
/* Remet toutes les masses à zéro */

void add_mass_over(struct Network *net, double mass, struct List *path);
/* Distribue la masse 'mass' sur le chemin en argument */
void add_mass_of_player(struct Network *net, double pmass,
                        struct List *paths, double *distrib);
/* Étant donné tous les chemins possibles d'un joueur, la demande de ce joueur
 * et la manière dont il distribue la masse sur ses chemins possibles,
 * met à jour la masse dans le réseau */

void add_mass_on_links(struct Network *net, double pmass,
                       int origin, int *neighbours, int n,
                       double *distrib);
/* Étant donnés les liens origin -> neighbours[.], ajoute de la masse
 * selon la distribution donnée */

/* ***************** Fonctions de calcul des coûts ***************** */

double compute_path_cost(struct Network *net, struct List *path);
/* Calcule \sum_{e \in path} c(x_e) */
double compute_modified_path_cost(struct Network *net, struct List *path);
/* Calcule \sum_{e \in path} x_e c'(x_e) + c(x_e) */

double  **cost_matrix(struct Network *net); /* renvoie la matrice cost[i][j] */
double **mcost_matrix(struct Network *net); /* renvoie la matrice des coûts modifiés */
void free_cost_matrix(double **cost_mat, int n); /* Libère une matrice de coût */

double fast_path_cost(struct List *path, double **cost_mat);
/* Fait la même chose que compute_path_cost mais utilise une matrice
 * précalculée des coûts */
double fast_modified_path_cost(struct List *path,
                               double **mcost_mat);
/* Fait la même chose que compute_modified_path_cost mais utilise une matrice
 * précalculée des coûts modifiés  */

/* ***************** CALCUL DU POTENTIEL ***************** */

double net_potential(struct Network *net); /* Renvoie le potentiel du réseau */
double net_d2potential(struct Network *net);


/* ***************** CALCUL DE CONVERGENCE ***************** */

/* Ces fonctions ont besoin que le graphe soit topologiquement trié */

double DAG_worst_used_Path(int s, int t, double **mass,
                      double **cost_mat, double min_mass, struct graph *g);
/* Renvoie le coût du pire chemin de s à t, de masse non nulle */
/* On met une masse d'arc minimale par sécurité */

double DAG_shortest_Path(int s, int t, double **mass,
                         double **cost_mat, struct graph *g);
/* Renvoie le coût du meilleur chemin de s à t (toutes masses confondues) */

int convergence_on(int s, int t, double **mass, double epsilon,
                   double **cost_mat, struct graph *g);
/* Renvoie 1 si c(p') <= c(p) + e pour tous p,p' chemins s --> t
 * avec p un chemin utilisé (i.e de masse non nulle).
 * Renvoie 0 sinon */

/* ***************** Fonctions d'initialisation ***************** */

void set_allfun (struct Network *net, dtod_t fun); /* met toutes les fonctions à fun */
void set_alldfun(struct Network *net, dtod_t fun); /* met toutes les dérivées à fun */
void set_alld2fun(struct Network *net, dtod_t fun);


/* ***************** AFFICHAGE ***************** */

void aff_masses(struct Network *net);


#endif
