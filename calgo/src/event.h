#ifndef event_h
#define event_h

#include <stdio.h>
#include <stdlib.h>

#define NEW_PAQUET   0
#define TREAT_PAQUET 1
#define UPDATE_DISTRIB 2

struct EventID
{
  double T; /* Instant de création de l'évent. */
  int source;
  int sink;
};

struct Event /* La structure pour les événements */
{
  double T;   /* Instant de l'event */
  int u;      /* Nœud concerné */
  int sink;   /* Destination */
  int action; /* ID de l'action */
  struct EventID ID; /* Identifiant de l'évent (ses origines) */
};

struct EventQueue
/* Une file de priorité en pratique
 * File de priorité contenant les événements à traiter */
{
  struct Event *events;
  int n; /* Pointe sur la prochaine case vide dans events */
  int maxsize;
};

struct EventQueue *new_EventQueue(int maxsize);
void free_EventQueue(struct EventQueue *qevents);

struct EventQueue *extend_EventQueue(struct EventQueue *qevents);
/* Double le nombre d'événements possibles dans la file */

struct EventID new_EventID(double T, int source, int sink);
/* Renvoie EventID(T, sink, source) */
struct Event new_Event(double T, int u, int action, int sink, struct EventID ID);
/* Renvoie Event(T, u, action, sink) */

/* ***************** MANIPULATION DES ÉVÉNEMENTS ***************** */

void add_Event(struct Event event, struct EventQueue **qevents);
/* Ajoute un événement à l'ensemble des événements.
 * Passer un pointeur vers cet ensemble, en cas d'extension. */

int next_event(struct Event *event, struct EventQueue *qevents);
/* Extrait l'événement suivant dans la file de priorité.
 * Renvoie 1 si l'extraction s'est passée sans soucis.
 * Renvoie 0 sinon. */

/* ***************** FONCTIONS D'AFFICHAGE ***************** */

void print_Event(struct Event event);

#endif
