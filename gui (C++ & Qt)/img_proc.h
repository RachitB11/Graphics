#ifndef IMG_PROC_H
#define IMG_PROC_H

#endif // IMG_PROC_H
#include "raster_tools.h"

unsigned char *bubble_sort(unsigned char *arr, int len);
double rotx(int row,int col,double th, double c_x,double c_y);
double roty(int row,int col,double th, double c_x,double c_y);
img_t *grayscale(img_t *img,int size);
img_t *flip(img_t *img,int size);
img_t *flop(img_t *img,int size);
img_t *transpose(img_t *img,int size);
img_t *boxblur(img_t *img,int size, int n);
img_t *median(img_t *img,int size, int n);
img_t *gaussian(img_t *img,int size, int n, float s);
img_t *rotate(img_t *img,float th);
img_t *sobel(img_t *img,int size);
img_t *resize(img_t *img,int w_new,int h_new);

img_t *process_image(img_t *img, int *applyProc, int win_size, float ang, float sig);
