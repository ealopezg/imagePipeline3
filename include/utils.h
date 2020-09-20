#ifndef UTILS_H
#define UTILS_H



//Estructura configuracion
typedef struct Config
{
    int images; //Cantidad de imagenes
    int threads; //Cantidad de hebras
    int bin_threshold; //Umbral para binarizar la imagen
    int rating_threshold; //Umbral para calificar la imagen
    int buff_size; //Tamaño del buffer
    int lap_mask[9]; //Arreglo de una dimension con la mascara para aplicar el filtro laplaciano
    int show; //Bandera para mostrar o no los resultados en pantalla
} Config;

Config* load_config(int argc, char const *argv[]);






#endif