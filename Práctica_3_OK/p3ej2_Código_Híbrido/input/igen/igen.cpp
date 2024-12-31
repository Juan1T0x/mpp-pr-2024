#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

using namespace std;

#define min 0
#define max 100

void generar_d(double *d, int n, int m)
{
  int i;
  double f;

  srand(time(NULL) + getpid());

  for(i=0;i<(n*n-n)/2;i++)
  {
 	f = (double) rand() / ((double) RAND_MAX + 1);
	*(d + i) = min + f*(max - min);
  }
}

void escribir(double *d, int n, int m, int n_gen, int tam_pob)
{
  int i, j, pos=0;

  //cout << "# Parámetros: n m n_gen tam_pob " << endl;
  //cout << n << " " << m << " " << n_gen << " " << tam_pob << endl;

  for(i=0;i<n;i++)
  {
     for(j=i+1;j<n;j++)
     {
         cout << i << " " << j << " " << fixed << setprecision(2) << d[pos] << " " << endl;
         pos+=1;
     }
  }
  cout << endl;
}

int main(int argc,char **argv)
{
        //	Check Number of Input Args
	if(argc < 4) {
		cerr << "Ayuda: " << endl;
		cerr << "  ./programa n m nGen tamPob " << endl;
		return(EXIT_FAILURE);
	}

	int n = atoi(argv[1]);
	int m = atoi(argv[2]);
        int n_gen = atoi(argv[3]);
	int tam_pob = atoi(argv[4]);

        double *d = new double[(n*n-n)/2];

	generar_d(d, n, m);
	escribir(d, n, m, n_gen, tam_pob);

        delete [] d;
}

