#ifndef PIPELINE_H
#define PIPELINE_H

void rgb_to_grayscale(Config * c ,Image *img);
void apply_lap_filter(Config * c,Image *img);
uint8_t laplace(uint8_t *p,int i,int j,Image*img,Config*c);
void apply_binary(Config * c,Image *img);
int rate(Config * c,Image *img);

#endif