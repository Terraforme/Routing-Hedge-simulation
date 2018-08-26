#include "list.h"

#define handle_error(s) do {fprintf(stderr, #s "\n"); exit(EXIT_FAILURE); } while(0);


/* **************** Fonctions administratives **************** */

struct List *new_empty(void)
/* Renvoie une liste vide */
{
  struct List *l = malloc(sizeof(struct List));
  if (l == NULL) handle_error("(List) new_empty");

  l->tail = l->head = NULL;
  return l;
}

void free_List(struct List *l, void (*free_el)(void* el))
/* Libère la mémoire dédiée à une liste ainsi que de ses éléments
 * Utilise la fonction de libération passée en argument */
{
  struct List *next = NULL;
  while (!is_empty(l))
  {
    next = l->tail;
    free_el(l->head);
    free(l);
    l = next;
  }
  free(l);
  return ;
}

void partial_free_List(struct List *l)
/* Libère la mémoire dédiée à une liste, et libère ses éléments en utilisant
 * la fonction 'free' */
{
  return free_List(l, free);
}

void pass(void* x)
/* ne fait rien */
{
  x = x;
  return ;
}

void free_paths(struct List *l)
/* Libère une liste de chemins (i.e type (int list) list) */
{
  struct List *next = NULL;
  while (!is_empty(l))
  {
    next = l->tail;
    free_List(l->head, pass);
    free(l);
    l = next;
  }
  free(l); /* on libère la liste vide */
  return;
}


/* *********************** Fonctions élémentaires ************************* */

int is_empty(struct List *l)
/* Renvoie 1 si l est vide, 0 sinon */
{
  return l->head == NULL;
}

int len(struct List *l)
/* Renvoie la taille de la liste.
 * par convention : len([]) = 0 */
{
  int res = 0;
  while (!is_empty(l))
  {
    res ++;
    l = l->tail;
  }
  return res;
}

struct List *push(void* el, struct List *l)
/* Renvoie une nouvelle liste : el :: l */
{
  struct List *el_l = new_empty();

  el_l->tail = l;
  el_l->head = el;
  if (is_empty(l)) el_l->jmper = el_l;
  else             el_l->jmper = l->jmper;

  return el_l;
}

struct List *concat(struct List *l1, struct List *l2)
/* Modifie le chaînage de l1 en l1 @ l2
 * Remarque : cette liste est exactement constituée des mêmes maillons que
 * l1 et l2. Donc si on fait free(l1 @ l2), on libère l1 et l2 en même temps.
 * Cette fonction libère de la mémoire de sorte à ce qu'il n'y ait que l1 @ l2
 * à libérer après une concaténation. */
{
  if (is_empty(l1))
  {
    free(l1);
    return l2;
  }
  else /* donc l1 != [] */
  {
    free(l1->jmper->tail); /* l1->jmper->tail est nécessairement [] */
    l1->jmper->tail = l2;
    if (!is_empty(l2)) l1->jmper = l2->jmper; /* on ne veut pas faire pointer
                                               * le jumper vers [] */
  }
  return l1;
}

struct List *add_on(struct List *l, void *el)
/* l doit être une liste de listes.
 * si l = l1 :: l2 :: l3 :: ..., renvoie (el :: l1) :: (el :: l2) :: .... */
{
  struct List *l0 = l;
  while (!is_empty(l))
  {
    l->head = push(el, l->head);
    l = l->tail;
  }
  return l0;
}

/* ******************************* AFFICHAGE ****************************** */

void aff_List(struct List *l, void (*aff_el)(void* el))
/* Affiche le contenu d'une liste. Prend en argument la fonction d'affichage */
{
  if (l == NULL || is_empty(l)) printf("[]");
  else
  {
    int i = 0; /* ce 'i' ne sert qu'à faire un bel affichage */

    printf("[");
    while (!is_empty(l))
    {
      if (i) printf(", "); /* Concaténation d'éléments */
      else i ++;

      aff_el(l->head);
      l = l->tail;
    }
    printf("]");
  }
  return ;
}

static void aff_int(void *ptr)
/* Affiche un pointeur en temps qu'entier 'int' */
{
  int n = *(int*) ptr;
  printf("%d", n);
  return;
}

void aff_List_int(void *l)
/* Affiche la liste en temps que liste d'entiers
 * équivalent à aff_List(l, aff_int)*/
{
  return aff_List(l, aff_int);
}

/* REMARQUE : On appelle 'paths' le type (int list) list */

void aff_paths(struct List *paths)
/* Affiche les chemins paths */
{
  aff_List(paths, aff_List_int);
}
