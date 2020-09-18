#ifndef UTILS_H
#define UTILS_H



//Estructura configuracion
typedef struct Config
{
    int images; //Cantidad de imagenes
    int bin_threshold; //Umbral para binarizar la imagen
    int rating_threshold; //Umbral para calificar la imagen
    char* lap_mask_file_name; //Nombre del archivo que contiene la mascara de filtro laplaciano
    int* lap_mask; //Arreglo de una dimension con la mascara para aplicar el filtro laplaciano
    int show; //Bandera para mostrar o no los resultados en pantalla
} Config;

int read_lap_mask(Config *c);
Config* load_config(int argc, char const *argv[]);






#endif