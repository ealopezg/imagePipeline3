#ifndef PIPELINE_H
#define PIPELINE_H

uint8_t * rgb_to_grayscale(uint8_t * data,int width, int height, int *channels);
uint8_t* apply_binary(uint8_t * data,int width, int height, int channels, int bin_threshold);
int rate(Config * c,Image *img);


uint8_t * apply_lap_filter(uint8_t * data,int width, int height, int channels,int * lap_mask);
uint8_t laplace(uint8_t * p,int i,int j,uint8_t * data,int width,int height,int channels,int * lap_mask);

/**
 * @brief Estructura del buffer
 * 
 */
typedef struct {
    uint8_t *buf; //Buffer de pixeles
    int images; // Cantidad de imagenes a leer
    size_t buff_size; // Tamaño de filas que tendrá el buffer (NO DE PIXELES)
    int lastReaded,lastLoaded; // Donde se guarda la ultima fila leida por una hebra y la ultima fila cargada al buffer por la hebra productora
    int full, empty; // Full = 0 ó 1 Empty = 0 a buff_size
    pthread_mutex_t mutex;
    pthread_cond_t notFull,notEmpty,canStart;
}buffer_t;
/**
 * @brief Estructura de la hebra consumidora
 * 
 */
typedef struct {
    buffer_t * buffer;
    int t_id; // ID
    int height; //Altura asignada (m)
    int start; //Fila inicial asignada
}c_info;





void *consumer(void *arg);
void take_from_buffer(buffer_t * buffer, uint8_t * data,int readed);
void *producer(void *arg);
uint8_t * read_img(Image * img,size_t * start);
void put_in_buffer(buffer_t * buffer,uint8_t * data);
buffer_t * buffer_init(int buff_size,int images);
c_info * c_info_init(buffer_t * buffer,int i);
void reset_buffer(buffer_t * buffer,int width,int channels);
int count_black_pixels(uint8_t * data,int width,int height, int channels);
#endif