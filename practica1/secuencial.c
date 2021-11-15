//gcc -std=c17 -Wall secuencial.c -o secuencial -lm
#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

unsigned char kern_mat_mul(unsigned char *A, int *K, int A_size, int K_size) {
    int sum = 0;
    for (int i=0; i<K_size; i++, A+=A_size-K_size) {
        for (int j=0; j<K_size; j++, A++, K++) {
            sum += ((*A)*(*K));
        }
    }
    return (uint8_t) fmin(fmax(0,sum),255);
}

unsigned char kern_mat_mul_ch(unsigned char *A, int *K, int A_size, int K_size, int channels) {
    int sum = 0;
    for (int i=0; i<K_size; i++, A+=(A_size-K_size)*channels) {
        for (int j=0; j<K_size; j++, A+=channels, K++) {
            sum += ((*A)*(*K));
        }
    }
    return (uint8_t) fmin(fmax(0,sum),255);
}

int main(){
    int width, height, channels;
    unsigned char *img = stbi_load("img/templos.jpg", &width, &height, &channels, 0);
    if(img == NULL) {
       printf("Error in loading the image\n");
       exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

    const int k_size = 9;
    
    int kernel[9][9] = {
        { 0, 0, 0,-1,-1,-1, 0, 0, 0},
        { 0,-2,-3,-3,-3,-3,-3,-2, 0},
        { 0,-3,-2,-1,-1,-1,-2,-3, 0},
        {-1,-3,-1, 9, 9, 9,-1,-3,-1},
        {-1,-3,-1, 9,19, 9,-1,-3,-1},
        {-1,-3,-1, 9, 9, 9,-1,-3,-1},
        { 0,-3,-2,-1,-1,-1,-2,-3, 0},
        { 0,-2,-3,-3,-3,-3,-3,-2, 0},
        { 0, 0, 0,-1,-1,-1, 0, 0, 0},
    };
    
    width -= k_size;
    height -= k_size;
    size_t cont_img_size = width * height * channels;

    unsigned char *cont_mult_img = malloc(cont_img_size);
    if(cont_mult_img == NULL){ printf("Error al reservar memoria"); return(1);}

    int i=0;
    for (unsigned char *pg = img, *pc = cont_mult_img; i<height; i++, pg+=k_size*channels) {
        for (int j=0; j<width; j++, pg+=channels, pc+=channels) {
            *pc = (int8_t) kern_mat_mul_ch(pg, &kernel[0][0], width+k_size, k_size, channels);
            *(pc+1) = (int8_t) kern_mat_mul_ch((pg+1), &kernel[0][0], width+k_size, k_size, channels);
            *(pc+2) = (int8_t) kern_mat_mul_ch((pg+2), &kernel[0][0], width+k_size, k_size, channels);
            //printf("%u %u %u %i %i\n", (unsigned char) *pc, (unsigned char) *(pc+1), (unsigned char) *(pc+2), i, j);
        }
    }

    stbi_write_jpg("img/img_contraste_mult.jpg", width, height, channels, cont_mult_img, 100);

    width += k_size;
    height += k_size;
    
    size_t img_size = width * height * channels;
    int gray_channels = channels == 4 ? 2 : 1;
    size_t gray_img_size = width * height * gray_channels;

    unsigned char *gray_img = malloc(gray_img_size);
    if(gray_img == NULL){ printf("Error al reservar memoria"); return(1);}

    for(unsigned char *p = img, *pg = gray_img; p != img + img_size; p+= channels, pg += gray_channels) {
        *pg = (uint8_t)((*p + *(p+1)+ *(p+2) )/ 3.0);
        if(channels == 4) *(pg+1) = *(p+3);
    }

    //Guardar imagen jpg
    stbi_write_jpg("img/img_gris.jpg", width, height, gray_channels, gray_img, 100);
    stbi_image_free(img);

    width -= k_size;
    height -= k_size;
    cont_img_size = width * height * gray_channels;

    unsigned char *cont_img = malloc(cont_img_size);
    if(cont_img == NULL){ printf("Error al reservar memoria"); return(1);}

    i=0;
    for (unsigned char *pg = gray_img, *pc = cont_img; i<height; i++, pg+=k_size*gray_channels) {
        for (int j=0; j<width; j++, pg+=gray_channels, pc+=gray_channels) {
            *pc = (int8_t) kern_mat_mul(pg, &kernel[0][0], width+k_size, k_size);
            //printf("%u %i %i\n", (unsigned char) *pc, i, j);
        }
    }

    stbi_write_jpg("img/img_contraste.jpg", width, height, gray_channels, cont_img, 100);
    stbi_image_free(gray_img);

    free(cont_img);
}

