#ifndef PIPELINE_H
#define PIPELINE_H

uint8_t * rgb_to_grayscale(uint8_t * data,int width, int height, int *channels);
uint8_t* apply_binary(uint8_t * data,int width, int height, int channels, int bin_threshold);
int rate(Config * c,Image *img);


void apply_lap_filter(Config * c,Image *img);
uint8_t laplace(uint8_t *p,int i,int j,Image*img,Config*c);


typedef struct {
    uint8_t *buf;
    size_t buff_size;
    int full, empty;
    pthread_mutex_t mutex;
    pthread_cond_t notFull,notEmpty;
}buffer_t;

typedef struct {
    buffer_t * buffer;
    int t_id;
    int height;
    int start;
}c_info;





void *consumer(void *arg);
void take_from_buffer(buffer_t * buffer, uint8_t * data,int readed);
void *producer(void *arg);
uint8_t * read_img(Image * img,size_t * start);
void put_in_buffer(buffer_t * buffer,uint8_t * data);
buffer_t * buffer_init(int buff_size);
c_info * c_info_init(buffer_t * buffer,int i);

#endif