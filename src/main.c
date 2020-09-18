#include<stdio.h>
#include<unistd.h>
#include <stdint.h>
#include<pthread.h>
#include "../include/image.h"
#include "../include/utils.h"
#include "../include/pipeline.h"

#define DEBUG 1 //DEJAR EN 1 PARA QUE NO SE SOBREESCRIBA LA IMAGEN
#define QUEUESIZE 10
typedef struct {
    int buf[QUEUESIZE];
    int head, tail;
    int full, empty;
    pthread_mutex_t mutex;
    pthread_cond_t notFull,notEmpty;
}buffer_t;





void *producer(void *arg){
    int v;
    buffer_t *buffer;
    buffer = (buffer_t *) arg;

    while(1){
        v = produce();
        pthread_mutex_lock(&buffer->mutex);
        while (buffer->full){
            pthread_cond_wait (&buffer->notFull, &buffer->mutex);
        }
        put_in_buffer(buffer, v);
        pthread_cond_signal(&buffer->notEmpty);
        pthread_mutex_unlock(&buffer->mutex);
    }

}

void *consumer(void *arg){
    int v;
    buffer_t *buffer;
    buffer = (buffer_t *) arg;
    while(1){
        pthread_mutex_lock(&buffer->mutex);
        while (buffer->empty) {
            pthread_cond_wait (&buffer->notEmpty, &buffer->mutex);
        }
        v = take_from_buffer(buffer);
        pthread_cond_signal(&buffer->notFull);
        pthread_mutex_unlock(&buffer->mutex);
    }
}









int main(int argc, char const *argv[])
{
    Image * img;
    Config * c = load_config(argc,argv); //Carga de configuracion
    int nb = 0;
    if(c != NULL){
        if(read_lap_mask(c)){
            char filename[30];
            int i = 0;
            if(c->show) {
                printf("|       image      |     nearly black    |\n|------------------|---------------------|\n");
            }
            for (int i = 0; i < c->images; i++)
            {
                sprintf(filename,"imagen_%d.jpg",i+1); //Genera el nombre de la imagen
                img = open_image(filename); //Abre la imagen
                if(img != NULL){ //Si la imagen se abrió correctamente corre el pipeline
                    rgb_to_grayscale(c,img);
                    apply_lap_filter(c,img);
                    apply_binary(c,img);
                    nb=rate(c,img);
                    //Se muestra el resultado por pantalla
                    if(c->show){
                        printf("|   %s   |         %s         |\n",filename,(nb ? "yes" : "no"));
                    }
                    //Se vuelve a guardar la imagen
                    if(DEBUG){
                        sprintf(filename,"imagen_%d_edit.jpg",i+1); //Si la bandera DEBUG está activada se le cambia el nombre a la imagen, de lo contrario se reemplaza.
                    }
                    write_image(filename,img);
                    free_image(img);
                }
                else{
                    printf("El archivo %s no se puede abrir o no existe!\n",filename);
                }
            }
        }
        else{
            printf("El archivo %s no se puede abrir o no existe!\n",c->lap_mask_file_name);
        }
    }
    else{
        printf("Los argumentos son inválidos o no están todos los necesarios!\n");
    }
    
    return 0;
}
