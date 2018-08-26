#ifndef ui_h
#define ui_h

#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "graph.h"
#include "network_th.h"
#include "distrib.h"

#define NO_NOISE 0
#define WITH_NOISE 1

/* ********** On définit ici le nécessaire pour les joueurs *************** */

/* ********** JOUEURS SEMI-BANDITS ********** */
struct SBPlayer
{
  int source, sink;    /* Couple origine/destination */
  double mass;         /* Demande du joueur */
  struct List *paths;  /* Liste des chemins possibles source --> sink */
  double *Y_uv; /* Liste des évaluations des chemins (Y_i) */
  int n;               /* Nombre d'actions */
};

/* ************* FONCTIONS ADMINISTRATIVES ************* */

struct SBPlayer *new_SBPlayers(int n);
/* Renvoie un pointeur vers un tableau de n nouveaux joueurs (non initalisés) */

void init_SBPlayer(int i, struct SBPlayer *players, struct graph *g,
                   int **vertices);
/* Initialise le i-ième joueur (couple ss, masse, chemins, évaluations) */
void init_SBPlayers(struct SBPlayer *players, struct graph *g, int n,
                    int **vertices);
/* Initialise les n premiers joueurs */
void set_SBPlayer(int i, struct SBPlayer *players, int source, int sink,
                  double mass, struct graph *g, int **vertices);
/* Initialise le joueur 'i' à (source, sink, mass) */

void normalize_SBPlayers(struct SBPlayer *players, int n);
/* Normalise les masses des n premiers joueurs de SBP */

void free_SBPlayers(struct SBPlayer *players, int n);
/* Libère les n premiers joueurs de players, et libère le pointeur 'players' */

void reset_SBPlayers(struct SBPlayer *players, int n);
/* Remet les évaluation des n premiers joueurs à 0 */

/* ******************** FONCTIONS DE JEU ******************** */

double* SBPlayer_distrib(int i, struct SBPlayer *players,
                         double e);
/* Calcule la distribution du joueur i qu'il renvoie sous la forme d'un tableau.
 * Méthode du semi-bandit : celui-là a relevé des coûts modifiés.
 * On donne aussi l'epsilon */

double* fast_eval_player(int i, struct SBPlayer *players, double **cost_mat);
/* Renvoie la matrice des coûts du joueur i, en fonction de la matrice des
 * coûts en paramètre. Si celle-si est la matrice des coûts purs, alors
 * on a le cas bandit ; si c'est la matrice des coûts modifiés, alors on a
 * le cas semi-bandit. */

double **paths_mass_spread(int p, struct SBPlayer *players, int n);
/* Renvoie la matrice des masses du joueur 'p'.
 * 'n' est la taille du graphe. */


/* ******************* FONCTIONS USER INTERFACE ******************* */

void aff_SBPlayers(struct SBPlayer *players, int n, int verbative);
/* Affiche les n premiers joueurs de 'players' */

void aff_SBPlayer_score(int i, struct SBPlayer *players);
/* Affiche la liste des correspondances chemin/masse accordée */



/* *********************** CAS D'UN JOUEUR PAR NOEUD *********************** */
/* Plaçons-nous maintenant dans le cas d'un joueur par nœud */

struct VertexPlayer /* One - One Player */
{
  int d; /* degré */
  int id; /* Identifiant (numéro du sommet) */
  int    *neighbours;  /* Tableau des voisins */
  double *Y_uv; /* Tableau des évaluations sur les possibilités */
  double *W_uv; /* Tableau des w sur les arêtes */
  double W_u;    /* Son propre score (w_v) */
};

struct VPPopulation /* Vertex - Players Population */
/* L'équivalent d'un joueur dans le cas précédent */
{
  struct VertexPlayer *players; /* En adéquation avec le graphe */
  double mass;
  int source, sink;
  int n; /* Nombre de joueurs */
};


/* ******************** ADMINISTRATION ******************** */

void init_VertexPlayer(struct VertexPlayer *players, struct graph *g, int id);
/* Initialise le player d'id ID */
void free_VertexPlayer(struct VertexPlayer *players, int id);
/* Libère la mémoire dédiée au joueur d'id 'id' */

struct VPPopulation *new_population_set(struct graph *g, int k);
/* Renvoie un tableau de k nouvelles populations en adéquation avec le graphe G */
void free_VPPopulation_set(struct VPPopulation *pop, int k);
/* Libère la mémoire dédiée à k populations */

void reset_VPPopulation_set(struct VPPopulation *pop, int k);
/* Remet à 0 les évaluation d'un ensemble de populations */

/* ******************** FONCTIONS USUELLES ******************** */

void normalize_VPPopulation_set(struct VPPopulation *pop, int k);
/* Normalise, ensembles, les k premières populations */

void set_VPPopulation_mass(double mass, struct VPPopulation *pop, int i);
/* Met la masse d'une population donnée à la quantité demandée */
/* Ne renormalise pas */

void convexity_fix_VPPopulation(struct VPPopulation *pop, struct graph *g, int n);
/* Sous optimal - met des coûts infinis pour éviter la perte de paquets */

/* ******************** FONCTIONS D'ÉVALUATION ******************** */

double *VertexPlayer_distrib(int i, struct VertexPlayer *players, double e);
/* Calcule la distribution du joueur i */

void update_eval_VertexPlayer(int i, struct VertexPlayer *players, double *eval,
                              int sink);
/* Met à jour les évaluations du joueur i étant données les coûts qu'il vient de
 * mesurer - on assume que les joueurs topologiquement supérieurs à i on
 * déjà actualisé leur score. -- eval déjà pondéré par gamma */

double **mass_spread(int i, struct VPPopulation *pop, double e);
/* Renvoie la matrice de la répartition de masse faite par la population i */



/* #################### CAS BANDIT #################### */
/* Dans ce cas, les joueurs  n'ont pas accès au dérivées */

struct VertexBandit /* One - One Player */
{
  int d; /* degré */
  int    *neighbours;  /* Tableau des voisins */
  double *Y_uv;        /* Tableau des Y_uv */
  double *W_uv;        /* Tableau des W_uv */
  double W_u;          /* Son propre score W_u */

  /* Deux utilitaires qui servent de tableaux de calcul
   * pour l'estimation de ~c */
  double *noise; /* les z_uv */
  double *costs;
  double *noisy_costs; /* c'est \sum z_uv c(x_uv + delta.z_uv) */
};

struct VBPopulation /* Vertex - Bandits Population */
/* L'équivalent d'un joueur dans le cas précédent */
{
  struct VertexBandit *bandits; /* En adéquation avec le graphe */
  double mass;
  int source, sink;
  int n; /* Nombre de joueurs */
};

/* #################### ADMINISTRATION #################### */

void init_VertexBandit(struct VertexBandit *players, struct graph *g, int u);
/* Initialise le bandit sur le sommet u */

void free_VertexBandit(struct VertexBandit *players, int u);
/* Libère la mémoire dédiée au bandit sur le sommet u */

struct VBPopulation *new_VBPopulation_set(struct graph *g, int k);
/* Renvoie un tableau de k nouvelles populations en adéquation avec le graphe G */

struct VBPopulation *VBPopulation_from_VPPopulation(struct VPPopulation *pop, int k);
/* Renvoie un tableau de k nouvelles populations en adéquation
 * avec la population semi-bandit donnée en argument */

void free_VBPopulation_set(struct VBPopulation *pop, int k);
/* Libère la mémoire dédiée à k populations */

void partial_free_VBPopulation_set(struct VBPopulation *pop, int k);
/* Ne libère que les champs noise et noisy et costs */

void reset_VBPopulation_set(struct VBPopulation *pop, int k);
/* Remet à 0 les évaluations d'un ensemble de populations */

/* #################### FONCTIONS USUELLES #################### */

void normalize_VBPopulation_set(struct VBPopulation *pop, int k);
/* Normalise, ensembles, les k premières populations */

void set_VBPopulation_mass(double mass, struct VBPopulation *pop, int i);
/* Met la masse d'une population donnée à la quantité demandée */
/* Ne renormalise pas */

void reset_VBPopulation_noisy_costs(struct VBPopulation *pop, int k);
/* Remet à 0 les coûts bruités de tous les individus */

/* #################### FONCTIONS D'ÉVALUATION #################### */

double *VertexBandit_distrib(int u, struct VertexBandit *bandits, double e,
                             int noise);
/* Calcule la distribution du bandit u */
/* Spécifier NO_NOISE pour ne pas rajouter de bruit, WITH_NOISE sinon. */

double **bandit_mass_spread(int p, struct VBPopulation *pop, double e, int noise);
/* Renvoie la matrice de la répartition de masse faite par la population i */
/* Spécifier NO_NOISE pour ne pas rajouter de bruit, WITH_NOISE sinon. */

void bandit_measure_costs(struct VBPopulation *pop, int k,
                          struct Network *net, double epsilon);
/* Mesure les coûts purs pour une population dans un réseau */

void bandit_add_noisy_measure(struct VBPopulation *pop, int k,
                              struct Network *net, double epsilon);
/* Rajoute une mesure bruitée (pour l'approximation du gradient */

void bandit_update_scores(struct VBPopulation *pop, int pop_size, struct Network *net,
                          double gamma, double epsilon, int k);
/* Actualise les Y_uv, W_uv, W_u de tous les joueurs
 * gamma : gamma_n, epsilon : epsilon_n, k : nombre d'itération de l'estimation
 * du gradient. */

/* ************************ DEBUG : AFFICHAGE ************************ */

void print_VertexBandit(struct VertexBandit *bandits, int u, double e);
/* Affiche le u-ième bandit */
void print_VBPopulation(struct VBPopulation *pop, int k, double e);

#endif
