#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TAM_LINEA 16
#define NUM_FILAS 8
#define TAM_BUS 12
#define DIR_ETQ 5
#define DIR_LINEA 3
#define DIR_PALABRA 4

//Estructura de datos que contine cada linea de la cache
//Estando el struct debajo de los prototipos de funciones este no queda declarado
typedef struct{
	unsigned char ETQ;
	unsigned char Data[TAM_LINEA];
}T_CACHE_LINE;

//Prototipos de funciones utilizadas
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
void VolcarCACHE(T_CACHE_LINE *tbl);
void INTaBIN(unsigned int num, int *bin);
int BINaINT(int *bin, int pos);
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);
void AciertoCACHE(unsigned int *tiempo, unsigned int *fallos, unsigned int addr, int ETQ, int linea, int palabra);

int main(){
	unsigned int globaltime, numfallos, numaccesos, direccion, ETQ_int, linea_int, palabra_int;
	globaltime = 0;
	numfallos = 0;
	numaccesos = 0;
	direccion = 0;
	ETQ_int = 0;
	linea_int = 0;
	palabra_int = 0;

	float tiempomedio = 0;

	//Solo me deja poner todos los elementos a {0} si cada uno esta con su propio int
	//int ETQ_bin[5], linea_bin[3], palabra_bin[4];
	int ETQ_bin[DIR_ETQ] = {0};
	int linea_bin[DIR_LINEA] = {0}; 
	int palabra_bin[DIR_PALABRA] = {0};
	int bloque_int = 0;
	
	unsigned char Simul_RAM[4096];

	T_CACHE_LINE datos_cache[NUM_FILAS];

	FILE *p_CONTENTS_RAM;
	FILE *p_accesos_memoria;

	p_CONTENTS_RAM = fopen("CONTENTS_RAM.bin", "rb");
	p_accesos_memoria = fopen("accesos_memoria.txt", "r");

	if(p_CONTENTS_RAM  == NULL){
		printf("p_CONTENTS_RAM\n");
		return -1;
    }

    if(p_accesos_memoria == NULL){
		printf("p_accesos_memoria\n");
        return -1;
    }

	LimpiarCACHE(datos_cache);
	printf("Estado inicial de la cache:\n");
	VolcarCACHE(datos_cache);

	//Guardo los contenidos de CONTENTS_RAM en Simul_RAM 
	while(fscanf(p_CONTENTS_RAM, "%c", Simul_RAM) != EOF);
	
	//Bucle de funcionamiento principal del programa
	while(fscanf(p_accesos_memoria, "%X", &direccion) != EOF){
		ParsearDireccion(direccion, ETQ_bin, palabra_bin, linea_bin, &bloque_int);

		ETQ_int = BINaINT(ETQ_bin, DIR_ETQ);
		linea_int = BINaINT(linea_bin, DIR_LINEA);
		palabra_int = BINaINT(palabra_bin, DIR_PALABRA);
		
		printf("ETQ:%X\nLinea:%d\nPalabra:%d\n", ETQ_int, linea_int, palabra_int);

		if(datos_cache[linea_int].ETQ == ETQ_int){
			printf("T: %d, Acierto de CACHE, ADDR %04X Label %X linea %02X palabra %02X DATO %02X", globaltime, direccion, ETQ_int, linea_int, palabra_int, datos_cache[linea_int].Data[palabra_int]);
		}else{
			numfallos++;
			TratarFallo(datos_cache, Simul_RAM, ETQ_int, linea_int, bloque_int);
			printf("T: %d, Fallo de CACHE %d, ADDR %04X Label %X linea %02X palabra %02X bloque %02X\n", globaltime, numfallos, direccion, ETQ_int, linea_int, palabra_int, bloque_int);
			printf("Cargando el bloque %02X en la linea %02X\n", bloque_int, linea_int);
			globaltime += 10;
		}
		VolcarCACHE(datos_cache);

		numaccesos++;
		//sleep(1);
	}
	tiempomedio = globaltime / numaccesos;
	printf("Accesos totales: %d; fallos: %d; Tiempo medio: %.2f\n", numaccesos, numfallos, tiempomedio);
	
	fclose(p_CONTENTS_RAM);
	fclose(p_accesos_memoria);

	return 0;
}

//Funcion que reinicia la cache
//La etiqueta a 0xFF y los datos a 0x23 en cada linea
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]){
	for(int i = 0; i < NUM_FILAS; i++){
        tbl[i].ETQ = 0xFF;
		
        for(int j = 0; j < TAM_LINEA; j++){
            tbl[i].Data[j] = 0x23;
        }
    }
}

//Funcion que muestra el estado actual de la cache por pantalla
void VolcarCACHE(T_CACHE_LINE *tbl){
    for(int i = 0; i < NUM_FILAS; i++){
        printf("ETQ:%02X  DATA", tbl[i].ETQ);
		
        for(int j = 0; j < TAM_LINEA; j++){
            printf(" %02X", tbl[i].Data[j]);
        }
        printf("\n");
    }
    printf("\n");
}

//Funcion que traduce valores int a arrays de bits eqivalentes
void INTaBIN(unsigned int num, int *bin){
	div_t division;
	int aux_bin[TAM_BUS] = {0};
	
	for(int i = 0; i < TAM_BUS; i++){
		division = div(num, 2);
		num = division.quot;
		aux_bin[i] = division.rem;
	}
	
	for(int i = 0, j = TAM_BUS - 1; i < TAM_BUS; i++, j--){
		bin[i] = aux_bin[j];
	}
}

//Funcion que traduce arrays de bits a valores int eqivalentes
int BINaINT(int *bin, int pos){
	int resultado, j; 
	j = 0;
	resultado = 0;

	for(double i = pos - 1; i >= 0; i--, j++){
		resultado += (int)pow(2,i) * bin[j]; //La funcion pow() esta pensada para double
	}
	
	return resultado;
}

//Funcion que divide una direccion de memoria en sus respectivos campos
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque){
	int addr_bin[TAM_BUS] = {0};
	INTaBIN(addr, addr_bin);
	//ETQ(5 bit): 0-4, Linea(3 bit): 5-6, Palabra(4 bit): 7-11
	//Campo ETQ
	for(int i = 0; i < DIR_ETQ; i++){
		ETQ[i] = addr_bin[i];
	}

	//Campo Linea
	for(int i = 0; i < DIR_LINEA; i++){
		linea[i] = addr_bin[i + DIR_ETQ];
	}

	//Campo Palabra
	for(int i = 0; i < DIR_PALABRA; i++){
		palabra[i] = addr_bin[i + DIR_ETQ + DIR_LINEA];
	}
	
	//Bloque al que pertenece la direccion de memoria
	*bloque = BINaINT(ETQ, DIR_ETQ) * NUM_FILAS + BINaINT(linea, DIR_LINEA);
}

void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque){
	tbl[linea].ETQ = ETQ;
	
	for(int i = 0; i < TAM_LINEA; i++){
		tbl[linea].Data[i] = MRAM[bloque * TAM_LINEA + i];
	}
}

void AciertoCACHE(unsigned int *tiempo, unsigned int *fallos, unsigned int addr, int ETQ, int linea, int palabra){
	
}
