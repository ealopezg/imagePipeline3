#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <stdint.h>
#include<pthread.h>
#include "../include/image.h"
#include "../include/utils.h"
#include "../include/pipeline.h"


//Variables globales
Image * img; //Imagen leida
Image * output_image; //Imagen de salida
int actual_line = -1; //Fila actual leida
int threads = 0; //Cantidad de hebras
int lines = 0;  //Cantidad de filas de la imagen actual
int bin_threshold = 0; //Parametro para binarizacion
int rating_threshold = 0; //Parametro de clasificación
int lap_mask[] = {0,1,0,1,-4,1,0,1,0}; //Máscara para el filtro laplaciano
int g_black_pixels = 0; //Variable global
int g_new_channels = 1;

int g_start = 0;
int g_threads_writed = 0;
int show = 0;
pthread_barrier_t barrier0;
pthread_barrier_t barrier1;
pthread_barrier_t barrier2;
pthread_barrier_t barrier3;
pthread_barrier_t barrier4;
pthread_barrier_t barrier5;
pthread_barrier_t barrier_wait_all;


/**
 * @brief Inicializa los parámetros de la hebra consumidora
 * 
 * @param buffer 
 * @param i 
 * @return c_info* 
 */
c_info * c_info_init(buffer_t * buffer,int i){
    c_info * info = malloc(sizeof(c_info));
    info->buffer = buffer;
    info->t_id = i;
    info->height = 0;
    info->start = 0;
    return info;
}


/**
 * @brief Inicializa la estructura buffer, no guarda
 * memoria para el buffer en sí, ya que el tamaño de este
 * en bytes depende de la imagen
 * 
 * @param buff_size 
 * @param images 
 * @return buffer_t* 
 */
buffer_t * buffer_init(int buff_size,int images){
    buffer_t * buffer = malloc(sizeof(buffer_t));
    buffer->images = images;
    buffer->buff_size = buff_size;
    return buffer;
}

/**
 * @brief Resetea el valor del buffer, usado para reutilizar
 * la estructura
 * 
 * @param buffer 
 * @param width 
 * @param channels 
 */
void reset_buffer(buffer_t * buffer,int width,int channels){
    buffer->empty = buffer->buff_size;
    buffer->lastLoaded = -1;
    buffer->lastReaded = -1;
    buffer->full = 0;

    buffer->buf = malloc(sizeof(uint8_t)*(buffer->buff_size*width*channels));
}


/**
 * @brief Ingresa una fila completa al buffer
 * 
 * @param buffer Buffer
 * @param data Datos a guardar
 */
void put_in_buffer(buffer_t * buffer,uint8_t * data){
    
    memcpy(buffer->buf+(buffer->buff_size-buffer->empty)*(img->width*img->channels),data,sizeof(uint8_t)*(img->width*img->channels));
    buffer->empty--;
    buffer->lastLoaded++;
    if(buffer->empty == 0){
        buffer->full = 1;
    }
     
}


/**
 * @brief Remueve una fila completa del buffer (en la posición 0)
 * 
 * @param buffer 
 * @param data Puntero para guardar la información
 * @param readed Entero que indica la posición a guardar en el arreglo
 * de pixeles de la hebra ( 0<= readed <= m)
 */
void take_from_buffer(buffer_t * buffer, uint8_t * data,int readed){
    memcpy(data+(img->width*img->channels*readed),  buffer->buf  ,sizeof(uint8_t)*(img->width*img->channels));
    memmove(buffer->buf,buffer->buf+(img->width*img->channels),sizeof(uint8_t)*(img->width*img->channels)*(buffer->buff_size-buffer->empty-1));
    buffer->empty++;
    buffer->lastReaded++;
    buffer->full = 0;
}


/**
 * @brief Produce una fila completa de la imagen
 * 
 * @param img Imagen
 * @param start Puntero para llevar el seguimiento
 * @return uint8_t* Fila producida
 */
uint8_t * read_img(Image * img,size_t * start){
    uint8_t * data = img->data+*start;
    *start = *start + img->width*img->channels;
    return data;
}




/**
 * @brief Función productor
 * 
 * @param arg 
 * @return void* 
 */
void *producer(void *arg){
    //Parametros de la hebra productora
    uint8_t * data;
    buffer_t *buffer;
    buffer = (buffer_t *) arg;
    char filename[30];
    if(show) {
        printf("|       image      |     nearly black    |\n|------------------|---------------------|\n");
    }
    for (size_t i = 0; i < buffer->images; i++)
    {
        //Inicio sección critica inicializar parametros de hebra productora
        pthread_mutex_lock(&buffer->mutex);
        actual_line = -1;     // No se ha leido ninguna linea todavía
        sprintf(filename,"imagen_%ld.jpg",i+1);
        img = open_image(filename); //Abro la imagen
        reset_buffer(buffer,img->width,img->channels); //Asignamos valores al buffer, ya que el tamaño en bytes del buffer depende de cada imagen
        size_t start = 0; //Posición de la imagen a leer
        lines = img->height; //Cantidad de filas de la imagen
        g_start = 1; //Valor para que las hebras consumidoras puedan comenzar
        pthread_mutex_unlock(&buffer->mutex);
        pthread_cond_broadcast(&buffer->canStart); //Enviamos la señal a las hebras consumidoras

        //Mientras no se hayan leido las filas
        while(actual_line < img->height-1){
            data = read_img(img,&start); // Lee una linea de la imagen
            
            pthread_mutex_lock(&buffer->mutex);
            while (buffer->full){
                pthread_cond_wait (&buffer->notFull, &buffer->mutex);
            }
            put_in_buffer(buffer, data); // Guardar en el buffer la fila producida
            actual_line = actual_line + 1; //Aumentamos el contador
            pthread_cond_signal(&buffer->notEmpty); //Damos la señal de que el buffer no está vacío
            pthread_mutex_unlock(&buffer->mutex);

        }
        
        output_image = empty_image(img->width,img->height,g_new_channels); // Asignación de memoria de la imagen de salida
        g_start = 0; //Reiniciando las variables
        pthread_barrier_wait(&barrier_wait_all); //Esperamos a que todas las hebras terminen
        g_black_pixels = 0; //Reiniciando las variables
        g_threads_writed = 0;
        free_image(img); //Liberando memoria
        free_image(output_image);
        free(buffer->buf);
        //Destruyendo las barreras (No sabía como resetearlas sin tener que destruirlas)
        pthread_barrier_destroy(&barrier0);
        pthread_barrier_destroy(&barrier1);
        pthread_barrier_destroy(&barrier2);
        pthread_barrier_destroy(&barrier3);
        pthread_barrier_destroy(&barrier4);
        pthread_barrier_destroy(&barrier5);
        pthread_barrier_destroy(&barrier_wait_all);

        //Creando las barreras
        pthread_barrier_init(&barrier0,NULL,threads);
        pthread_barrier_init(&barrier1,NULL,threads);
        pthread_barrier_init(&barrier2,NULL,threads);
        pthread_barrier_init(&barrier3,NULL,threads);
        pthread_barrier_init(&barrier4,NULL,threads);
        pthread_barrier_init(&barrier5,NULL,threads);
        pthread_barrier_init(&barrier_wait_all,NULL,threads+1);

    }
    



    

    return NULL;
}


/**
 * @brief Función consumidor
 * 
 * @param arg 
 * @return void* 
 */
void *consumer(void *arg){
    
    //Obteniendo los argumentos de la hebra consumidora
    c_info * info;
    info = (c_info*) arg;
    buffer_t *buffer;
    buffer = info->buffer;

    for (size_t i = 0; i < buffer->images; i++)
    {
        pthread_mutex_lock(&buffer->mutex);
        while(g_start == 0){
            pthread_cond_wait (&buffer->canStart, &buffer->mutex); // Esperamos la señal de la hebra productora para comenzar
        }
        pthread_mutex_unlock(&buffer->mutex);

        //Asignación de parámetros de cada hebra

        info->height = lines/threads;
        info->start = info->height * info->t_id;
        if(info->t_id == threads-1){
            info->height = info->height + lines%threads; //La última se lleva el resto
        }

        //En data se guardará la imagen
        uint8_t * data = malloc(sizeof(uint8_t)*info->height*img->width*img->channels);
        
        int readed = 0; //Contador de filas leidas
        while(readed < info->height){
            if(info->start <= buffer->lastReaded+1 && buffer->lastReaded+1 <= info->start+info->height-1){
                pthread_mutex_lock(&buffer->mutex);
                while (buffer->lastLoaded != img->height-1 && buffer->empty) {
                    pthread_cond_wait (&buffer->notEmpty, &buffer->mutex);
                }
                take_from_buffer(buffer,data,readed);
                readed = readed + 1;
                pthread_cond_signal(&buffer->notFull);
                pthread_mutex_unlock(&buffer->mutex);
            }

                
        }
        //Se terminó de leer
        pthread_barrier_wait(&barrier0);
        int newChannels = img->channels; //Al cambiar la imagen a Escala de grises los canales cambian
        data = rgb_to_grayscale(data,img->width,info->height,&newChannels); // Cambio a escala de grises
        pthread_barrier_wait(&barrier1);
        data = apply_lap_filter(data,img->width,info->height,newChannels,lap_mask); // Filtro laplaciano
        pthread_barrier_wait(&barrier2);
        data = apply_binary(data,img->width,info->height,newChannels,bin_threshold); //Binarización
        pthread_barrier_wait(&barrier3);
        write_lines_to_image(output_image,data,info->start,info->height);//Guardar a imagen global, no es necesaria la exclusión mutua
        pthread_barrier_wait(&barrier4);
        int black_pixels = count_black_pixels(data,img->width,info->height,newChannels); //Contar los pixeles negros
        

        pthread_mutex_lock(&buffer->mutex);
        g_black_pixels = g_black_pixels + black_pixels; // Seccion critica, aumentar en el acumulador la cantidad de pixeles negros
        g_threads_writed++;
        if(g_threads_writed == threads){ //Si es la ultima en escribir, tendrá que mostrar el resultado en pantalla y guardar la imagen
            char filename[30];
            sprintf(filename,"imagen_%ld.jpg",i+1); 
            if(show) {
                if( g_black_pixels*100.0/img->width*img->height >= rating_threshold){
                    printf("|   %s   |         yes         |\n",filename);
                }
                else{
                    printf("|   %s   |         no          |\n",filename);
                }
                
            }
            sprintf(filename,"out_%ld.jpg",i+1);
            write_image(filename,output_image);
        }
        pthread_mutex_unlock(&buffer->mutex);
        
        pthread_barrier_wait(&barrier5); //Esperamos a que todas las hebras consumidoras terminen

        pthread_barrier_wait(&barrier_wait_all); //Se espera que todas incluyendo la productora termine
        
    }
    return NULL;
    

    
}







int main(int argc, char const *argv[])
{
    Config * c = load_config(argc,argv); //Carga de configuracion
    
    memcpy(c->lap_mask,lap_mask,9*sizeof(int));

    buffer_t *buffer;
    pthread_t pro; //Hebra productora
    pthread_t con[c->threads]; //Hebras consumidoras

    // Asignación de variables globales
    threads = c->threads;
    show = c->show;
    bin_threshold = c->bin_threshold;
    rating_threshold = c->rating_threshold;
    

    buffer = buffer_init(c->buff_size,c->images); //inicializando el buffer

    

    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->notFull, NULL);
    pthread_cond_init(&buffer->notEmpty, NULL);
    pthread_cond_init(&buffer->canStart, NULL);

    pthread_barrier_init(&barrier0,NULL,threads);
    pthread_barrier_init(&barrier1,NULL,threads);
    pthread_barrier_init(&barrier2,NULL,threads);
    pthread_barrier_init(&barrier3,NULL,threads);
    pthread_barrier_init(&barrier4,NULL,threads);
    pthread_barrier_init(&barrier5,NULL,threads);
    pthread_barrier_init(&barrier_wait_all,NULL,threads+1); //Barrera que espera a todas las hebras incluyendo a la productora

    
    
    pthread_create(&pro, NULL, producer, (void *) buffer); //Creando la hebra productora

    
    for (int i = 0; i < c->threads; i++)
    {
        c_info * info = c_info_init(buffer,i);
        pthread_create(&con[i], NULL, consumer, (void *) info);
    }
    
    pthread_join(pro,NULL);
    for (size_t i = 0; i < c->threads; i++)
    {
        pthread_join(con[i],NULL);
    }
    pthread_barrier_destroy(&barrier0);
    pthread_barrier_destroy(&barrier1);
    pthread_barrier_destroy(&barrier2);
    pthread_barrier_destroy(&barrier3);
    pthread_barrier_destroy(&barrier4);
    pthread_barrier_destroy(&barrier5);
    pthread_barrier_destroy(&barrier_wait_all);
    
    return 0;

}
