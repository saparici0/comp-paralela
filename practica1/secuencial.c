//gcc -std=c17 -Wall secuencial.c -o secuencial -lm
#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

int main(){
    int width, height, channels;
    unsigned char *img = stbi_load("img/img.png", &width, &height, &channels, 0);
    if(img == NULL) {
       printf("Error in loading the image\n");
       exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

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
    
    size_t img_size = width * height * channels;
    int gray_channels = channels == 4 ? 2 : 1;
    size_t gray_img_size = width * height * gray_channels;

    unsigned char *gray_img = malloc(gray_img_size);
    if(gray_img == NULL){ printf("Error al reservar memoria");return(1);}

    for(unsigned char *p = img, *pg = gray_img; p != img + img_size; p+= channels, pg += gray_channels){
        *pg = (uint8_t)((*p + *(p+1)+ *(p+2) )/ 3.0);
        if(channels == 4) *(pg+1) = *(p+3);
    }

    //Guardar imagen jpg
    stbi_write_jpg("img/img_gris.png", width, height, gray_channels, gray_img, 100);
    stbi_image_free(img);
    free(gray_img);
}