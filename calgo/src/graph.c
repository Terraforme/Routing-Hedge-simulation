#include "graph.h"

/* Fonctions de base :
 * nouveau graphe, libération d'un graphe, initialisation aléatoire */

#define handle_error(s) do {fprintf(stderr, #s "\n"); exit(EXIT_FAILURE); } while(0);

struct graph *new_graph(int n)
/* Renvoie un nouveau graphe */
{
  struct graph *g = calloc(1, sizeof (struct graph));
  if (g == NULL) handle_error("new_graph");

  g->n = n;
  g->network = calloc(n, sizeof (int*));
  if (g->network == NULL) handle_error("new_graph");
  for (int i=0; i<n; i++)
  {
    g->network[i] = calloc(n, sizeof (int));
    if (g->network[i] == NULL) handle_error("new_graph");
  }

  return g;
}

void free_graph(struct graph *g)
/* Libère la mémoire allouée à un graphe */
{
  if (g != NULL)
  {
    for (int i=0; i<g->n; i++) free(g->network[i]);
    free(g->network); free(g);
  }
  return ;
}

void set_random (struct graph *g, double p)
/* Erdös-Rényi : cas non orienté */
{
  for (int i=0; i<g->n-1; i++)
  for (int j=i+1; j<g->n; j++)
  {
    if (drand48() < p) g->network[j][i] = g->network[i][j] = 1;
    else               g->network[j][i] = g->network[i][j] = 0;
  }
  return ;
}

void set_drandom(struct graph *g, double p)
/* Erdös-Rényi : cas orienté */
{
  for (int i=0; i<g->n; i++)
  for (int j=0; j<g->n; j++)
  {
    if (i != j && drand48() < p) g->network[i][j] = 1;
    else                         g->network[i][j] = 0;
  }
  return ;
}

static int has_edges(struct graph *g)
/* Renvoie 1 si le graphe g a au moins une arête.
 * Renvoie 0 sinon */
{
  for (int u=0; u<g->n; u++)
  for (int v=0; v<g->n; v++)
  if  (g->network[u][v]) return 1;

  return 0;
}

void set_randDAG(struct graph *g, double p)
/* Random DAG */
/* REMARQUE IMPORTANTE : L'ordre topologique est inhérent à cette génération.
 * On a : il existe un chemin i --> j => i < j, avec cette génération */

/* Ne renvoie pas le graphe vide (le graphe sans arête) */
{
  for (int i=0; i<g->n; i++)
  for (int j=0; j<g->n; j++)
  {
    if (i < j && drand48() < p) g->network[i][j] = 1;
    else                        g->network[i][j] = 0;
  }

  if (!has_edges(g)) set_randDAG(g, p);
  return ;
}

/* ************** FONCTIONS SPECIFIQUES *************** */


static int search_DFS(int u, int v, struct graph *g, int *visited)
/* Parcours en profondeur pour chercher v depuis u dans le graphe g;
 * sachant que les sommets w tels que visited[w] = 1 ont déjà été visités. */
{
  if (u == v) return 1;
  /* On cherche sur les voisins de u */
  for (int w=0; w<g->n; w++) if (g->network[u][w] && !visited[w])
  {
    visited[w] = 1;
    if (search_DFS(w, v, g, visited)) return 1;
  }
  return 0;
}

int connected(int u, int v, struct graph *g)
/* Renvoie 1 s'il existe un chemin u --> v
 * Renvoie 0 sinon. */
{
  int *visited = calloc(g->n, sizeof(int));
  visited[u] = 1;
  int res = search_DFS(u, v, g, visited);
  free(visited);
  return res;
}

struct Couple connected_couple_DAG(struct graph *g)
/* Renvoie un couple (u < v) tel qu'il existe un chemin u --> v
 * Spécifique aux DAG, mais on pourrait étendre la fonction. */
{
  int a, b;
  int u, v;
  do
  {
    a = rand() % g->n;
    b = rand() % g->n;
    u = (a > b) ? b : a;
    v = (a > b) ? a : b;
  } while ( a == b || !connected(u, v, g));

  struct Couple ss;
  ss.left = u; ss.right = v;
  return ss;
}

int **vertices_array(int n)
/* Renvoie un tableau t de taille n tel que t[i] = un pointeur vers un int
 * de valeur i */
{
  int **t = malloc(n*sizeof(int*));
  if (t == NULL) handle_error("(malloc) vertices_array");
  for (int i=0; i<n; i++)
  {
    t[i] = malloc(sizeof(int));
    if (t[i] == NULL) handle_error("(malloc) vertices_array");
    *t[i] = i;
  }
  return t;
}

void free_vertices(int **vertices, int n)
/* Libère un tableau type vertices_array */
{
  for (int i=0; i<n; i++) free(vertices[i]);
  return free(vertices);
}

static struct List *path_from_to_on(int u, int v, struct graph *g, int *available,
                                    int **vertices)
/* Renvoie la liste des chemins u --> v dans g en n'utilisant que des sommets
 * tels que available[.] = 1 */
{
  //printf("@ Here at %d\n", u);
  struct List *local_paths = new_empty();

  if (u == v) /* Cas où on a atteint v : on renvoie [[v]] */
  {
    struct List *singleton = new_empty();
    singleton = push(vertices[u], singleton);
    local_paths = push(singleton, local_paths);
    /*printf("(%d)-> got :", u);
    aff_List(local_paths, aff_List_int);
    printf("\n");*/
    return local_paths;
  }

  /* Cas u != v : formule de récurrence */
  int sup = (DAG == 1) ? (v+1) : g->n;
  for (int w=0; w<sup; w++)
  if (available[w] && g->network[u][w])
  {
    //printf("### from %d search %d\n", u, w);
    available[w] = 0;
    local_paths = concat(local_paths, path_from_to_on(w, v, g, available, vertices));
    available[w] = 1;
  }


  local_paths = add_on(local_paths, vertices[u]);
  /*printf("(%d)-> got :", u);
  aff_List(local_paths, aff_List_int);
  printf("\n");*/

  return local_paths;
}

struct List *path_from_to(int u, int v, struct graph *g, int **vertices)
/* Renvoie la liste des chemins u --> v dans g
 * Astuce : on a un tableau t tel que t[i] est un pointeur vers un
 * entier de valeur i */
{
  int *available = malloc(g->n*sizeof(int));
  for (int i=0; i<g->n; i++) available[i] = 1;
  available[u] = 0;

  struct List *all_paths = path_from_to_on(u, v, g, available, vertices);
  free(available);
  return all_paths;
}


/* Fonctions utilitaires : Affichages & co */

void aff_graph(struct graph *g, char coma)
/* affiche le graphe en matrice d'adjacence
 * dans le terminal */
{
  for (int i=0; i<g->n; i++)
  {
    for (int j=0; j<g->n; j++)
    {
      if (g->network[i][j]) putchar('1');
      else                  putchar('0');
      if (coma && j != g->n-1) putchar(',');
      else                     putchar(' ');
    }
    putchar('\n');
  }
  return ;
}

void list_links(struct graph *g)
{
  for (int i=0; i<g->n; i++)
  for (int j=0; j<g->n; j++)
  if (g->network[i][j]) printf("%d -> %d\n", i, j);
  return ;
}
