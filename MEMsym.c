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
void MostrarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);

int main(){
	unsigned int globaltime, numfallos, addr, fin_fichero;
	globaltime = 0;
	numfallos = 0;
	addr = 0;
	fin_fichero = 1;

	unsigned char Simul_RAM[4096];

	T_CACHE_LINE datos_cache[NUM_FILAS];

	FILE *p_CONTENTS_RAM;
	FILE *p_accesos_memoria;

	p_CONTENTS_RAM = fopen("CONTENTS_RAM.bin", "rb");
	p_accesos_memoria = fopen("accesos_memoria.txt", "r");

/*	Este me da problemas, no se porque
	if(p_CONTENTS_RAM || p_accesos_memoria == NULL){
                printf("Alguno de los ficheros que ha intentado leer no existe\n");
                return -1;
        }
*/

	if(p_CONTENTS_RAM  == NULL){
		printf("p_CONTENTS_RAM\n");
		return -1;
	}

	if(p_accesos_memoria == NULL){
                printf("p_accesos_memoria\n");
                return -1;
        }

	LimpiarCACHE(datos_cache);
        MostrarCACHE(datos_cache);

	while(fscanf(p_CONTENTS_RAM, "%c", Simul_RAM) != EOF);

	do{
		//while(fscanf(p_accesos_memoria, "%X", &addr) != '\n');
		if(fscanf(p_accesos_memoria, "%X", &addr) != EOF) printf("%X\n", addr);
		else fin_fichero = 0;


		//sleep(1);
	}while(fin_fichero);

	fclose(p_CONTENTS_RAM);
	fclose(p_accesos_memoria);

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
void MostrarCACHE(T_CACHE_LINE tbl[NUM_FILAS]){
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
