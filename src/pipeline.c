#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../include/image.h"
#include "../include/utils.h"

#include "../include/pipeline.h"


/**
 * @brief Función que devuelve el valor de la función a 
 *        aplicar en la Mascara de filtro laplaciano.
 *        
 * 
 * @param p Pixel escogido
 * @param img Estructura imagen
 * @param c Configuracion
 * @return uint8_t 
 */
uint8_t laplace(uint8_t * p,int i,int j,uint8_t * data,int width,int height,int channels,int * lap_mask){
    // Se inicializa el valor inicial, en esta variable se irá guardando
    // El resultado de la función
    uint8_t pixel = 0;
    uint8_t * pivot; // Pivote para recorrer la imagen

    /**
     * @brief Primero es importante saber como la librería STBI para el manejo
     * de imágenes funciona. Al abrir una imagen, esta queda guardada como un
     * arreglo de dimensión 1 de largo: largo*ancho, donde cada pixel tiene el largo en base
     * a la cantidad de canales (en este caso 1 ya que es escala de grises).
     * Para poder recorrer y simular como si fuera una matriz, basta con ir sumando y restando
     * valores al puntero hasta llegar al pixel deseado.
     * 
     * Se necesita buscar los pixeles adyacentes, es decir:
     *   |---|---|---|
     *   | 0 | 1 | 2 |
     *   |---|---|---|
     *   | 3 | 4 | 5 |
     *   |---|---|---|
     *   | 6 | 7 | 8 |
     *   |---|---|---|
     * 
     * Donde el pixel 4 es el pixel seleccionado (p).
     * Luego se recorre utilizando el pivote para obtener los pixeles e ir multiplicando en
     * base a la matriz laplaciana(c->lap_mak)
     * 
     * Como las dimensiones son conocidas, no es necesario hacer un ciclo,
     * El orden de lo que está abajo es buscar los pixeles: 0-1-2 3-4-5 6-7-8
     * 
     * Para cada pixel encontrado, se pregunta si es nulo(Caso de borde), si no lo es
     * se le multiplica por su valor correspondiente en la matriz laplaciana y luego se
     * suma a la variable pixel. Si la bandera useSamePixel está activada se utilizará (p) en
     * los casos de borde. Y si no está activada esta no se toma en cuenta(o en otro modo "se multiplica por 0")
     * 
     * 
     */



    // PARA PIXELES 0 1 2


    //PIXEL 0: largo+1 pixeles atrás
    
    if(i-1 >= 0 && j-1 >= 0){
        pivot = p-(channels*(width+1));
        pixel = pixel + (*pivot)*lap_mask[0]; 
    }

    //PIXEL 1: largo pixeles atrás
    
    if(j-1 >= 0){
        pivot = p-(channels*(width));
        pixel = pixel + (*pivot)*lap_mask[1];
    }
    
    //PIXEL 2: largo - 1 pixeles atrás
    
    if(i+1 < width && j-1 >= 0){
        pivot = p-(channels*(width-1));
        pixel = pixel + (*pivot)*lap_mask[2];
    }


    //PIXEL 3: 1 pixel atras
    
    if(i-1 >= 0 ){
        pivot = p-(channels);
        pixel = pixel + (*pivot)*lap_mask[3];
    }
   
    //PIXEL 4: El mismo pixel
    
    
    pixel = pixel + (*p)*lap_mask[4];
    
    

    //PIXEL 5: 1 pixel adelante
    
    if(i+1 < width ){
        pivot = p+(channels);
        pixel = pixel + (*pivot)*lap_mask[5];
    }
    


    //PIXEL 6: largo-1 pixeles adelante
    
    if(i-1 >= 0  && j+1 < height){
        pivot = p+(channels*(width-1));
        pixel = pixel + (*pivot)*lap_mask[6];
    }
    

    //PIXEL 7: largo pixeles adelante
    
    if(j+1 < height){
        pivot = p+(channels*(width));
        pixel = pixel + (*pivot)*lap_mask[7];
    }
    
    //PIXEL 8: largo+1 pixeles adelante
    
    if(i+1 < width && j+1 < height){
        pivot = p+(channels*(width+1));
        pixel = pixel + (*pivot)*lap_mask[8];
    }



    return pixel;

}


uint8_t * rgb_to_grayscale(uint8_t * data,int width, int height, int *channels){
    // Inicializa las variables
    int img_size = width*height* *channels; //Tamaño de la imagen
    int gray_channels = *channels == 4 ? 2 : 1; // Nuevos canales
    int gray_img_size = width*height*gray_channels; //Tamaño de la imagen usando los nuevos canales
    uint8_t *gray_img = malloc(sizeof(uint8_t)*gray_img_size); //Asignar la memoria para la nueva imagen
    //Recorriendo la imagen
    //Idea extraida del video: https://solarianprogrammer.com/2019/06/10/c-programming-reading-writing-images-stb_image-libraries/
    for(uint8_t *p = data, *pg = gray_img; p != data + img_size; p+= *channels, pg+= gray_channels){
        *pg = (uint8_t)((*p)*0.3 + (*p + 1)*0.59 + (*p+2)*0.11);
    }
    free(data);
    *channels = gray_channels;
    return gray_img;
}



uint8_t * apply_lap_filter(uint8_t * data,int width, int height, int channels,int * lap_mask){
    int img_size = width*height*channels; //Tamaño de la imagen
    uint8_t *lap_img = malloc(sizeof(uint8_t)*img_size); //Asignar la memoria para la nueva imagen
    //Recorriendo la imagen
    uint8_t *p = data;
    uint8_t *pg = lap_img;
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            *pg = laplace(p,i,j,data,width,height,channels,lap_mask);
            p+= channels;
            pg+= channels;
        }
        
    }
    free(data);
    return lap_img;
}


uint8_t* apply_binary(uint8_t * data,int width, int height, int channels, int bin_threshold){
    
    int img_size = width*height*channels; //Tamaño de la imagen
    uint8_t *binary_img = malloc(sizeof(uint8_t)*img_size); //Asignar la memoria para la nueva imagen
    //Recorriendo la imagen
    //Idea extraida del video: https://solarianprogrammer.com/2019/06/10/c-programming-reading-writing-images-stb_image-libraries/
    for(uint8_t *p = data, *pg = binary_img; p != data + img_size; p+= channels, pg+= channels){
        //Si el valor del pixel es mayor al umbral asignarlo a 255 sino asignarlo a 0
        if(*p > bin_threshold){
            *pg = (uint8_t)255; 
        }
        else{
            *pg = (uint8_t)0;
        }
    }
    free(data);
    return binary_img;
}


int rate(Config * c,Image *img){
    int img_size = img->width*img->height*img->channels; //Tamaño de la imagen
    int black_count = 0; // Contador
    for(uint8_t *p = img->data; p != img->data + img_size; p+= img->channels){
        //Si es negro suma al contador
        if(*p == 0){
            black_count+=1; 
        }
    }
    // Si el porcentaje de pixeles negros es mayor o igual al umbral devuelve 1
    if((black_count*100.0/img->width*img->height) >= c->rating_threshold){
        return 1;
    }
    else{
        return 0;
    }

}

int count_black_pixels(uint8_t * data,int width,int height, int channels){
    int black_count = 0; // Contador
    int img_size = width*height*channels; //Tamaño de la imagen
    for(uint8_t *p = data; p != data + img_size; p+= channels){
        //Si es negro suma al contador
        if(*p == 0){
            black_count+=1; 
        }
    }
    return black_count;
}

