/*
fractal.c - Sample Mandelbrot Fractal Display
Starting code for CSE 30341 Project 3.
*/

#include "gfx.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <complex.h>
#include <pthread.h>

 pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*
Compute the number of iterations at point x, y
in the complex space, up to a maximum of maxiter.
Return the number of iterations at that point.

This example computes the Mandelbrot fractal:
z = z^2 + alpha

Where z is initially zero, and alpha is the location x + iy
in the complex plane.  Note that we are using the "complex"
numeric type in C, which has the special functions cabs()
and cpow() to compute the absolute values and powers of
complex values.
*/
void *compute_image(void *);

//Thread argument structure
struct thread_args{
    int sH;
    int eH;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double maxiter;
};


void createThreads(int numT, double xmin, double xmax, double ymin, double ymax, double maxiter){
    
    //Chunk variables
    int sH = 0;
    int eH = 0;

    //Step and pixel counter
    int pixelCounter = 0;
    int step = gfx_ysize()/numT;
    int num = 0;

    //Array of threads
    pthread_t threads[numT];
    int i = 0;

    struct thread_args *args;
    //Create the threads and divide the pixel count
    for(int pixCount = 0; pixCount+step<=gfx_ysize(); pixCount += step){
        sH = pixCount;
        eH = pixCount + step;

        //Creat the arguments for the current thread
        args = malloc(sizeof(struct thread_args));
        args->ymin = ymin;
        args->ymax = ymax;
        args->xmin = xmin;
        args->xmax = xmax;
        args->maxiter = maxiter;
        args->sH = sH;
        args->eH = eH;

        //Create the threads and compute function
        pthread_create(&threads[i], NULL, compute_image, args);
        i++;
    }

    //Join the threads
    for(int i = 0; i < numT; i++){
        pthread_join(threads[i], NULL);
    }
    

    return;

}


static int compute_point( double x, double y, int max )
{
	double complex z = 0;
	double complex alpha = x + I*y;

	int iter = 0;

	while( cabs(z)<4 && iter < max ) {
		z = cpow(z,2) + alpha;
		iter++;
	}

	return iter;
}

/*
Compute an entire image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax).
*/

void *compute_image(void *myArgs)
{
	int i,j;
    int width = gfx_xsize();
    int height = gfx_ysize();

	// For every pixel i,j, in the image...
    struct thread_args *args = (struct thread_args*)myArgs;
    int sH = args->sH;
    int eH = args->eH;
    double xmin = args->xmin;
    double xmax = args->xmax;
    double ymin = args->ymin;
    double ymax = args->ymax;
    double maxiter = args->maxiter;

	for(j=sH;j<eH;j++) {
		for(i=0;i<width;i++) {

			// Scale from pixels i,j to coordinates x,y
			double x = xmin + i*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

            //Critical section begins
            pthread_mutex_lock(&lock);
			// Compute the iterations at x,y
			int iter = compute_point(x,y,maxiter);

			// Convert a iteration number to an RGB color.
			// (Change this bit to get more interesting colors.)
			int gray = 255 * iter / maxiter;
            gfx_color(gray,gray,gray);
			gfx_point(i,j);
            pthread_mutex_unlock(&lock);
		}
	}
 
    return NULL;
}

int main( int argc, char *argv[] )
{
	// The initial boundaries of the fractal image in x,y space.
	double xmin=-1.5;
	double xmax= 0.5;
	double ymin=-1.0;
	double ymax= 1.0;

 

	// Maximum number of iterations to compute.
	// Higher values take longer but have more detail.
	int maxiter=500;
    int threadCount = 1;

	// Open a new window.
	gfx_open(640,480,"Mandelbrot Fractal");

	// Show the configuration, just in case you want to recreate it.
	printf("coordinates: %lf %lf %lf %lf\n",xmin,xmax,ymin,ymax);

	// Fill it with a dark blue initially
	gfx_clear_color(0,0,255);
    gfx_clear();
   

	// Display the fractal image
    double vert = 0.1;
    double hor = 0.1;
	while(1) {
		// Wait for a key or mouse click.
        int c = gfx_wait();
        gfx_clear();
 
        //Determine the thread count
        if(c == '1') threadCount = 1;
        else if(c == '2') threadCount = 2;
        else if(c == '3') threadCount = 3;
        else if(c == '4') threadCount = 4;
        else if(c == '5') threadCount = 5;
        else if(c == '6') threadCount = 6;
        else if(c == '7') threadCount = 7;
        else if(c == '8') threadCount = 8;

        //Determine movement information
        switch(c){
            case '=':       //Zoom in
                xmin+=hor;
                xmax-=hor;
                ymin+=vert;
                ymax-=vert;
                break;
            case '-':       //Zoom out
                xmin-=hor;
                xmax+=hor;
                ymin-=vert;
                ymax+=vert;
                break;
            case 'w':       //Move up
                ymin+=vert;
                ymax+=vert;
                break;
            case 'a':       //Move left
                xmin+=hor;
                xmax+=hor;
                break;
            case 's':       //Move down
                ymin-=vert;
                ymax-=vert;
                break;
            case 'd':       //Move right
                xmin-=hor;
                xmax-=hor;
                break;
            case 'z':       //Decrease iter
                if((maxiter-50) < 0) break;
                maxiter-=50;
                break;
            case 'x':       //Increase iter
                maxiter+=50;
                break;
            case 'q':       //Quit
                exit(0);
                break;
            default:
                break;

        }

        //Create the image
        createThreads(threadCount, xmin, xmax, ymin, ymax, maxiter);
        //Let the people know we made it
        printf("Computed\n");
	}

	return 0;
}
