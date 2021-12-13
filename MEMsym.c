#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#define TAM_LINEA 16
#define NUM_FILAS 8
#define TAM_BUS 12
#define TAM_RAM 4096
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
void AciertoCACHE(T_CACHE_LINE *tbl, unsigned int tiempo, unsigned int addr, int ETQ, int linea, int palabra, unsigned char *lectura, int *contador);

int main(){
	//Inicializo las variables necesarias
	unsigned int globaltime, numfallos, numaccesos, direccion, ETQ_int, linea_int, palabra_int;
	globaltime = 0;
	numfallos = 0;
	numaccesos = 0;
	direccion = 0;
	ETQ_int = 0;
	linea_int = 0;
	palabra_int = 0;

	float tiempomedio = 0;

	int ETQ_bin[DIR_ETQ] = {0};
	int linea_bin[DIR_LINEA] = {0};
	int palabra_bin[DIR_PALABRA] = {0};
	int bloque_int, contador_texto;
	bloque_int = 0;
	contador_texto = 0;

	unsigned char Simul_RAM[TAM_RAM], texto[100];

	T_CACHE_LINE datos_cache[NUM_FILAS];

	//Abro los ficheros CONTENTS_RAM.bin, accesos_memoria.txt y CONTENTS_CACHE.bin
	FILE *p_CONTENTS_RAM;
	FILE *p_accesos_memoria;
	FILE *p_CONTENTS_CACHE;
	p_CONTENTS_RAM = fopen("CONTENTS_RAM.bin", "rb");
	p_accesos_memoria = fopen("accesos_memoria.txt", "r");
	p_CONTENTS_CACHE = fopen("CONTENTS_CACHE.bin", "wb");

	//Control de errores de p_CONTENTS_RAM
	if(p_CONTENTS_RAM  == NULL){
		printf("Error al abrir CONTENTS_RAM.bin\n");
		return -1;
    }

	//Control de errores de p_accesos_memoria
    if(p_accesos_memoria == NULL){
		printf("Error al abrir accesos_memoria.txt\n");
        return -1;
    }

	//Control de errores de p_CONTENTS_CACHE
	if(p_CONTENTS_CACHE == NULL){
		printf("Error al crear CONTENTS_CACHE.bin\n");
        return -1;
    }

	//Guardo los contenidos de CONTENTS_RAM en Simul_RAM
	fread(Simul_RAM, 1, TAM_RAM, p_CONTENTS_RAM);

	//Reinicio los contenidos de la cache y seguidamente la muestro por pantalla
	LimpiarCACHE(datos_cache);
	printf("Estado inicial de la cache:\n");
	VolcarCACHE(datos_cache);

	//Bucle principal de funcionamiento del programa
	while(fscanf(p_accesos_memoria, "%X", &direccion) != EOF){
		//Divido la direccion de memoria solicitada en sus respectivos campos y calculo el numero de bloque
		ParsearDireccion(direccion, ETQ_bin, palabra_bin, linea_bin, &bloque_int);

		//Combierto de binario a decimal/hexadecial los campos de la direccion de memoria
		ETQ_int = BINaINT(ETQ_bin, DIR_ETQ);
		linea_int = BINaINT(linea_bin, DIR_LINEA);
		palabra_int = BINaINT(palabra_bin, DIR_PALABRA);

		//Obtengo el numero de linea y compruebo que la etiqueta de dicha linea de la cache es igual a la de la direccion
		//En ese caso es un acierto de cache, en el contrario en un fallo de cache
		if(datos_cache[linea_int].ETQ == ETQ_int){
			//Aunque no fuese especificado en el enunciado, por intuicion, incremento una unidad globaltime
			globaltime++;
			AciertoCACHE(datos_cache, globaltime, direccion, ETQ_int, linea_int, palabra_int, texto, &contador_texto);
		}else{
			numfallos++;
			printf("T: %d, Fallo de CACHE %d, ADDR %04X Label %X linea %02X palabra %02X bloque %02X\n", globaltime, numfallos, direccion, ETQ_int, linea_int, palabra_int, bloque_int);
			globaltime += 10;
			TratarFallo(datos_cache, Simul_RAM, ETQ_int, linea_int, bloque_int);
			printf("Cargando el bloque %02X en la linea %02X\n", bloque_int, linea_int);
			AciertoCACHE(datos_cache, globaltime, direccion, ETQ_int, linea_int, palabra_int, texto, &contador_texto);
		}

		//Muestro el contenido de la cache despues de realizar la operacion anterior
		VolcarCACHE(datos_cache);

		//Incremento el contador de accesos una unidad
		numaccesos++;

		//Hago el sleep() de un segundo
		sleep(1);
	}

	//Calculo el tiempo medio de acceso
	tiempomedio = (float)globaltime / (float)numaccesos;

	printf("Accesos totales: %d; fallos: %d; Tiempo medio: %.2f\n", numaccesos, numfallos, tiempomedio);
	texto[contador_texto] = '\0';
	printf("Texto leido: %s\n", texto);

	//Escribo los contenidos de la cache en el fichero CONTENTS_CACHE.bin
	for(int i = 0; i < NUM_FILAS; i++){
		fwrite(datos_cache[i].Data, TAM_LINEA, 1, p_CONTENTS_CACHE);
	}

	//Cierro los ficheros previamente abiertos
	fclose(p_CONTENTS_RAM);
	fclose(p_accesos_memoria);
	fclose(p_CONTENTS_CACHE);

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
		/*
        for(int j = 0; j < TAM_LINEA; j++){
            printf(" %02X", tbl[i].Data[j]);
        }
		*/
		for(int j = TAM_LINEA - 1; j >= 0; j--){
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

//Funcion que trae los datos solicitados que no se encuentran el la cache desde la RAM
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque){
	tbl[linea].ETQ = ETQ;

	for(int i = 0; i < TAM_LINEA; i++){
		tbl[linea].Data[i] = MRAM[bloque * TAM_LINEA + i];
	}
}

//Funcion que imprime por pantalla la informacion sujeta a un acierto de cache y guarda los datos leidos en un array
void AciertoCACHE(T_CACHE_LINE *tbl, unsigned int tiempo, unsigned int addr, int ETQ, int linea, int palabra, unsigned char *lectura, int *contador){
	printf("T: %d, Acierto de CACHE, ADDR %04X Label %X linea %02X palabra %02X DATO %02X\n", tiempo, addr, ETQ, linea, palabra, tbl[linea].Data[palabra]);
	lectura[*contador] = tbl[linea].Data[palabra];
	(*contador)++;
}

