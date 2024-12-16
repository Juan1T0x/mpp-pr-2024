void mezclar(Individuo **poblacion, int izq, int med, int der)
{	
	int i, j, k;
	
	Individuo **pob = (Individuo **) malloc((der - izq)*sizeof(Individuo *));
	assert(pob);
	
	for(i = 0; i < (der - izq); i++) {
		pob[i] = (Individuo *) malloc(sizeof(Individuo));
	}
	
	k = 0;
	i = izq;
	j = med;
	while( (i < med) && (j < der) ) {
		if (poblacion[i]->fitness < poblacion[j]->fitness) { // considerar "<" o ">" según sea el problema de minimización o de maximización
			// copiar poblacion[i++] en pob[k++] // por ejemplo con memmove
		}
		else {
			// copiar poblacion[j++] en pob[k++]
		}
	}
	
	for(; i < med; i++) {
		// copiar poblacion[i] en pob[k++]
	}
	
	for(; j < der; j++) {
		// copiar poblacion[j] en pob[k++]
	}
	
	i = 0;
	while(i < (der - izq)) {
		// copiar pob[i] en poblacion[i + izq]
		// ...
		// liberar individuo 'i'
		free(pob[i]);
		i++;
	}
	free(pob);
}
