#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <stdint.h>
#include<pthread.h>
#include "../include/image.h"
#include "../include/utils.h"
#include "../include/pipeline.h"

#define DEBUG 1 //DEJAR EN 1 PARA QUE NO SE SOBREESCRIBA LA IMAGEN
#define QUEUESIZE 10


Image * img;
int actual_line = 0;
int linesPerBuffer = 0;
int linesPerThread = 0;
int threads = 0;
int lines = 0;
pthread_barrier_t barrier;



c_info * c_info_init(buffer_t * buffer,int i){
    c_info * info = malloc(sizeof(c_info));
    info->buffer = buffer;
    info->t_id = i;
    info->height = 0;
    info->start = 0;
    return info;
}



buffer_t * buffer_init(int buff_size){
    buffer_t * buffer = malloc(sizeof(buffer_t));
    buffer->buff_size = buff_size;
    buffer->empty = buff_size;
    buffer->full = 0;
    return buffer;
}
void allocate_buffer(buffer_t * buffer,int width,int channels){
    buffer->buf = malloc(sizeof(uint8_t)*(buffer->buff_size*width*channels));
}



void put_in_buffer(buffer_t * buffer,uint8_t * data){
    memcpy(buffer->buf+(buffer->buff_size-buffer->empty),data,sizeof(uint8_t)*(img->width*img->channels));
    buffer->empty--;
    if(buffer->empty == 0){
        buffer->full = 1;
    }
}

void take_from_buffer(buffer_t * buffer, uint8_t * data,int readed){
    memcpy(data+(img->width*img->channels*readed),buffer->buf+(buffer->buff_size-buffer->empty),sizeof(uint8_t)*(img->width*img->channels));
    memmove(buffer->buf,buffer->buf+sizeof(uint8_t)*(img->width*img->channels),sizeof(uint8_t)*(buffer->buff_size-buffer->empty)-1);
    buffer->empty++;
    if(buffer->empty == buffer->buff_size){
        buffer->full = 0;
    }
}

uint8_t * read_img(Image * img,size_t * start){
    uint8_t * data = img->data+*start;
    *start = *start + sizeof(uint8_t)*img->width*img->channels;
    return data;
}
void *producer(void *arg){
    printf("Soy la hebra productora\n");
    uint8_t * data;
    buffer_t *buffer;
    buffer = (buffer_t *) arg;
    size_t start = 0;

    while(actual_line < img->height){
        data = read_img(img,&start); // Lee una linea
        pthread_mutex_lock(&buffer->mutex);
        while (buffer->full){
            pthread_cond_wait (&buffer->notFull, &buffer->mutex);
        }
        put_in_buffer(buffer, data);
        actual_line = actual_line + linesPerBuffer;
        pthread_cond_signal(&buffer->notEmpty);
        pthread_mutex_unlock(&buffer->mutex);

    }
    
    //pthread_barrier_wait(&barrier);
    //printf("Terminó la hebra productora\n");
    return NULL;
}



void *consumer(void *arg){
    c_info * info;
    info = (c_info*) arg;

    info->height = lines/threads;
    info->start = info->height * info->t_id;
    if(info->t_id == threads-1){
        info->height = info->height + lines%threads;
    }

    buffer_t *buffer;
    uint8_t * data = malloc(sizeof(uint8_t)*info->height*img->width*img->channels);
    uint8_t * pointer = data;
    buffer = info->buffer;
    int readed = 0;
    while(readed < info->height){
        if(info->start <= actual_line && actual_line < info->start+info->height){
            printf("Linea actual: %d Hebra %d: lee %d filas\n",actual_line,info->t_id,info->height);
            pthread_mutex_lock(&buffer->mutex);
            while (buffer->empty) {
                pthread_cond_wait (&buffer->notEmpty, &buffer->mutex);
            }
            take_from_buffer(buffer,pointer,readed);
            readed = readed + linesPerBuffer;
            pthread_cond_signal(&buffer->notFull);
            pthread_mutex_unlock(&buffer->mutex);
        }
           
        
    }
    pthread_barrier_wait(&barrier);
    printf("Termino hebra %d\n",info->t_id);
    return NULL;
}







int main(int argc, char const *argv[])
{
    printf("INICIANDO\n");
    Config * c = load_config(argc,argv); //Carga de configuracion
    int lap_mask[] = {0,1,2,3,4,5,6,7,8};
    memcpy(c->lap_mask,lap_mask,9*sizeof(int));
    printf("MASCARA LEIDA\b");
    buffer_t *buffer;
    pthread_t pro; //Hebra productora
    pthread_t con[c->threads]; //Hebras consumidoras
    threads = c->threads;
    img = open_image("imagen_1.jpg"); //Abro la imagen
    printf("Imagen abierta\n");

    linesPerBuffer = c->buff_size;
    buffer = buffer_init(c->buff_size); //inicializando el buffer


    allocate_buffer(buffer,img->width,img->channels);//Depende de la imagen.
    
    linesPerThread = img->height/threads; //Cantidad de filas por hebra, depende de la imagen
    lines = img->height;

    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->notFull, NULL);
    pthread_cond_init(&buffer->notEmpty, NULL);

    pthread_barrier_init(&barrier,NULL,c->threads-1);

    pthread_create(&pro, NULL, producer, (void *) buffer);

    printf("Buffer->full: %d Buffer->empty: %d\n",buffer->full,buffer->empty);
    for (int i = 0; i < c->threads; i++)
    {
        c_info * info = c_info_init(buffer,i);
        pthread_create(&con[i], NULL, consumer, (void *) info);
    }
    printf("Hebras creadas, ahora a esperar\n");
    pthread_join(pro,NULL);
    for (size_t i = 0; i < c->threads; i++)
    {
        pthread_join(con[i],NULL);
    }
    
    pthread_barrier_destroy(&barrier);
    
    return 0;

}
