#ifndef PIPELINE_H
#define PIPELINE_H

uint8_t * rgb_to_grayscale(uint8_t * data,int width, int height, int *channels);
uint8_t* apply_binary(uint8_t * data,int width, int height, int channels, int bin_threshold);
int rate(Config * c,Image *img);


void apply_lap_filter(Config * c,Image *img);
uint8_t laplace(uint8_t *p,int i,int j,Image*img,Config*c);


#endif