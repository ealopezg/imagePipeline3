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
Image * output_image;
int actual_line = -1;
int threads = 0;
int lines = 0;
int bin_threshold = 0;
int rating_threshold = 0;
int lap_mask[] = {0,1,0,1,-4,1,0,1,0};
int g_black_pixels = 0;
int g_new_channels = 1;

int g_start = 0;
int g_threads_writed = 0;
int show = 0;
pthread_barrier_t barrier1;
pthread_barrier_t barrier2;
pthread_barrier_t barrier3;
pthread_barrier_t barrier4;
pthread_barrier_t barrier5;

pthread_barrier_t barrier0;
pthread_barrier_t barrier_wait_all;

c_info * c_info_init(buffer_t * buffer,int i){
    c_info * info = malloc(sizeof(c_info));
    info->buffer = buffer;
    info->t_id = i;
    info->height = 0;
    info->start = 0;
    return info;
}



buffer_t * buffer_init(int buff_size,int images){
    buffer_t * buffer = malloc(sizeof(buffer_t));
    buffer->images = images;
    buffer->buff_size = buff_size;
    return buffer;
}

void reset_buffer(buffer_t * buffer,int width,int channels){
    buffer->empty = buffer->buff_size;
    buffer->lastLoaded = -1;
    buffer->lastReaded = -1;
    buffer->full = 0;

    buffer->buf = malloc(sizeof(uint8_t)*(buffer->buff_size*width*channels));
}



void put_in_buffer(buffer_t * buffer,uint8_t * data){
    //printf("Full %d Empty %d\n",buffer->full,buffer->empty);
    memcpy(buffer->buf+(buffer->buff_size-buffer->empty)*(img->width*img->channels),data,sizeof(uint8_t)*(img->width*img->channels));
    //printf("FILA INGRESADA AL BUFFER\n");
    buffer->empty--;
    buffer->lastLoaded++;
    if(buffer->empty == 0){
        buffer->full = 1;
    }
    //printf("Full %d Empty %d\n",buffer->full,buffer->empty);
     
}

void take_from_buffer(buffer_t * buffer, uint8_t * data,int readed){
    //printf("Full %d Empty %d\n",buffer->full,buffer->empty);
    memcpy(data+(img->width*img->channels*readed),  buffer->buf  ,sizeof(uint8_t)*(img->width*img->channels));
    memmove(buffer->buf,buffer->buf+(img->width*img->channels),sizeof(uint8_t)*(img->width*img->channels)*(buffer->buff_size-buffer->empty-1));
    //printf("FILA SACADA DEL BUFFER\n");
    buffer->empty++;
    buffer->lastReaded++;
    buffer->full = 0;
    //printf("Full %d Empty %d\n",buffer->full,buffer->empty);
}

uint8_t * read_img(Image * img,size_t * start){
    uint8_t * data = img->data+*start;
    *start = *start + img->width*img->channels;
    return data;
}





void *producer(void *arg){
    //printf("Soy la hebra productora\n");
    uint8_t * data;
    buffer_t *buffer;
    buffer = (buffer_t *) arg;
    char filename[30];
    if(show) {
        printf("|       image      |     nearly black    |\n|------------------|---------------------|\n");
    }
    for (size_t i = 0; i < buffer->images; i++)
    {
        pthread_mutex_lock(&buffer->mutex);
        actual_line = -1;
        sprintf(filename,"imagen_%d.jpg",i+1);
        img = open_image(filename); //Abro la imagen
        reset_buffer(buffer,img->width,img->channels);
        
        size_t start = 0;
        lines = img->height;
        g_start = 1;
        pthread_mutex_unlock(&buffer->mutex);
        pthread_cond_broadcast(&buffer->canStart); //Enviamos la señal

        while(actual_line < img->height-1){
            //printf("Actual_line %d alto imagen %d\n",actual_line,img->height);
            data = read_img(img,&start); // Lee una linea
            
            pthread_mutex_lock(&buffer->mutex);
            while (buffer->full){
                pthread_cond_wait (&buffer->notFull, &buffer->mutex);
            }
            put_in_buffer(buffer, data);
            actual_line = actual_line + 1;
            pthread_cond_signal(&buffer->notEmpty);
            pthread_mutex_unlock(&buffer->mutex);

        }
        
        //printf("Terminó de cargar la hebra productora %d\n",actual_line);
        output_image = empty_image(img->width,img->height,g_new_channels); // Asignación de memoria
        g_start = 0;
        pthread_barrier_wait(&barrier_wait_all);
        g_black_pixels = 0;
        g_threads_writed = 0;
        free_image(img);
        free_image(output_image);
        free(buffer->buf);
        pthread_barrier_destroy(&barrier0);
        pthread_barrier_destroy(&barrier1);
        pthread_barrier_destroy(&barrier2);
        pthread_barrier_destroy(&barrier3);
        pthread_barrier_destroy(&barrier4);
        pthread_barrier_destroy(&barrier5);
        pthread_barrier_destroy(&barrier_wait_all);









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



void *consumer(void *arg){
    
    c_info * info;
    info = (c_info*) arg;
    buffer_t *buffer;
    buffer = info->buffer;

    for (size_t i = 0; i < buffer->images; i++)
    {
        pthread_mutex_lock(&buffer->mutex);
        while(g_start == 0){
            pthread_cond_wait (&buffer->canStart, &buffer->mutex); // Esperamos la señal
        }
        pthread_mutex_unlock(&buffer->mutex);



        info->height = lines/threads;
        //printf("%d\n",lines);
        info->start = info->height * info->t_id;
        if(info->t_id == threads-1){
            info->height = info->height + lines%threads;
        }

        
        uint8_t * data = malloc(sizeof(uint8_t)*info->height*img->width*img->channels);
        
        int readed = 0;
        //printf("HEBRA %d TIENE QUE LEER DE %d hasta %d\n",info->t_id,info->start,info->start+info->height-1);
        while(readed < info->height){
            if(info->start <= buffer->lastReaded+1 && buffer->lastReaded+1 <= info->start+info->height-1){
                pthread_mutex_lock(&buffer->mutex);
                //printf("Hebra %d: Lee la fila %d, de %d / %d filas, hay %d/%d espacios libres en el buffer\n",info->t_id,buffer->lastLoaded,readed+1,info->height,buffer->empty,buffer->buff_size);
                while (buffer->lastLoaded != img->height-1 && buffer->empty) {
                    pthread_cond_wait (&buffer->notEmpty, &buffer->mutex);
                }
                take_from_buffer(buffer,data,readed);
                readed = readed + 1;
                pthread_cond_signal(&buffer->notFull);
                //printf("\n\n\n"); 
                pthread_mutex_unlock(&buffer->mutex);
            }

                
        }
        //printf("Termino de leer hebra %d\n",info->t_id);
        pthread_barrier_wait(&barrier0);
        //printf("Hebra %d: Iniciando edición\n",info->t_id);
        int newChannels = img->channels;
        data = rgb_to_grayscale(data,img->width,info->height,&newChannels);
        //printf("Hebra %d: Imagen guardada en escala de grises\n",info->t_id);
        pthread_barrier_wait(&barrier1);
        data = apply_lap_filter(data,img->width,info->height,newChannels,lap_mask);
        //printf("Hebra %d: Filtro laplaciano aplicado\n",info->t_id);
        pthread_barrier_wait(&barrier2);
        data = apply_binary(data,img->width,info->height,newChannels,bin_threshold);
        //printf("Hebra %d: Binarización aplicada\n",info->t_id);
        pthread_barrier_wait(&barrier3);
        write_lines_to_image(output_image,data,info->start,info->height);//Guardar a imagen global, no es necesaria la exclusión mutua
        pthread_barrier_wait(&barrier4);
        int black_pixels = count_black_pixels(data,img->width,info->height,newChannels);
        

        pthread_mutex_lock(&buffer->mutex);
        g_black_pixels = g_black_pixels + black_pixels;
        g_threads_writed++;
        //printf("Hebra %d: %d %d\n",info->t_id,g_black_pixels,img->width*img->height*newChannels);
        if(g_threads_writed == threads){
            char filename[30];
            sprintf(filename,"imagen_%d.jpg",i+1); 
            //printf("Soy la ultima hebra!\n");
            if(show) {
                if( g_black_pixels*100.0/img->width*img->height >= rating_threshold){
                    printf("|   %s   |         yes         |\n",filename);
                }
                else{
                    printf("|   %s   |         no          |\n",filename);
                }
                
            }
            sprintf(filename,"out_%d.jpg",i+1);
            write_image(filename,output_image);
        }
        pthread_mutex_unlock(&buffer->mutex);
        
        pthread_barrier_wait(&barrier5);

        pthread_barrier_wait(&barrier_wait_all);
        
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
    pthread_barrier_init(&barrier_wait_all,NULL,threads+1);

    
    
    pthread_create(&pro, NULL, producer, (void *) buffer);

    //printf("Buffer->full: %d Buffer->empty: %d\n",buffer->full,buffer->empty);
    for (int i = 0; i < c->threads; i++)
    {
        c_info * info = c_info_init(buffer,i);
        pthread_create(&con[i], NULL, consumer, (void *) info);
    }
    //printf("Hebras creadas, ahora a esperar\n");
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
