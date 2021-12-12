#include <stdio.h>

#define TAM_LINEA 16
#define NUM_FILAS 8

//Estructura de datos que contine cada linea de la cache
//Estando el struct debajo de los prototipos de funciones este no queda declarado
typedef struct{
	unsigned char ETQ;
	unsigned char Data[TAM_LINEA];
}T_CACHE_LINE;

//Prototipos de funciones necesarias
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
void MostrarCACHE(T_CACHE_LINE *tbl);

int main(){
	unsigned int globaltime, numfallos;
	globaltime = 0;
	numfallos = 0;

	T_CACHE_LINE datos_cache[NUM_FILAS];

	LimpiarCACHE(datos_cache);
	MostrarCACHE(datos_cache);

	return 0;
}

//Funcion que reinicia la cache
//La etiqueta a 0xFF y los datos a 0x23 en cada linea
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]){
	int i, j;
	for(i = 0; i < NUM_FILAS; i++){
		tbl[i].ETQ = 0xFF;
		for(j = 0; j < TAM_LINEA; j++){
			tbl[i].Data[j] = 0x23;
		}
	}
}

//Funcion que muestra el estado actual de la cache por pantalla
void MostrarCACHE(T_CACHE_LINE *tbl){
	int i, j;
	for(i = 0; i < NUM_FILAS; i++){
		printf("ETQ:%X  DATA", tbl[i].ETQ);
		for(j = 0; j < TAM_LINEA; j++){
			printf(" %X", tbl[i].Data[j]);
		}
		printf("\n");
	}
	printf("\n");
}
