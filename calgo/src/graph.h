#ifndef graph_h
#define graph_h

#include <stdio.h>
#include <stdlib.h>
#include "list.h"

struct Couple
/* Petite structure de couple bien pratique */
{
  int left, right;
};

struct graph
{
  int **network;
  int n; /* nombre de sommets */
  int m; /* nombre d'arêtes   */
};

#define DAG 1 /* "Booléen" à désactiver quand on n'est plus sur des DAG */

/* Fonctions de base :
 * nouveau graphe, libération d'un graphe, initialisation aléatoire */

struct graph *new_graph(int n);   /* Renvoie un nouveau graphe */
void free_graph(struct graph *g); /* Libère la mémoire allouée à un graphe */

void set_random (struct graph *g, double p); /* Erdös-Rényi : cas non orienté */
void set_drandom(struct graph *g, double p); /* Erdös-Rényi : cas orienté */
void set_randDAG(struct graph *g, double p); /* Random DAG */

/* ************** FONCTIONS SPECIFIQUES *************** */

int connected(int u, int v, struct graph *g);
/* Renvoie 1 s'il existe un chemin u --> v
 * Renvoie 0 sinon. */

struct Couple connected_couple_DAG(struct graph *g);
/* Renvoie un couple (u < v) tel qu'il existe un chemin u --> v
 * Spécifique aux DAG, mais on pourrait étendre la fonction. */

int **vertices_array(int n);
/* Renvoie un tableau t de taille n tel que t[i] = un pointeur vers un int
 * de valeur i */

void free_vertices(int **vertices, int n);
/* Libère un tableau type vertices_array */

struct List *path_from_to(int u, int v, struct graph *g, int **vertices);
/* Renvoie la liste des chemins u --> v dans g
 * Astuce : on a un tableau t tel que t[i] est un pointeur vers un
 * entier de valeur i */


/* Utilitaires : affichages & co */

void aff_graph(struct graph *g, char coma);
                                 /* affiche le graphe en matrice d'adjacence
                                  * dans le terminal */
void list_links(struct graph *g); /* liste les liens de g (ie les arcs */

#endif
