//mpicc -std=c17 -fopenmp practica4.c -o practica4 -lm
//mpirun -np 4 practica4 ./img/1080p.jpg 1 cg 1

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "omp.h"
#include <mpi.h>
#include <math.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#define MSG_LENGTH 15

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

unsigned char* multicolor_filter(unsigned char *img, int width, int height, int channels, int k, int k_size, int thread_count){
	int width_t = width - k_size; //Dimensiones de la nueva imagen
	int height_t = height - k_size;
	size_t cont_img_size = width_t * height_t * channels;
	const int n_secs = thread_count*4;

	unsigned char *cont_mult_img = malloc(cont_img_size); // Reserva de memoria para la nueva imagen
	if(cont_mult_img == NULL){ printf("Error al reservar memoria en multicolor\n");}
	
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
	return cont_mult_img;
}

unsigned char* gray_filter(unsigned char *img, int width, int height, int channels, int k, int k_size, int thread_count){
	// Conversión de la imagen a escala de grises  
	int gray_channels = channels == 4 ? 2 : 1;
	size_t gray_img_size = width * height * gray_channels; // Tamaño de la nueva imagen
	unsigned char *gray_img = malloc(gray_img_size);
	if(gray_img == NULL){ printf("Error al reservar memoria\n");} // Reserva de memoria
	const int n_secs = thread_count*4;
	int s_width = width/n_secs;

	#pragma omp parallel for num_threads(thread_count)
	for (int sec=0; sec<n_secs; sec++){
		int i=0; // Iteracion sobre la imagen
		for (unsigned char *p = img+sec*s_width*gray_channels, *pg = gray_img+sec*s_width*gray_channels; i<height; i++, p+=(width-s_width)*channels, pg+=(width-s_width)*gray_channels){ 
			for (int j=0; j<s_width; j++, p+=channels, pg+=gray_channels) {
				*pg = (uint8_t)((*p + *(p+1)+ *(p+2) )/ 3.0); // Aproximacion de los valores de los 3 canales
				if(channels == 4) *(pg+1) = *(p+3);
			}
		}
	}

	// Filtro sobel sobre la imagen en escala de grises (unico canal)
	int width_t = width - k_size; //Dimensiones de la nueva imagen
	int height_t = height - k_size;
	size_t cont_img_gray_size = width_t * height_t * gray_channels; // Tamaño de la nueva imagen

	unsigned char *cont_img = malloc(cont_img_gray_size); // Reserva de memoria
	if(cont_img == NULL){ printf("Error al reservar memoria\n");}

	s_width = width_t/n_secs;

	#pragma omp parallel for num_threads(thread_count)
	for (int sec=0; sec<n_secs; sec++){
		// Iteracion sobre la imagen
		int i=0;
		for (unsigned char *pg = gray_img+sec*s_width*gray_channels, *pc = cont_img+sec*s_width*gray_channels; i<height_t; i++, pg+=(width-s_width)*gray_channels, pc+=(width_t-s_width)*gray_channels){ 
			for (int j=0; j<s_width; j++, pg+=gray_channels, pc+=gray_channels) { // Multiplicacion convolucional de cada canal
				*pc = (int8_t) kern_mat_mul(pg, &kernel[k][0][0], width, k_size, gray_channels);
				//printf("%u %i %i\n", (unsigned char) *pc, i, j);
			}
		} 
	}
	return cont_img;
}

int main (int argc, char *argv[])
{
	if(argc < 4){ // Verficacion de los argumentos necesarios
        printf("Debe ingresar la ruta de la imagen, el numero de kernel (1-3), la opcion de filtro (c (solo contraste en color), g (solo contraste en escala de grises) ó cg (ambas opciones))) y el número de hilos");
        return 0;
    }

	int i, tag=1, tasks, iam, p;
	int w_corte;
	int h_corte;
	int width, height, channels;
	MPI_Status status;
	int n_variables = 4, pedacito_size;
	int message[n_variables];

	int k = argv[2][0] - '0' - 1; // Kernel deseado
	if (k>3 || k<0) { // Verificación de k valido
        printf("Kernel invalido\n");
        return 0;
    }
	int thread_count = atoi(argv[4]);
	/*
	printf("argc: %d\n", argc);
	printf("argv[0]: %s\n", argv[0]);
	printf("argv[1]: %s\n", argv[1]);
	printf("argv[2]: %s\n", argv[2]);
	printf("argv[3]: %s\n", argv[3]);
	printf("argv[4]: %s\n", argv[4]);
	*/
	const int k_size = k == 0 ? 9 : 3;
    

    const int n_secs = thread_count*4;

	bool multicolor = !strcmp(argv[3],"cg") || !strcmp(argv[3],"c");
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &tasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &iam);
	if (iam == 0) {
		unsigned char *img = stbi_load(argv[1], &width, &height, &channels, 0); // Cargue de la imagen
		if(img == NULL) { //Verificacion de la imagen
			printf("Error cargando la imagen\n");
			exit(1);
		}

		int width_t = width - k_size; //Dimensiones de la nueva imagen
        int height_t = height - k_size;

		//Imagen multicolor
        size_t cont_img_size = width_t * height_t * channels;
		unsigned char *multicolor_img = malloc(cont_img_size); // Reserva de memoria
		if(multicolor_img == NULL){ printf("Error al reservar memoria\n");};

		//Imagen grises
		int gray_channels = channels == 4 ? 2 : 1;
        size_t cont_img_gray_size = width_t * height_t * gray_channels; // Tamaño de la nueva imagen
		unsigned char *gray_img = malloc(cont_img_gray_size); // Reserva de memoria
		if(gray_img == NULL){ printf("Error al reservar memoria\n");}

		int pedacito_height = height/tasks;
		int n_pixeles_pedacito = pedacito_height*width;
		pedacito_size = n_pixeles_pedacito*channels;
		unsigned char *pedacito = malloc(pedacito_size); // Reserva de memoria
		if(pedacito == NULL){ printf("Error al reservar memoria\n");}

		int n_pedacito_procesado = (width-k_size)*(pedacito_height-k_size);
		int pedacito_procesado_multicolor_size = n_pedacito_procesado * channels;
		unsigned char *pedacito_multicolor_procesado = malloc(pedacito_procesado_multicolor_size); // Reserva de memoria
		if(pedacito_multicolor_procesado == NULL){ printf("Error al reservar memoria\n");}

		int pedacito_procesado_gray_size = n_pedacito_procesado * gray_channels;
		unsigned char *pedacito_gray_procesado = malloc(pedacito_procesado_gray_size); // Reserva de memoria
		if(pedacito_gray_procesado == NULL){ printf("Error al reservar memoria\n");}
		message[0] = height;
		message[1] = width;
		message[2] = channels;
		message[3] = pedacito_size;
		for (int p = 0; p < tasks; p++){
			int pixel_inicial_original = p*(n_pixeles_pedacito);
			int pixel_inicial_resultado = p*(n_pedacito_procesado);
			for(int pixel=0; pixel<n_pixeles_pedacito; pixel++){
				for(int canal=0; canal < channels; canal++){
					pedacito[pixel * channels + canal] = img[(pixel_inicial_original+pixel)*channels + canal];
				}
			}
			if(p == 0){
				if(multicolor){ // Filtro convolucional sobel sobre canales multicolor
					pedacito_multicolor_procesado = multicolor_filter(pedacito, width, pedacito_height, channels, k, k_size, thread_count);
					for(int pixel=0; pixel<n_pedacito_procesado; pixel++){//Poner pedacito procesado en imagen final
						for(int canal=0; canal < channels; canal++){
							multicolor_img[(pixel + pixel_inicial_resultado) * channels + canal] = pedacito_multicolor_procesado[pixel*channels + canal];
						}
					}
				}
			}else{
				MPI_Ssend(message, n_variables, MPI_INT, p, tag, MPI_COMM_WORLD);
				MPI_Ssend(pedacito,pedacito_size, MPI_UNSIGNED_CHAR, p, tag, MPI_COMM_WORLD);
				if(multicolor){
					MPI_Recv(pedacito_multicolor_procesado, pedacito_procesado_multicolor_size, MPI_UNSIGNED_CHAR,p, tag, MPI_COMM_WORLD, &status);
					for(int pixel=0; pixel<n_pedacito_procesado; pixel++){//Poner pedacito procesado en imagen final
						for(int canal=0; canal < channels; canal++){
							multicolor_img[(pixel + pixel_inicial_resultado) * channels + canal] = pedacito_multicolor_procesado[pixel*channels + canal];
						}
					}
				}
			}
				
		}
		if(multicolor){
			stbi_write_jpg("img/img_contraste_color.jpg", width_t, height_t, channels, multicolor_img, 100); // Guardado de la nueva imagen
		}
		stbi_image_free(multicolor_img);
		stbi_image_free(gray_img);
		stbi_image_free(pedacito_multicolor_procesado);
		stbi_image_free(pedacito_gray_procesado);
		stbi_image_free(pedacito);
		stbi_image_free(img);
		/*
		*/
	} else{
		MPI_Recv(message, n_variables, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		height = message[0];
		width = message[1];
		channels = message[2];
		pedacito_size = message[3];
		int gray_channels = channels == 4 ? 2 : 1;
		int pedacito_height = height/tasks;
		int n_pixeles_pedacito = pedacito_height*width;
		pedacito_size = n_pixeles_pedacito*channels;
		unsigned char *pedacito = malloc(pedacito_size); // Reserva de memoria
		if(pedacito == NULL){ printf("Error al reservar memoria\n");}

		int n_pedacito_procesado = (width-k_size)*(pedacito_height-k_size);
		int pedacito_procesado_multicolor_size = n_pedacito_procesado * channels;
		unsigned char *pedacito_multicolor_procesado = malloc(pedacito_procesado_multicolor_size); // Reserva de memoria
		if(pedacito_multicolor_procesado == NULL){ printf("Error al reservar memoria\n");}

		int pedacito_procesado_gray_size = n_pedacito_procesado * gray_channels;
		unsigned char *pedacito_gray_procesado = malloc(pedacito_procesado_gray_size); // Reserva de memoria
		if(pedacito_gray_procesado == NULL){ printf("Error al reservar memoria\n");}
		
		MPI_Recv(pedacito, pedacito_size, MPI_UNSIGNED_CHAR, 0, tag, MPI_COMM_WORLD, &status);

		if(multicolor){
			pedacito_multicolor_procesado = multicolor_filter(pedacito, width, pedacito_height, channels, k, k_size, thread_count);
			MPI_Ssend(pedacito_multicolor_procesado, pedacito_procesado_multicolor_size, MPI_UNSIGNED_CHAR, 0, tag, MPI_COMM_WORLD);
		}
		stbi_image_free(pedacito_multicolor_procesado);
		stbi_image_free(pedacito_gray_procesado);
		stbi_image_free(pedacito);
		/*
		*/
	}
	//printf("node %d width: %d\n", iam, width);
	//printf("node %d height: %d\n", iam, height);
	//printf("node %d channels: %d\n", iam, channels);
	//printf("node %d pedacito_size: %d\n", iam, pedacito_size);
	MPI_Finalize();
	return 0;
}

