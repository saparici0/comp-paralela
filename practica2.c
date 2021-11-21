// gcc -std=c17 -fopenmp -Wall practica2.c -o practica2 -lm
// ./practica2 ./img/720p.jpg 1 cg 3

#include "omp.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

// Función de multiplicacion matricial de forma convolucional entre una seccion de la matriz original y el kernel
unsigned char kern_mat_mul(unsigned char *A, int *K, int A_size, int K_size, int channels) {
    int sum = 0;
    for (int i=0; i<K_size; i++, A+=(A_size-K_size)*channels) { // Iteracion sobre la matriz
        for (int j=0; j<K_size; j++, A+=channels, K++) {
            sum += ((*A)*(*K));
        }
    }
    return (uint8_t) fmin(fmax(0,sum),255); // Retorno de la suma. Valor del nuevo pixel
}

int main(int argc, char **argv){
    if(argc < 4){ // Verficacion de los argumentos necesarios
        printf("Debe ingresar como primer argumento la ruta de la imagen, como segundo el numero de kernel (1-3) y como tercero la opcion de filtro (c (solo contraste en color), g (solo contraste en escala de grises) ó cg (ambas opciones)))");
        return 0;
    }
      
    int width, height, channels;
    int k = argv[2][0] - '0' - 1; // Kernel deseado

    if (k>3 || k<0) { // Verificación de k valido
        printf("Kernel invalido\n");
        return 0;
    }

    int thread_count = atoi(argv[4]);
    
    unsigned char *img = stbi_load(argv[1], &width, &height, &channels, 0); // Cargue de la imagen
    if(img == NULL) { //Verificacion de la imagen
       printf("Error cargando la imagen\n");
       exit(1);
    }
    printf("Image with width: %dpx, height: %dpx in %d channels\n", width, height, channels);

    struct timeval tval_before, tval_after; //Declaracion de variables de tiempo
    gettimeofday(&tval_before, NULL);

    const int k_size = k == 0 ? 9 : 3;
    
    int kernel[][9][9] = {{ // Kernels
        { 0, 0, 0,-1,-1,-1, 0, 0, 0},
        { 0,-2,-3,-3,-3,-3,-3,-2, 0},
        { 0,-3,-2,-1,-1,-1,-2,-3, 0},
        {-1,-3,-1, 9, 9, 9,-1,-3,-1},
        {-1,-3,-1, 9,19, 9,-1,-3,-1},
        {-1,-3,-1, 9, 9, 9,-1,-3,-1},
        { 0,-3,-2,-1,-1,-1,-2,-3, 0},
        { 0,-2,-3,-3,-3,-3,-3,-2, 0},
        { 0, 0, 0,-1,-1,-1, 0, 0, 0},
    },{ {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    },{ { 1, 2, 1},
        { 0, 0, 0},
        {-1,-2,-1}
    }};

    const int n_secs = 2;
    
    if(!strcmp(argv[3],"cg") || !strcmp(argv[3],"c")){ // Filtro convolucional sobel sobre canales multicolor
        int width_t = width - k_size; //Dimensiones de la nueva imagen
        int height_t = height - k_size;
        size_t cont_img_size = width_t * height_t * channels;

        unsigned char *cont_mult_img = malloc(cont_img_size); // Reserva de memoria para la nueva imagen
        if(cont_mult_img == NULL){ printf("Error al reservar memoria\n"); return(1);}
        
        int s_width = width_t/n_secs;

        #pragma omp parallel for num_threads(thread_count)
        for (int sec=0; sec<n_secs; sec++){
            int i=0; // Iteracion sobre la imagen
            for (unsigned char *p = img+sec*s_width*channels, *pc = cont_mult_img+sec*s_width*channels; i<height_t; i++, p+=(width-s_width)*channels, pc+=(width_t-s_width)*channels){ 
                for (int j=0; j<s_width; j++, p+=channels, pc+=channels) { // Multiplicacion convolucional de cada canal
                    *pc = (int8_t) kern_mat_mul(p, &kernel[k][0][0], width, k_size, channels);
                    *(pc+1) = (int8_t) kern_mat_mul((p+1), &kernel[k][0][0], width, k_size, channels);
                    *(pc+2) = (int8_t) kern_mat_mul((p+2), &kernel[k][0][0], width, k_size, channels);
                    //printf("%u %u %u %i %i\n", (unsigned char) *pc, (unsigned char) *(pc+1), (unsigned char) *(pc+2), i, j);
                }
            } 
        }
        
        stbi_write_jpg("img/img_contraste_color.jpg", width_t, height_t, channels, cont_mult_img, 100); // Guardado de la nueva imagen
        stbi_image_free(cont_mult_img);
    }
    
    if(!strcmp(argv[3],"cg") || !strcmp(argv[3],"g")) {

        // Conversión de la imagen a escala de grises  
        int gray_channels = channels == 4 ? 2 : 1;
        size_t gray_img_size = width * height * gray_channels; // Tamaño de la nueva imagen
        unsigned char *gray_img = malloc(gray_img_size);
        if(gray_img == NULL){ printf("Error al reservar memoria\n"); return(1);} // Reserva de memoria

        int s_width = width/n_secs;

        #pragma omp parallel for num_threads(thread_count)
        for (int sec=0; sec<n_secs; sec++){
            int i=0; // Iteracion sobre la imagen
            for (unsigned char *p = img+sec*s_width*channels, *pg = gray_img+sec*s_width*gray_channels; i<height; i++, p+=(width-s_width)*channels, pg+=(width-s_width)*gray_channels){ 
                for (int j=0; j<s_width; j++, p+=channels, pg+=gray_channels) {
                    *pg = (uint8_t)((*p + *(p+1)+ *(p+2) )/ 3.0); // Aproximacion de los valores de los 3 canales
                    if(channels == 4) *(pg+1) = *(p+3);
                }
            }
        }

        stbi_write_jpg("img/img_gris.jpg", width, height, gray_channels, gray_img, 100); //Guardado de imagen
        
        // Filtro sobel sobre la imagen en escala de grises (unico canal)

        int width_t = width - k_size; //Dimensiones de la nueva imagen
        int height_t = height - k_size;
        size_t cont_img_gray_size = width_t * height_t * gray_channels; // Tamaño de la nueva imagen

        unsigned char *cont_img = malloc(cont_img_gray_size); // Reserva de memoria
        if(cont_img == NULL){ printf("Error al reservar memoria\n"); return(1);}

        s_width = width_t/n_secs;

        #pragma omp parallel for num_threads(thread_count)
        for (int sec=0; sec<n_secs; sec++){
            int i=0; // Iteracion sobre la imagen
            for (unsigned char *pg = gray_img+sec*s_width*gray_channels, *pc = cont_img+sec*s_width*gray_channels; i<height_t; i++, pg+=(width-s_width)*gray_channels, pc+=(width_t-s_width)*gray_channels){ 
                for (int j=0; j<s_width; j++, pg+=gray_channels, pc+=gray_channels) { // Multiplicacion convolucional de cada canal
                    *pc = (int8_t) kern_mat_mul(pg, &kernel[k][0][0], width, k_size, gray_channels);
                    //printf("%u %i %i\n", (unsigned char) *pc, i, j);
                }
            } 
        }

        stbi_write_jpg("img/img_contraste_gris.jpg", width_t, height_t, gray_channels, cont_img, 100); // Guardado de imagen
        stbi_image_free(img);
        stbi_image_free(gray_img);
        stbi_image_free(cont_img);
    }

    gettimeofday(&tval_after, NULL); // Medicion de tiempo
    printf("Tiempo de procesamiento de %s con kernel=%s y opcion=%s :%f\n\n", argv[1],argv[2],argv[3], (tval_after.tv_sec + tval_after.tv_usec/1000000.0) - (tval_before.tv_sec + tval_before.tv_usec/1000000.0));
}

