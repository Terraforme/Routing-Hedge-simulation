#include "event.h"

struct EventQueue *new_EventQueue(int maxsize)
{
  struct EventQueue *qevents = malloc(sizeof(struct EventQueue));
  if (qevents == NULL) { fprintf(stderr, "(malloc) new_EventQueue\n");
                         exit(EXIT_FAILURE); }

  qevents->events = malloc(maxsize * sizeof(struct Event));
  if (qevents->events == NULL) { fprintf(stderr, "(malloc) new_EventQueue\n");
                                exit(EXIT_FAILURE); }
  qevents->n = 0;
  qevents->maxsize = maxsize;
  return qevents;
}


void free_EventQueue(struct EventQueue *qevents)
{
  free(qevents->events);
  return free(qevents);
}

struct EventQueue *extend_EventQueue(struct EventQueue *qevents)
/* Double le nombre d'événements possibles dans la file */
{
  struct EventQueue *extended_qevents = new_EventQueue(qevents->maxsize * 2);
  for (int i=0; i<qevents->maxsize; i++)
    extended_qevents->events[i] = qevents->events[i];
  extended_qevents->n = qevents->n;
  free_EventQueue(qevents);
  return extended_qevents;
}


struct EventID new_EventID(double T, int source, int sink)
/* Renvoie EventID(T, sink, source) */
{
  struct EventID ID;
  ID.T = T;
  ID.source = source;
  ID.sink   = sink;
  return ID;
}

struct Event new_Event(double T, int u, int action, int sink, struct EventID ID)
/* Renvoie Event(T, u, action, sink) */
{
  struct Event event;
  event.T  = T;
  event.u  = u;
  event.ID = ID;
  event.sink   = sink;
  event.action = action;

  return event;
}

/* ***************** MANIPULATION DES ÉVÉNEMENTS ***************** */

void add_Event(struct Event event, struct EventQueue **p_qevents)
/* Ajoute un événement à l'ensemble des événements.
 * Passer un pointeur vers cet ensemble, en cas d'extension. */
{
  //printf("Adding : "); print_Event(event);
  if ((*p_qevents)->n >= (*p_qevents)->maxsize)
      *p_qevents = extend_EventQueue(*p_qevents);
  struct EventQueue *qevents = *p_qevents;

  int n = qevents->n;
  qevents->events[n] = event;
  qevents->n ++;
  while (n > 0)
  {
    int m = (n-1) / 2;
    if (qevents->events[m].T <= event.T) break;

    qevents->events[n] = qevents->events[m];
    qevents->events[m] = event;
    n = m;
  }

  return;
}

int next_event(struct Event *event, struct EventQueue *qevents)
/* Extrait l'événement suivant dans la file de priorité.
 * Renvoie 1 si l'extraction s'est passée sans soucis.
 * Renvoie 0 sinon. */
{
  if (!qevents->n) return 0;
  //printf("== At top : "); print_Event(qevents->events[0]);
  *event = qevents->events[0];

  /*printf("Pre-state : \n");
  for (int i=0; i<qevents->n; i++)
  { printf(" · "); print_Event(qevents->events[i]); }*/

  qevents->events[0] = qevents->events[qevents->n-1];
  qevents->n --;
  int k = 0, n = qevents->n;
  while (1) /* On fait descendre l'événement */
  {
    if (2*k+1 >= n) break;
    else if (2*k+2 > n && qevents->events[k].T > qevents->events[2*k+1].T)
    {
      qevents->events[k] = qevents->events[2*k+1];
      qevents->events[2*k+1] = qevents->events[n];
      break;
    }

    else if (qevents->events[k].T > qevents->events[2*k+1].T
        && qevents->events[2*k+2].T >= qevents->events[2*k+1].T)
    {
      qevents->events[k] = qevents->events[2*k+1];
      qevents->events[2*k+1] = qevents->events[n];
      k = 2*k+1;
    }

    else if (qevents->events[k].T > qevents->events[2*k+2].T
        && qevents->events[2*k+1].T >= qevents->events[2*k+2].T)
    {
      qevents->events[k] = qevents->events[2*k+2];
      qevents->events[2*k+2] = qevents->events[n];
      k = 2*k+2;
    }

    else break;
  }

  /*printf("Post-state : \n");
  for (int i=0; i<qevents->n; i++)
  { printf(" · "); print_Event(qevents->events[i]); }*/

  return 1;
}

/* ***************** FONCTIONS D'AFFICHAGE ***************** */

void print_Event(struct Event event)
{
  printf("(T: %.3f, u: %d, a: %d, t:%d)\n", event.T,      event.u,
                                            event.action, event.sink);
  return;
}
