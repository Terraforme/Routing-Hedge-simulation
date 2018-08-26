#ifndef network_simu_h
#define network_simu_h

#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "distrib.h"
#include "network_th.h"
#include "event.h"

struct SimulatedPlayer
{
  int d; /* Son degré */
  double mu; /* Sa 'vitesse de travail' */
  int *neighbours;

  double *W_u;       /* W_u[t] = W_u^{(t)}      */
  double **W_uv;     /* W_uv[t][v] = W_uv^{(t)} */
  double  *Y_uv;     /* Indépendant de t        */
  double **X_uv;     /* Distribution sur uv en fonction de t,  */
                     /* X_u[t][v] = X_uv^{(t)} */

  double own_c_uv;
  double own_c_uv_count;

  int n; /* Compteur de paquets */
  int nIter; /* Compteur d'updates */
};

struct SimulatedNetwork
{
  double **lambda;                /* lambda[s][t] : intensity of
                                   * emission from s towards t */
  double E; /* Constante d'échantillonage */
  struct EventQueue *qevents;
  struct SimulatedPlayer *vertex;
  double *L; /* Tableau des chargements (load) */
  double *T; /* Tableau des derniers temps d'arrivées */
  int n; /* Nombre de joueurs */
};

/* *********************** ADMINISTRATION *********************** */

struct SimulatedPlayer *new_SimulatedPlayers(struct graph *g);
void free_SimulatedPlayers(struct SimulatedPlayer *vertex, int n);
/* n : taille du graphe */

struct SimulatedNetwork *new_SimulatedNetwork(struct graph *g, double E);
void free_SimulatedNetwork(struct SimulatedNetwork *snet);

/* *********************** UTILITAIRE *********************** */

double positive_part(double x); /* Renvoie 0 si x < 0, x sinon */

void reset_c_uv(struct SimulatedPlayer *vertex, int u);
/* Remet les c_uv et count_uv à 0 pour le sommet u */

void update_distrib_SimulatedPlayer(struct SimulatedPlayer *vertex, int u, int n,
                                    int nPlayers);
/* Remet à jour la distribution du joueur k, selon une itération n */

/* *********************** SIMULATION *********************** */

void event_NEW_PAQUET(struct SimulatedNetwork *snet  , struct Event event);
void event_TREAT_PAQUET(struct SimulatedNetwork *snet, struct Event event);
void event_UPDATE_DISTRIB(struct SimulatedNetwork *snet, struct Event event);

void treat_new_event(struct SimulatedNetwork *snet);
/* Fonction de traitement d'un événement.
 * À utiliser en while(...) treat_new_event(snet); */

/* *********************** UTILITAIRE *********************** */

void spread_Simulated_mass(struct SimulatedNetwork *snet, struct Network *net);
/* Met la masse dans le network 'net' selon les distributions
 * des joueurs simulés. */

#endif
