/* Descripcion: Clase que realiza el problema de la mochila
* Autor: Víctor del Valle del Apio
* e-mail: victorvalleapio@gmail.com
* Fecha: 03-07-2013
*/

#include <iostream>;
#include <fstream>;
#include <queue>
#include "timer.h"
#include <stdlib.h>;
#include <time.h>;
using namespace std;

int const N = 100;
int const M = 250;

struct solucionPD{
	double valor;
	int cuales[N];
};

solucionPD mochila_pd(double P[], double V[], int n){

	solucionPD sol;
	double mochila[N+1][M+1];

	for(int i = 0; i <= n; i++) mochila[i][0] = 0;
	for(int i = 1; i <= M; i++) mochila[0][i] = 0;

	for(int i = 1; i <= n; i++){
		for(int j = 1; j <= M; j++){
			//Indice de i tiene que ser uno menos en los arrays de pesos y valores
			if (P[i-1] > j) 
				mochila[i][j] = mochila[i-1][j];
			else
				mochila[i][j] = max(mochila[i-1][j], mochila[i-1][j-(int)P[i-1]] + V[i-1]);//Al ser el peso un double, hay que hacerle el casting		}
	}

	sol.valor = mochila[n][M];
	int resto = M;

	//Calculo de objetos
	for(int i = n; i > 0; i--){

		if (mochila[i][resto] == mochila[i-1][resto]) //No cogemos el objeto i 
			sol.cuales[i-1] = 0; //Con el array solucion ocurre lo mismo que con los pesos y los valores
		else{
			sol.cuales[i-1] = 1;
			resto = resto - P[i-1];
		}	
	}

	return sol;
}

struct estimacionRP{
	double opt;
	double pes;
};

struct solucionRP{
	int sol_mejor[N];
	double beneficio_mejor;
};

class Nodo{
	public:
		int sol[N];
		int k;
		double peso, beneficio;
		double beneficio_opt; //Prioridad

	friend bool operator< (const Nodo &n1, const Nodo &n2);
};

bool operator<(const Nodo &n1, const Nodo &n2){
	return n1.beneficio_opt < n2.beneficio_opt;
}

estimacionRP calculo_estimaciones(double P[], double V[], int k, double peso, double beneficio, int n){

	estimacionRP estim;
	double hueco = M - peso; estim.pes = beneficio; estim.opt = beneficio;
	int j = k + 1;

	while (j < n && P[j] <= hueco){
		hueco = hueco - P[j]; 
		estim.opt = estim.opt + V[j];
		estim.pes = estim.pes + V[j];
		j++;
	}

	if (j < n){
		estim.opt = estim.opt + (hueco / P[j]) * V[j];
		j++;
		
		while (j < n && hueco > 0){
			if (P[j] <= hueco){
				hueco = hueco - P[j];
				estim.pes = estim.pes + V[j];
			}
			j++;
		}
	}
	return estim;
}

solucionRP mochila_rp(double P[], double V[], int n){

	solucionRP sol;
	Nodo X, Y;
	priority_queue<Nodo> C;

	//Generamos la raiz
	Y.k = -1; Y.peso = 0; Y.beneficio = 0;

	estimacionRP estim = calculo_estimaciones(P, V, Y.k, Y.peso, Y.beneficio, n);
	Y.beneficio_opt = estim.opt;
	sol.beneficio_mejor = estim.pes;

	C.push(Y);

	while (!C.empty() && C.top().beneficio_opt >= sol.beneficio_mejor){

		Y = C.top(); C.pop();
		X.k = Y.k + 1; 
		for(int i = 0; i < n; i ++) X.sol[i] = Y.sol[i];

		//Probamos a meter el objeto en la mochila
		if (Y.peso + P[X.k] <= M){

			X.sol[X.k] = 1; X.peso = Y.peso + P[X.k]; X.beneficio = Y.beneficio + V[X.k]; X.beneficio_opt = Y.beneficio_opt;

			if (X.k == n-1){
				for(int i = 0; i < n; i ++) sol.sol_mejor[i] = X.sol[i];
				sol.beneficio_mejor = X.beneficio;
			}
			else{
				C.push(X);
				//No se puede mejorar el beneficio_mejor
			}
		}

		//Probamos a no meter el objeto en la mochila
		estim = calculo_estimaciones(P, V, X.k, Y.peso, Y.beneficio, n);
		X.beneficio_opt = estim.opt;
		double pes = estim.pes;

		if (X.beneficio_opt >= sol.beneficio_mejor){

			X.sol[X.k] = 0; X.peso = Y.peso; X.beneficio = Y.beneficio;

			if (X.k == n-1){
				for(int i = 0; i < n; i ++) sol.sol_mejor[i] = X.sol[i];
				sol.beneficio_mejor = X.beneficio;
			}
			else{
				C.push(X);
				sol.beneficio_mejor = max(sol.beneficio_mejor, pes);
			}
		}
	}

	return sol;
}

struct solucionVA{
	double beneficio;
	int sol[N];
};

double calculo_estimacion(double P[], double V[], int k, double peso, double beneficio, int n){

	double hueco = M - peso; double estimacion = beneficio;
	int j = k + 1;

	while (j < n && P[j] <= hueco){

		hueco = hueco - P[j]; 
		estimacion = estimacion + V[j];
		j++;
	}

	return estimacion;
}

void mochila_va(double P[], double V[], int solucion[], int* solucion_mejor, int k, double peso, double beneficio, double& beneficio_mejor, int n){
	
	//Cojo el objeto
	solucion[k] = 1;
	//Marco el objeto
	peso = peso + P[k]; beneficio = beneficio + V[k];
	//No hago estimacion

	if (peso <= M){
		if (k == n-1){
			for (int i = 0; i < n; i++) solucion_mejor[i] = solucion[i];
			beneficio_mejor = beneficio;
		}
		else 
			mochila_va(P, V, solucion, solucion_mejor, k+1, peso, beneficio, beneficio_mejor, n);
	}
					
	//Desmarco el objeto
	peso = peso - P[k]; beneficio = beneficio - V[k];
	//No cojo el objeto
	solucion[k] = 0;
	//Hago la estimacion
	double beneficio_estimado = calculo_estimacion(P, V, k, peso, beneficio, n);

	if (beneficio_estimado > beneficio_mejor){
		if (k == n-1){
			for (int i = 0; i < n; i++) solucion_mejor[i] = solucion[i];
			beneficio_mejor = beneficio;
		}
		else 
			mochila_va(P, V, solucion, solucion_mejor, k+1, peso, beneficio, beneficio_mejor, n);
	}
}

solucionVA mochila_principal(double P[], double V[], int n){

	solucionVA sol;
	double beneficio_mejor = -1;
	int solucion[N], solucion_mejor[N];

	mochila_va(P, V, solucion, solucion_mejor, 0, 0, 0, beneficio_mejor, n);

	sol.beneficio = beneficio_mejor;
	for (int i = 0; i < n; i++) sol.sol[i] = solucion_mejor[i];

	return sol;
}

void prueba_algoritmo_sobre_permutacion(int repeticiones, float min_tiempo_ms, std::ostream &out) {
	double V[N];
	double P[N];

	//Insertamos objetos aleatoriamente
	srand(time(NULL));
	for(int j = 1; j<N; j++){
		V[j]=1 + rand()%N;
		P[j]=1 + rand()%20;
	}
	//Ordenamos los objetos
    double aux1, aux2;
    int j;
    for (int i=1; i<N; i++) {
	   aux1 = V[i];
	   aux2 = P[i];
       for (j=i-1; j>=0 && V[j]/P[j]<aux1/aux2; j--){
          V[j+1] = V[j];
		  P[j+1] = P[j];
          V[j] = aux1;
		  P[j] = aux2;
        }
     }

	Timer t;
	for(int i = 1; i<=N; i++){	
		float tiempo_total = 0;
		for (int num_rep = 0; num_rep < repeticiones; num_rep ++) {
			int repeticiones_internas = 0;
			float tiempo = 0;
			do {			
				repeticiones_internas ++;
				t.start();

				//Descomentamos el que queramos examinar

				//mochila_rp(P, V, i); //Ramificacion y Poda
				mochila_pd(P, V, i); //Programacion dinamica
				//mochila_principal(P, V, i); //Vuelta Atras
				
				tiempo = t.pause();			
			} while (tiempo < min_tiempo_ms);
			
			tiempo_total += t.stop()/repeticiones_internas;			
		}
		out << i << " " << tiempo_total/repeticiones << std::endl;
		std::cout << ".";
		std::cout.flush();
	}
	std::cout << "*\n";
}

int main(){
	Timer t;
	t.start();
	
	std::ofstream out1("Nombre.txt");
	prueba_algoritmo_sobre_permutacion(100, 100, out1);
	out1.close();
	
	std::cout << "tiempo total invertido: " << (t.stop() / 1000.0) << " segundos\n";
	return 0;
}
