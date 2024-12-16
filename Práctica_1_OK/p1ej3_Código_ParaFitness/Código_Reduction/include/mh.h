#ifndef _MH
#define _MH
	
	typedef struct {
		int *array_int;
		double fitness;
	} Individuo;
	
	void cruzar(Individuo *, Individuo *, Individuo *, Individuo *, int, int);
	void mutar(Individuo *, int, int);
	void fitness(const double *, Individuo *, int, int);
#endif
