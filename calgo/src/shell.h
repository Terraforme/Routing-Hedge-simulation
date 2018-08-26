#ifndef shell_h
#define shell_h

/* On définit ici une bibliothèque pour faire un interface de type shell */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "graph.h"
#include "network_th.h"
#include "network_simu.h"
#include "list.h"
#include "ui.h"
#include "fun.h"


/* Les booléens */
#define TRUE  1
#define FALSE 0

/* Valeurs de retours des fonctions (codes d'erreurs) */
#define NORMAL 0
#define EMPTY  1
#define UNKNOWN 2
#define NOTOKEN 3
#define NOOBJECT 4
#define MISSING  5

/* Modes d'exécution */
#define MODE_PATHS  1
#define MODE_VERTEX 2
#define MODE_BANDIT 4
#define MODE_SIMU   8

#define SILENT    16
#define POTENTIAL 32
#define TIME      64
#define STOP      128
#define STOP_CCC  512

#define GAMMA_CORRECTION 256

struct ShellPlayer
/* On a besoin d'une structure spéciale de joueurs pour le
 * shell. Celle-ci a besoin d'être simple et juste descriptive. */
{
  int source, sink;
  double mass;
};

struct Shell
{
  char token[1024];
  int exists_token;

  struct graph       *g;
  struct Network     *net;
  struct ShellPlayer *players;

  /* Ensemble de paramètres */
  int initialized_network, initialized_players;
  int nPlayers, nGraph;
  int exec_mode;

  int nIter;
  double precision;
};

struct Shell *new_Shell(void); /* Renvoie un nouveal Shell */
void free_Shell(struct Shell *sh); /* Libère la mémoire dédiée à un Shell */

int next_token(struct Shell *sh);
/* Lis le token suivant/
 * Renvoie 1 si c'est le dernier token de la ligne (fin : '\n' ou '\0')
 * Renvoie 0 si ce n'est pas le derner token */
void read_cmd(struct Shell *sh); /* Lis une ligne de commande */

/* ************************ MANIPULATION DES TOKENS ************************ */

int empty_token(const char *token);
/* Renvoie 1 si sh->token est le token vide. Renvoie 0 sinon */
int cmp_token(const char *token1, const char *token2);
/* Compare sh->token avec la chaîne de caractères en argument
 * Renvoie 1 si elles sont égales, 0 sinon */
void flush_tokens(struct Shell *sh);
/* Vide stdin */

/* ************************ MANIPULATION DES COMMANDES ************************ */

int treat_cmd(struct Shell *sh);
/* *** FONCTION PRINCIPALE *** */

int quit(struct Shell *sh);
int unknown(struct Shell *sh);

/* Création */
int new_smg(struct Shell *sh);
int shell_new_graph(struct Shell *sh);   /* Nouveau graphe [taille] [probas] */
int shell_new_network(struct Shell *sh); /* Nouveau réseau [taille] */
int shell_new_players(struct Shell *sh); /* Nouveaux joueurs [nombre] */

/* Settings */
int set(struct Shell *sh); /* Fonction maîtresse */
int set_network(struct Shell *sh);
int set_player(struct Shell *sh);
int set_graph(struct Shell *sh);
int set_mass(struct Shell *sh);
int set_beta(struct Shell *sh);
int set_cst_gamma(struct Shell *sh);


/* Conversion utiles pour les simulations */
struct SBPlayer *ShellPlayers_to_SBPlayers(struct Shell *sh, int n,
                                           int **vertices);
/* Étant donnés 'n' joueurs type shell, renvoie une structure équivalente
 * de 'n' joueurs type sb et les initialise pour se préparer à une simulation.
 * A besoin de structures auxiliaires. */

struct VPPopulation *ShellPlayers_to_VPPopulation(struct Shell *sh, int n);
/* Étant donnés 'n' joueurs type shell, renvoie une structure équivalente
 * de 'n' popuations type vertex et les initialise pour se préparer à une
 * simulation.
 * A besoin de structures auxiliaires. */

struct VBPopulation *ShellPlayers_to_VBPopulation(struct Shell *sh, int n);
/* Étant donnés 'n' joueurs type shell, renvoie une structure équivalente
 * de 'n' popuations type BANDIT et les initialise pour se préparer à une
 * simulation.
 * A besoin de structures auxiliaires. */

/* Simulation */
int run(struct Shell *sh);

/* Affichage */
int print(struct Shell *sh);                /* Fonction d'affichage maîtresse */
int shell_print_graph(struct Shell *sh);    /* Fonction d'affichage du graphe */
int shell_print_network(struct Shell *sh);  /* Fonction d'affichage du réseau */
int shell_print_players(struct Shell *sh);  /* Affichage de la liste des joueurs */
int shell_print_player(struct Shell *sh);   /* Affichage d'un joueur */
int shell_print_scores(struct Shell *sh);   /* Affiche les scores des joueurs */
int shell_print_masses(struct Shell *sh);   /* Affiche les masses dans le graphe */
int shell_print_potential(struct Shell *sh);
int shell_graphviz(struct Shell *sh);

/* **** MODES **** */

int change_mode(struct Shell *sh); /* change le mode du shell (un joueur par sommet par exemple) */
int convert_to_vertex(struct Shell *sh); /* Convertit les joueurs paths en vertex */
int convert_to_paths (struct Shell *sh); /* Convertit les joueurs vertex en paths */


#endif
