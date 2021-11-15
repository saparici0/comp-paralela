//gcc -std=c17 -Wall secuencial.c -o secuencial -lm
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
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

int main(int argc, char **argv){
    if(argc < 4){
        printf("Debe ingresar como primer argumento la ruta de la imagen, como segundo el numero de kernel (1-3) y como tercero la opcion de filtro (c (solo contraste en color), g (solo contraste en escala de grises) ó cg (ambas opciones)))");
        return 0;
    }
      
    int width, height, channels;
    int k = argv[2][0] - '0' -1;
    
    unsigned char *img = stbi_load(argv[1], &width, &height, &channels, 0);
    if(img == NULL) {
       printf("Error in loading the image\n");
       exit(1);
    }
    //printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

    struct timeval tval_before, tval_after;
    gettimeofday(&tval_before, NULL);

    const int k_size = k == 0 ? 9 : 3;
    
    int kernel[][9][9] = {{
        { 0, 0, 0,-1,-1,-1, 0, 0, 0},
        { 0,-2,-3,-3,-3,-3,-3,-2, 0},
        { 0,-3,-2,-1,-1,-1,-2,-3, 0},
        {-1,-3,-1, 9, 9, 9,-1,-3,-1},
        {-1,-3,-1, 9,19, 9,-1,-3,-1},
        {-1,-3,-1, 9, 9, 9,-1,-3,-1},
        { 0,-3,-2,-1,-1,-1,-2,-3, 0},
        { 0,-2,-3,-3,-3,-3,-3,-2, 0},
        { 0, 0, 0,-1,-1,-1, 0, 0, 0},
    },{ {-1,0,1},
        {-2,0,2},
        {-1,0,1}
    },{ {1,2,1},
        {0,0,0},
        {-1,-2-1}
    }};
    
    if(!strcmp(argv[3],"cg") || !strcmp(argv[3],"c")){
        int width_t = width - k_size;
        int height_t = height - k_size;
        size_t cont_img_size = width_t * height_t * channels;

        unsigned char *cont_mult_img = malloc(cont_img_size);
        if(cont_mult_img == NULL){ printf("Error al reservar memoria"); return(1);}

        int i=0;
        for (unsigned char *pg = img, *pc = cont_mult_img; i<height_t; i++, pg+=k_size*channels) {
            for (int j=0; j<width_t; j++, pg+=channels, pc+=channels) {
                *pc = (int8_t) kern_mat_mul_ch(pg, &kernel[k][0][0], width_t+k_size, k_size, channels);
                *(pc+1) = (int8_t) kern_mat_mul_ch((pg+1), &kernel[k][0][0], width_t+k_size, k_size, channels);
                *(pc+2) = (int8_t) kern_mat_mul_ch((pg+2), &kernel[k][0][0], width_t+k_size, k_size, channels);
                //printf("%u %u %u %i %i\n", (unsigned char) *pc, (unsigned char) *(pc+1), (unsigned char) *(pc+2), i, j);
            }
        }
        
        stbi_write_jpg("img/img_contraste_color.jpg", width, height, channels, cont_mult_img, 100);
        stbi_image_free(cont_mult_img);
    }
    
    if(!strcmp(argv[3],"cg") || !strcmp(argv[3],"g")){    
        size_t img_size = width * height * channels;
        int gray_channels = channels == 4 ? 2 : 1;
        size_t gray_img_size = width * height * gray_channels;
        unsigned char *gray_img = malloc(gray_img_size);
        if(gray_img == NULL){ printf("Error al reservar memoria"); return(1);}
        
        for(unsigned char *p = img, *pg = gray_img; p != img + img_size; p+= channels, pg += gray_channels) {
            *pg = (uint8_t)((*p + *(p+1)+ *(p+2) )/ 3.0);
            if(channels == 4) *(pg+1) = *(p+3);
        }
        stbi_write_jpg("img/img_gris.jpg", width, height, gray_channels, gray_img, 100);
        
        width -= k_size;
        height -= k_size;
        size_t cont_img_gray_size = width * height * gray_channels;

        unsigned char *cont_img = malloc(cont_img_gray_size);
        if(cont_img == NULL){ printf("Error al reservar memoria"); return(1);}

        int i=0;
        for (unsigned char *pg = gray_img, *pc = cont_img; i<height; i++, pg+=k_size*gray_channels) {
            for (int j=0; j<width; j++, pg+=gray_channels, pc+=gray_channels) {
                *pc = (int8_t) kern_mat_mul(pg, &kernel[k][0][0], width+k_size, k_size);
                //printf("%u %i %i\n", (unsigned char) *pc, i, j);
            }
        }
        stbi_write_jpg("img/img_contraste_gris.jpg", width, height, gray_channels, cont_img, 100);
        stbi_image_free(img);
        stbi_image_free(gray_img);
        stbi_image_free(cont_img);
    }
    gettimeofday(&tval_after, NULL);
    printf("Tiempo de procesamiento de %s con kernel=%s y opcion=%s :%f\n\n", argv[1],argv[2],argv[3], (tval_after.tv_sec + tval_after.tv_usec/1000000.0) - (tval_before.tv_sec + tval_before.tv_usec/1000000.0));
}

