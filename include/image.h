#ifndef IMAGE_H
#define IMAGE_H

//Estructura imagen
typedef struct Image
{
    int width; //Ancho
    int height; //Alto
    int channels; //Cantidad de canales
    uint8_t * data; //Arreglo de pixeles
} Image;

Image* open_image(char* filename);
void write_image(char* filename, Image *img);
void free_image(Image *img);
void print_image(Image *img);
Image * empty_image(int width,int height,int channels);
void write_lines_to_image(Image * img,uint8_t * data,int start, int heigth);

#endif 
