#ifndef _MH
#define _MH

#ifndef M
#define M 2000
#endif

typedef struct
{
	int array_int[M]; // Array estático de longitud M, el mayor tamaño posible
	double fitness;
} Individuo;

// Declaraciones de funciones
void cruzar(const Individuo *, const Individuo *, Individuo *, Individuo *, int, int);
void mutar(Individuo *, int, int);
void fitness(const double *, Individuo *, int, int);

#endif