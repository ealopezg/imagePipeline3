#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include "../include/utils.h"


/**
 * @brief Carga la configuración obtenida de los argumentos
 *        Los argumentos necesarios para la ejecución del
 *        programa son: Numero de imagenes (C)
 *                      Umbral para binarizar la imagen (U)
 *                      Umbral para la clasificacion (N)
 *                      Nombre del archivo que contiene la mascara a utilizar (M)
 *        (b: Bandera que indica si se deben mostrar los resultados por pantalla) no
 *         es necesario para la ejecución.
 *         Devuelve una estructura Config o NULO en el caso de que falten los argu-
 *         mentos obligatorios.
 * 
 * @param argc 
 * @param argv 
 * @return Config* 
 */
Config* load_config(int argc, char const *argv[]){
    Config* config = (Config*)malloc(sizeof(Config));
    config->images = 0;
    int existImages = 0;
    config->bin_threshold = 0;
    int existBin_threshold = 0;
    config->rating_threshold = 0;
    int existRating_threshold = 0;
    config->lap_mask_file_name = NULL;
    int existLap_mask_file_name = 0;
    config->show = 0;
    int c;

    while ((c = getopt(argc,argv,"c:u:n:m:b")) != -1)
    switch (c)
    {
        case 'c':
            config->images = atoi(optarg);
            existImages = 1;
            break;
        case 'u':
            config->bin_threshold = atoi(optarg);
            existBin_threshold = 1;
            break;
        case 'n':
            config->rating_threshold = atoi(optarg);
            existRating_threshold = 1;
            break;
        case 'm':
            config->lap_mask_file_name = optarg;
            existLap_mask_file_name = 1;
            break;
        case 'b':
            config->show = 1;
            break;
        default:
            abort ();
    }
    if(existImages && existBin_threshold && existRating_threshold && existLap_mask_file_name){
        return config;
    }
    return NULL;
    
}


/**
 * @brief Función que lee el archivo con la máscara laplaciana y la guarda
 *         en la estructura de configuracion. La mascara se guarda en un arreglo
 *         de largo 9
 * 
 * @param c Estructura configuracion 
 * @return int Si es válida la apertura devuelve un 1 de lo contrario 0.
 */
int read_lap_mask(Config *c){
    FILE *fp;
    char str[10000];
    
    fp = fopen(c->lap_mask_file_name, "r");
    if (fp == NULL){
        return 0;
    }
    int lap_mask[9];
    int i = 0;
    int j = 0;
    char * token;
    // Va leyendo linea por linea y separa la linea en 3
    while (fgets(str, 10000, fp) != NULL){
        
        token = strtok(str, " ");
        lap_mask[i] = atoi(token);
        i = i+1;

        token = strtok(NULL, " ");
        lap_mask[i] = atoi(token);
        i = i+1;

        token = strtok(NULL, " ");
        lap_mask[i] = atoi(token);
        i = i+1;
    }
    fclose(fp);
    c->lap_mask = (int*)malloc(sizeof(int)*9);
    i = 0;
    while(i < 9){
        c->lap_mask[i] = lap_mask[i];
        i = i+1;
    }
    
    return 1;
}



