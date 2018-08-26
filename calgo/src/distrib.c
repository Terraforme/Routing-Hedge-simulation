  #include "distrib.h"

#define handle_error(s) do {fprintf(stderr, #s "\n"); exit(EXIT_FAILURE); } while(0);

/* *************** Fonctions administratives ***************** */
double* new_distrib(int n)
/* Renvoie une nouvelle distribution sur
 * { 0 ... n-1 } */
{
  double *x = calloc(n, sizeof(double));
  if (x == NULL) handle_error("(malloc) new_distrib");

  return x;
}

void free_distrib(double *x)
{
  return free(x);
}

/* *************** Fonctions basiques ****************** */

void set_uniform(double *x, int n)
/* x devient la loi uniforme */
{
  for (int i=0; i<n; i++) x[i] = 1. / n;
  return;
}

void reset_distrib(double *x, int n)
/* remet la distribution à 0 */
{
  for (int i=0; i<n; i++) x[i] = 0;
  return ;
}

void normalize(double *x, int n)
/* Normalise la distribution */
{
  double norm = 0;
  for (int i=0; i<n; i++) norm += x[i];
  for (int i=0; i<n; i++) x[i] /= norm;
  return ;
}

double min(double *x, int n)
/* Renvoie le minimum de la distribution */
{
  if (n == 0) return 0;

  double m = x[0];
  for (int i=1; i<n; i++) if(m > x[i]) m = x[i];

  return m;
}

double max(double *x, int n)
/* Renvoie le maximum de la distribution */
{
  if (n == 0) return 0;

  double m = x[0];
  for (int i=1; i<n; i++) if (m < x[i]) m = x[i];

  return m;
}

int select_on_distrib(double *x, int n)
/* La distribution x se doit d'être initialisée.
 * Sélectionne un nombre aléatoire sur {0 ... n-1} selon la distribution x
 * i.e P(i) = x[i]. */
{
  double lambda = drand48();
  double sum = 0;

  for (int i=0; i<n-1; i++)
    if (lambda < sum + x[i]) return i;
    else sum += x[i];

  return n-1;
}

void print_distrib(double *x, int n)
/* Affiche la distribution x */
{
  if (!n) printf("[]\n");
  else
  {
    printf("[");
    for (int i=0; i<n-1; i++) printf("%lf, ", x[i]);
    printf("%lf]\n", x[n-1]);
  }
  return ;
}

/* **************** Opérations ********************* */

void sum_distrib(double *x, double *y, double scal, int n)
/* x <- x + scal.y normalisé */
{
  for (int i=0; i<n; i++) x[i] += scal * y[i];
  return normalize(x, n);
}

void logit(double *target, double *y, int n)
/* target <- logit(y) */
/* Rappelons la formule du logit :
 * y[i] <- exp(-y[i]) / sum_j(exp(-y[j])). Pour éviter les débordements
 * flottants, on fait plutôt :
 * y[i] <- exp(y_min - y[i]) / sum_j(exp(y_min - y[j])) */
{
  double y_min = min(y, n);

  /* Calcul de \sum_j(exp y_min - y[j]) puis du logit normalisé */
  double s = 0;
  for (int i=0; i<n; i++) s += exp(y_min - y[i]);

  for (int i=0; i<n; i++) target[i] = exp(y_min - y[i]) / s;
  return ;
}

void pos_logit(double *target, double *y, int n)
/* target <- pos_logit et y n'est pas modifié */
{
  double w_max = max(y, n); //printf("max is %f\n", w_max);
  double s = 0;
  for (int i=0; i<n; i++) s += exp(y[i] - w_max);

  for (int i=0; i<n; i++) target[i] = exp(y[i] - w_max) / s;
  return ;
}

void balanced_logit(double *target, double *y, double e, int n)
/* target <- e.unif_n + (1 - e).logit(y)
 * la normalisation est inhérente à la formule */
{
  logit(target, y, n);
  for (int i=0; i<n; i++) target[i] = e / n + (1 - e) * target[i];
  return ;
}

void pos_balanced_logit(double *target, double *y, double e, int n)
/* idem à balanced_logit mais avec avec le logit positif */
{
  pos_logit(target, y, n);
  for (int i=0; i<n; i++) target[i] = e / n + (1 - e) * target[i];
  return ;
}

/* ********************* Bruit sphérique ********************* */

double Gaussian_rand(void)
/* Renvoie un nombre de loi N(0, 1) */
/* Box-Muller */
{
  double u1, u2;
  do {
    u1 = drand48();
    u2 = drand48();
  } while(u1 == 0.);

  return sqrt(-2 * log(u1)) * cos(2 * PI * u2);
}

double *spherical_noise(int n)
/* Renvoie un vecteur aléatoire uniforme dans la sphére unité de R^n */
/* https://pdfs.semanticscholar.org/467c/634bc770002ad3d85ccfe05c31e981508669.pdf */
{
  double *distrib = new_distrib(n);
  for (int i=0; i<n; i++) distrib[i] = Gaussian_rand();

  /* Normalisation */
  double norm2 = 0;
  for (int i=0; i<n; i++) norm2 += distrib[i] * distrib[i];
  norm2 = sqrt(norm2);
  for (int i=0; i<n; i++) distrib[i] /= norm2;

  return distrib;
}

/* ************************* LOI EXPONENTIELLE ************************* */

double rand_exponential(double lambda)
/* Renvoie un nombre aléatoire de loi E(lambda) */
{
  double u = drand48();
  return - log(1-u) / lambda;
}
