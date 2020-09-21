#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../include/image.h"


#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

/**
 * @brief Función que abre una imagen
 * 
 * @param filename Nombre del archivo
 * @return Image* Estructura de la imagen
 */
Image* open_image(char* filename){
    Image* img = (Image*)malloc(sizeof(Image));
    img->data = stbi_load(filename,&img->width,&img->height,&img->channels,0);
    if(img->data == NULL){
        free(img->data);
        free(img);
        return NULL;
    } 
    return img;
}

/**
 * @brief Función que libera de memoria la imagen
 * 
 * @param img Estructura Imagen
 */
void free_image(Image *img){
    if(img->data != NULL){
        stbi_image_free(img->data);
    } 
    free(img);
}

/**
 * @brief Función que guarda una estructura Imagen en un archivo
 *        con formato jpg
 * 
 * @param filename Nombre del archivo a guardar
 * @param img Estructura Imagen
 */
void write_image(char* filename, Image *img){
    stbi_write_jpg(filename, img->width, img->height, img->channels, img->data, 100);
}

Image * empty_image(int width,int height,int channels){
    Image * img = malloc(sizeof(Image));
    img->data = malloc(sizeof(uint8_t)*width*height*channels);
    img->channels = channels;
    img->height = height;
    img->width = width;
    return img;
}


void write_lines_to_image(Image * img,uint8_t * data,int start, int heigth){
    memcpy(img->data + (start*img->width*img->channels),data,img->width*img->channels*heigth);
}