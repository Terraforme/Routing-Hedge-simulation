#ifndef distrib_h
#define distrib_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI (3.141592653589793)

/* On définit ici les méthodes classiques pour manipuler des distributions */
/* Les distributions ne sont pas nécessairement normalisées ;
 * on doit pouvoir faire la somme de distributions, produit avec un scalaire,
 * logit etc. */

/* Une distribution est un tableau de 'double' */
/* Il faut toujours passer la taille de la distribution en argument */

/* Quelques remarques :
 * le logit est normalisé
 * balanced_logit correspond à l'opération classique de l'algo e-Hedge
 * Toutes les opérations sont EN PLACE, pour des questions de performance */

/* *************** Fonctions administratives ***************** */
double* new_distrib(int n); /* Renvoie une nouvelle distribution sur
                             * { 0 ... n-1 } */
void free_distrib(double *x);

/* *************** Fonctions basiques ****************** */

void set_uniform(double *x, int n);   /* x devient la loi uniforme */
void reset_distrib(double *x, int n); /* remet la distribution à 0 */

void normalize(double *x, int n); /* Normalise la distribution */
double min(double *x, int n); /* Renvoie le minimum de la distribution */
double max(double *x, int n); /* Renvoie le maximum de la distribution */

int select_on_distrib(double *x, int n);
/* La distribution x se doit d'être initialisée.
 * Sélectionne un nombre aléatoire sur {0 ... n-1} selon la distribution x
 * i.e P(i) = x[i]. */

void print_distrib(double *x, int n);
/* Affiche la distribution x */

/* **************** Opérations ********************* */

void sum_distrib(double *x, double *y, double scal, int n);
/* x <- x + scal.y normalisé */

void logit(double *target, double *y, int n);
/* target <- logit(y), et y n'est pas modifié. */

void pos_logit(double *target, double *y, int n);
/* target <- pos_logit et y n'est pas modifié */

void balanced_logit(double *target, double *y, double e, int n);
/* target <- e.unif_n + (1 - e).logit(y)
 * y n'est pas modifié.*/

void pos_balanced_logit(double *target, double *y, double e, int n);
/* idem à balanced_logit mais avec avec le logit positif */

/* ********************* Bruit sphérique ********************* */

double Gaussian_rand(void); /* Renvoie un nombre de loi N(0, 1) */
double *spherical_noise(int n);
/* Renvoie un vecteur aléatoire uniforme dans la sphére unité de R^n */

/* ************************* LOI EXPONENTIELLE ************************* */

double rand_exponential(double lambda);
/* Renvoie un nombre aléatoire de loi E(lambda) */

#endif
