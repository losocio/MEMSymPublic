#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
void MostrarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
void INTaBIN(unsigned int num, int *bin);
int BINaINT(int *bin, int pos);
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);

int main(){
        unsigned int globaltime, numfallos, direccion, fin_fichero;
        globaltime = 0;
        numfallos = 0;
        direccion = 0;
        fin_fichero = 1;
		
		//Solo me deja poner todos los elementos a {0} si cada uno esta con su propio int
		//int addr_bin[TAM_BUS], ETQ[5], linea[3], palabra[4], bloque[4];
		int ETQ_bin[DIR_ETQ] = {0};
		int linea_bin[DIR_LINEA] = {0}; 
		int palabra_bin[DIR_PALABRA] = {0};
		int bloque_bin[DIR_PALABRA] = {0};
        
		unsigned char Simul_RAM[4096];

        T_CACHE_LINE datos_cache[NUM_FILAS];

        FILE *p_CONTENTS_RAM;
        FILE *p_accesos_memoria;

        p_CONTENTS_RAM = fopen("CONTENTS_RAM.bin", "rb");
		p_accesos_memoria = fopen("accesos_memoria.txt", "r");
		
		//Control de errores de lectura de ficheros
		/*      	
		Este me da problemas, no se porque
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
		
		//Guardo los contenidos de CONTENTS_RAM en Simul_RAM 
        while(fscanf(p_CONTENTS_RAM, "%c", Simul_RAM) != EOF);

        do{
                //while(fscanf(p_accesos_memoria, "%X", &addr) != '\n');
				//BUG, puede mejorarse. Con fscanf estoy mas perdido que un pulpo en un garage 
                if(fscanf(p_accesos_memoria, "%X", &direccion) != EOF) printf("%X\n", direccion);
                else fin_fichero = 0;
				
				ParsearDireccion(direccion, ETQ_bin, palabra_bin, linea_bin, bloque_bin);
				
				//printf("ETQ:%d\nLinea:%d\nPalabra:%d\n", BINaINT(ETQ_bin, DIR_ETQ), BINaINT(linea_bin, DIR_LINEA), BINaINT(palabra_bin, DIR_PALABRA));
				
				if(datos_cache[BINaINT(linea_bin, DIR_LINEA)].ETQ == BINaINT(ETQ_bin, DIR_ETQ)){
					//AciertoCACHE();
				}else{
					//TratarFallo();
				}
				
                //sleep(1);
        }while(fin_fichero);

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
void MostrarCACHE(T_CACHE_LINE tbl[NUM_FILAS]){
        for(int i = 0; i < NUM_FILAS; i++){
                printf("ETQ:%X  DATA", tbl[i].ETQ);
                for(int j = 0; j < TAM_LINEA; j++){
                        printf(" %X", tbl[i].Data[j]);
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

void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque){
	int addr_bin[TAM_BUS] = {0};
	INTaBIN(addr, addr_bin);
	//ETQ(5 bit): 0-4, Linea(3 bit): 5-6, Palabra(4 bit): 7-11
	//ETQ
	for(int i = 0; i < DIR_ETQ; i++){
		ETQ[i] = addr_bin[i];
	}
				
	//Linea
	for(int i = 0; i < DIR_LINEA; i++){
		linea[i] = addr_bin[i + DIR_ETQ];
	}
				
	//Palabra
	for(int i = 0; i < DIR_PALABRA; i++){
		palabra[i] = addr_bin[i + DIR_ETQ + DIR_LINEA];
	}
}

