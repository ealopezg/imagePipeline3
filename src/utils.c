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

    config->threads = 0;
    int existThreads = 0;

    config->bin_threshold = 0;
    int existBin_threshold = 0;
    config->rating_threshold = 0;
    int existRating_threshold = 0;

    config->buff_size = 0;
    int existBuff = 0;

    config->show = 0;
    int c;

    while ((c = getopt(argc,argv,"c:h:u:n:b:f")) != -1)
    switch (c)
    {
        case 'c':
            config->images = atoi(optarg);
            existImages = 1;
            break;
        case 'h':
            config->threads = atoi(optarg);
            existThreads = 1;
            break;
        case 'u':
            config->bin_threshold = atoi(optarg);
            existBin_threshold = 1;
            break;
        case 'n':
            config->rating_threshold = atoi(optarg);
            existRating_threshold = 1;
            break;
        case 'b':
            config->buff_size = atoi(optarg);
            existBuff = 1;
            break;
        case 'f':
            config->show = 1;
            break;
        default:
            abort ();
    }
    if(existImages && existThreads && existBin_threshold && existRating_threshold && existBuff){
        return config;
    }
    return NULL;
    
}
