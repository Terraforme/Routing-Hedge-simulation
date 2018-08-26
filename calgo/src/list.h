#ifndef list_h
#define list_h

#include <stdio.h>
#include <stdlib.h>

/* ************************* LISTES ************************ */
/* Je définis ici une structure de listes non typées
 * Les listes peuvent être non homogènes. Je conseille tout
 * de même (très vivement) de travailler avec de listes
 * homogènes */


struct List
{
  void *head;
  struct List *tail;
  struct List *jmper; /* Queue de la liste : dernier élément NON VIDE */
                      /* Sert à faire des concaténations en O(1) */
                      /* RQ : ce pointeur n'est correct que pour la VRAIE
                       * TETE de liste (pour des raisons de complexité */
};

/* ******************* Fonctions administratives ******************* */

struct List *new_empty(void); /* Renvoie une liste vide */
void free_List(struct List *l, void (*free_el)(void* el));
/* Libère la mémoire dédiée à une liste ainsi que de ses éléments
 * Utilise la fonction de libération passée en argument */
void partial_free_List(struct List *l);
/* Libère la mémoire dédiée à une liste, et libère ses éléments en utilisant
 * la fonction 'free' */

void free_paths(struct List *l);
/* Libère une liste de chemins (i.e type (int list) list) */

/* ******************* Fonctions élémentaires ******************* */

int is_empty(struct List *l); /* Renvoie 1 si l est vide, 0 sinon */
int len(struct List *l); /* Renvoie la taille de la liste.
                          * par convention : len([]) = 0 */

struct List *push(void* el, struct List *l);
/* Renvoie une nouvelle liste : el :: l */

struct List *concat(struct List *l1, struct List *l2);
/* Modifie le chaînage de l1 en l1 @ l2, qui est renvoyé
 * Remarque : cette liste est exactement constituée des mêmes maillons que
 * l1 et l2. Donc si on fait free(l1 @ l2), on libère l1 et l2 en même temps.
 * Cette fonction libère de la mémoire de sorte à ce qu'il n'y ait que l1 @ l2
 * à libérer après une concaténation. */

struct List *add_on(struct List *l, void *el);
/* l doit être une liste de listes.
 * si l = l1 :: l2 :: l3 :: ..., renvoie (el :: l1) :: (el :: l2) :: .... */

/* Fonctions d'affichage */

void aff_List(struct List *l, void (*aff_el)(void* el));
/* Affiche le contenu d'une liste. Prend en argument la fonction d'affichage */

void aff_List_int(void *l);
/* Affiche la liste en temps que liste d'entiers
 * équivalent à aff_List(l, aff_int)*/

/* REMARQUE : On appelle 'paths' le type (int list) list */

void aff_paths(struct List *paths);
/* Affiche les chemins paths */

#endif
