#ifndef fun_h
#define fun_h

/* Je définis ici des fonctions utilisées pour les réseaux */

/* Fonctions affines */

double fun_zero(double x);
double fun_cst (double x);
double fun_dcst(double x);
double fun_d2cst(double x);

double fun_lin(double x);
double fun_dlin(double x);
double fun_d2lin(double x);

double fun_aff(double  x);
double fun_daff(double x);
double fun_d2aff(double x);

/* Fonctions polynomiales */

double fun_deg3(double  x);
double fun_ddeg3(double x);
double fun_d2deg3(double x);

/* Fonctions en 1/(a - x) */

double fun_inv  (double x);
double fun_dinv (double x);
double fun_inv2 (double x);
double fun_dinv2(double x);

#endif
