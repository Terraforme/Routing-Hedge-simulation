#include "fun.h"

/* Fonctions affines */

double fun_zero(double x) { x = x; return 0; }
double fun_cst (double x) { x = x; return 1; }
double fun_dcst(double x) { x = x; return 0; }
double fun_d2cst(double x) { x = x; return 0; }

double fun_lin(double x) { return x; }
double fun_dlin(double x) { x=x; return 1; }
double fun_d2lin(double x) { x = x; return 0; }

double fun_aff(double  x) { return x+1; }
double fun_daff(double x) { x=x; return 1; }
double fun_d2aff(double x) { x = x; return 0; }

/* Fonctions polynomiales */

double fun_deg3(double  x)  { return x*x*x + 1; }
double fun_ddeg3(double x)  { return 3*x*x; }
double fun_d2deg3(double x) { return 6*x; }

/* Fonctions en 1/(a - x) */

double fun_inv  (double x) { return 1 / (2 - x); }
double fun_dinv (double x) { return 1 / ((2-x)*(2-x)); }
double fun_inv2 (double x) { return 1 / ((2-x)*(2-x)); }
double fun_dinv2(double x) { return 1 / ((2-x)*(2-x)*(2-x)); }
