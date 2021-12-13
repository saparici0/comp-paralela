#include "omp.h"
#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#define BLOCKSPERGRID 1

__global__ void kern_mat_mul(const unsigned char *A, const int *K, unsigned char *B, int A_size, int max_row, int K_size, int channels){
    
    __shared__ int kernel[9][9];

    for(int i=0; i<K_size*K_size; i++){
        kernel[i/K_size][i-(i/K_size)*K_size] = *(K + i);
    }

    int row = blockDim.x * blockIdx.x + threadIdx.x;

    int ai = row * A_size * channels;

    if(row <= max_row) {
        for(int c=0; c<(A_size-K_size)*channels; c++, ai++) {
            int sum = 0; 
            for (int i=0; i<K_size; i++, ai+=(A_size-K_size)*channels) { // Iteracion sobre la matriz
                for (int j=0; j<K_size; j++, ai+=channels) {
                    sum += ((*(A+ai))*(kernel[i][j]));
                }
            }
            
            if (sum > 255) sum = 255;
            else if (sum <0) sum = 0;
            *(B + row * (A_size-K_size) * channels + c) = (uint8_t) sum;
        }
    }
}

int main(int argc, char **argv){
    cudaError_t err = cudaSuccess;

    if(argc < 4){ // Verficacion de los argumentos necesarios
        printf("Debe ingresar la ruta de la imagen, el numero de kernel (1-3), la opcion de filtro (c (solo contraste en color), g (solo contraste en escala de grises) ó cg (ambas opciones))) y el número de hilos");
        return 0;
    }
      
    int width, height, channels;
    int k = argv[2][0] - '0' - 1; // Kernel deseado

    if (k>3 || k<0) { // Verificación de k valido
        printf("Kernel invalido\n");
        return 0;
    }

    int threads_per_block = atoi(argv[4]);
    
    unsigned char *img = stbi_load(argv[1], &width, &height, &channels, 0); // Cargue de la imagen
    if(img == NULL) { //Verificacion de la imagen
       printf("Error cargando la imagen\n");
       exit(1);
    }

    const int k_size = k == 0 ? 9 : 3;
    
    int kernels[][9][9] = {{ // Kernels
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

    struct timeval tval_before, tval_after; //Declaracion de variables de tiempo
    gettimeofday(&tval_before, NULL);
    
    int width_t = width - k_size; //Dimensiones de la nueva imagen
    int height_t = height - k_size;
    size_t cont_img_size = width_t * height_t * channels;

    //
    
    unsigned char *cont_mult_img = (unsigned char *) malloc(cont_img_size);// Reserva de memoria para la nueva imagen
    if(cont_mult_img == NULL){ printf("Error al reservar memoria img host\n"); return(1);}

    //

    int* h_k = (int*) malloc(k_size*k_size*sizeof(int));
    if(h_k == NULL){ printf("Error al reservar memoria kernel host\n"); return(1);}
    
    for(int i=0; i<k_size*k_size; i++){
        *(h_k+i) = kernels[k][i/k_size][i-(i/k_size)*k_size];
    }

    int* d_k = NULL;

    err = cudaMalloc((void**)&d_k, k_size*k_size*sizeof(int));
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to allocate device kernel matrix (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // 

    unsigned char *d_img = NULL;

    err = cudaMalloc((void**)&d_img, width*height*channels);
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to allocate device img matrix (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    //

    unsigned char *d_cont_img = NULL;

    err = cudaMalloc((void**)&d_cont_img, cont_img_size);
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to allocate device cont img matrix (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    //

    err = cudaMemcpy(d_k, h_k, k_size*k_size*sizeof(int), cudaMemcpyHostToDevice);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to copy kernel matrix to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    //

    err = cudaMemcpy(d_img, img, width*height*channels, cudaMemcpyHostToDevice);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to copy img matrix to device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    //

    int n_threads = height_t;
    int blocks_per_grid = n_threads/threads_per_block+1;

    printf("CUDA kernel launch with %d blocks of %d threads\n", blocks_per_grid, threads_per_block);
    kern_mat_mul<<<blocks_per_grid, threads_per_block>>>(d_img, d_k, d_cont_img, width, height, k_size, channels);
    
    err = cudaGetLastError();

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to launch kernel (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    //

    err = cudaMemcpy(cont_mult_img, d_cont_img, cont_img_size, cudaMemcpyDeviceToHost);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to copy vector C from device to host (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    //

    err = cudaFree(d_k);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to free kernel matrix in device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaFree(d_img);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to free img matrix in device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    } 

    err = cudaFree(d_cont_img);

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to free cont img matrix in device (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    } 

    stbi_write_jpg("img/img_contraste_color.jpg", width_t, height_t, channels, cont_mult_img, 100); // Guardado de la nueva imagen
    stbi_image_free(cont_mult_img);

    gettimeofday(&tval_after, NULL); // Medicion de tiempo
    printf("Tiempo de procesamiento de %s con kernel=%s opcion=%s y %d hilos por block: %f\n", argv[1],argv[2],argv[3], threads_per_block, (tval_after.tv_sec + tval_after.tv_usec/1000000.0) - (tval_before.tv_sec + tval_before.tv_usec/1000000.0));
}