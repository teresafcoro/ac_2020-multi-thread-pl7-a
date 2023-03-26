/*
 * Main.cpp
 *
 * Created on: Fall 2019
 * 
 * Teamwork: 2020-multi-thread-pl7-a
 */

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <CImg.h>
#include <time.h>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>

// Using the classes/funtions from cimg_library
using namespace cimg_library;

// Data type for image components
typedef float data_t;

// Pointers to the source image and the destination image
const char* SOURCE_IMG 		= "/home/student/Pictures/normal/uniovi_2.bmp";
const char* DESTINATION_IMG = "/home/student/Pictures/uniovi_2_final.bmp";

// Number of threads
#define NUM_HILOS  4

// Function declaration
void *photoConversor(void *v);

// Number os repetitions for the loop of the elapsed time
int nREPS = 30;

// Structure of piels to process
typedef struct {
	int pixel_init;
	int pixel_fin;
} thread_structure;

typedef struct {
	// Creating the variables for the image components
	uint width, height; // Width and height of the image
	data_t *pRsrc, *pGsrc, *pBsrc; // Pointers to the R, G and B components
	data_t *pLdest; // Pointer to the L component
	data_t L;	// Luminosity of the image
} photoStructure;

photoStructure photo;

// Principal method of the class
int main() {

	// Try to load the source image
	// If it is not possible, catch the exception
	cimg::exception_mode(0);
	try {	

		// Open file and object initialization
		CImg<data_t> srcImage(SOURCE_IMG);

		// Creating the variables for the image components
		data_t *pDstImage; // Pointer to the new image pixels
		uint nComp; // Number of image components

		// Variables initialization
		srcImage.display(); // Displays the source image
		photo.width  = srcImage.width(); // Getting information from the source image
		photo.height = srcImage.height();
		nComp  = srcImage.spectrum();
					
		// Allocate memory space for destination image components
		pDstImage = (data_t *) malloc (photo.width * photo.height * nComp * sizeof(data_t));
		if (pDstImage == NULL) {
			perror("Allocating destination image");
			exit(-2);
		}

		// Pointers to the componet arrays of the source image
		photo.pRsrc = srcImage.data(); 							// pRcomp points to the R component array
		photo.pGsrc = photo.pRsrc + photo.height * photo.width; // pGcomp points to the G component array
		photo.pBsrc = photo.pGsrc + photo.height * photo.width; // pBcomp points to the B component array
		photo.pLdest = pDstImage;								// pLdest points to the L component array
		
		// Create the variables for the time measurement
		struct timespec tStart, tEnd;
		double dElapsedTimeS = 0;

		// Start time
		if (clock_gettime(CLOCK_REALTIME, &tStart) == -1) {
			printf("ERROR: clock_gettime: %d, \n", errno);
			exit(EXIT_FAILURE);
		}

		// Using nComp=1 for B/W images
		nComp = 1;

		// Arrays of threads
		pthread_t threads[NUM_HILOS];
		thread_structure thread[NUM_HILOS];

		// Size of the image threads
		int tamImageThread = photo.width * photo.height / NUM_HILOS;

		// Time measurement and threads execution
		// Making a loop so we obtain significant results
		// We considerate the differences between last thread and the others
		for (int j = 0; j < nREPS; j++) {
			for (int h = 0; h < NUM_HILOS; h++) {
				thread[h].pixel_init = tamImageThread * h;
				thread[h].pixel_fin = thread[h].pixel_init + tamImageThread;

				if (h == NUM_HILOS-1)
					thread[h].pixel_fin = photo.width * photo.height;

				int pthread_ret = pthread_create(
					&threads[h],
					NULL,
					photoConversor,
					&(thread[h])
				);

				if (pthread_ret) {
					printf("ERROR: phtread_create error code: %d \n", pthread_ret);
					exit(EXIT_FAILURE);
				}

				pthread_join(threads[h], NULL);
			}
		}

		// End time
		if (clock_gettime(CLOCK_REALTIME, &tEnd) == -1) {
			printf("ERROR: clock_gettime: %d, \n", errno);
			exit(EXIT_FAILURE);
		}
		
		// Calculate the elapsed time in the execution of the algorithm
		dElapsedTimeS = (tEnd.tv_sec - tStart.tv_sec);
		dElapsedTimeS += (tEnd.tv_nsec - tStart.tv_nsec) / 1e+9;

		// Create a new image object with the calculated pixels
		CImg<data_t> dstImage(pDstImage, photo.width, photo.height, 1, nComp);
		dstImage.save(DESTINATION_IMG);

		// Display destination image
		dstImage.display();

		// Showing the elapsed time
		printf("Elapsed time	: %f s.\n", dElapsedTimeS);

	} catch (CImgException& e) {
		std::fprintf(stderr, "Error while loading the source image: %s", e.what());
	}

	return 0;
}

// Conversor for the photo in every single thread
void *photoConversor(void *v) {
	thread_structure *thread = (thread_structure*)v;

	// Initial and final pixels
	int pixel_init = thread->pixel_init;
	int pixel_fin = thread->pixel_fin;
	
	for (long i = pixel_init; i < pixel_fin; i++) {
		photo.L = 0.3 * photo.pRsrc[i] + 0.59 * photo.pGsrc[i] + 0.11 * photo.pBsrc[i];
		photo.L = 255 - photo.L;
		photo.pLdest[i] = photo.L;
	}
	
	return NULL;
}
