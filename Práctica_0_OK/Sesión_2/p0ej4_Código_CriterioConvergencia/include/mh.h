#ifndef _MH
#define _MH

typedef struct
{
	int *array_int;
	double fitness;
} Individuo;

void cruzar(Individuo *, Individuo *, Individuo *, Individuo *, int, int);
void mutar(Individuo *, int, int);
void fitness(const double *, Individuo *, int, int);
double aplicar_mh(const double *d, int n, int m, int n_gen, int tam_pob, double c_convergencia, int *sol);
#endif
