#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "timers.h"

#define COUNT_MAX   2000
#define MIN(x,y)    ((x) < (y) ? (x) : (y))


typedef struct {
  unsigned int r;
  unsigned int g;
  unsigned int b;
} RgbColor;

RgbColor HSVtoRGB(unsigned h, unsigned s, unsigned v);


// Main part of the below code is originated from Lode Vandevenne's code.
// Please refer to http://lodev.org/cgtutor/juliamandelbrot.html
void julia(int w, int h) {
  // each iteration, it calculates: new = old*old + c,
  // where c is a constant and old starts at current pixel

  // real and imaginary part of the constant c
  // determinate shape of the Julia Set
  double cRe, cIm;

  // real and imaginary parts of new and old
  double newRe, newIm, oldRe, oldIm;

  // you can change these to zoom and change position
  double zoom = 1, moveX = 0, moveY = 0;

  // after how much iterations the function should stop
  int maxIterations = COUNT_MAX;

  char *output_filename = "julia.ppm";
  FILE *output_unit;
  double wtime;

  // pick some values for the constant c
  // this determines the shape of the Julia Set
  cRe = -0.7;
  cIm = 0.27015;

  int (*r)[h] = (int (*)[h])calloc(w * h, sizeof(int));
  int (*g)[h] = (int (*)[h])calloc(w * h, sizeof(int));
  int (*b)[h] = (int (*)[h])calloc(w * h, sizeof(int));

  printf( "  Sequential C version\n" );
  printf( "\n" );
  printf( "  Create an ASCII PPM image of the Julia set.\n" );
  printf( "\n" );
  printf( "  An ASCII PPM image of the set is created using\n" );
  printf( "    W = %d pixels in the X direction and\n", w );
  printf( "    H = %d pixels in the Y direction.\n", h );

  timer_init();
  timer_start(0);

  // loop through every pixel
  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
    {
      // calculate the initial real and imaginary part of z,
      // based on the pixel location and zoom and position values
      newRe = 1.5 * (x - w / 2) / (0.5 * zoom * w) + moveX;
      newIm = (y - h / 2) / (0.5 * zoom * h) + moveY;

      // i will represent the number of iterations
      int i;
      //start the iteration process
      for (i = 0; i < maxIterations; i++)
      {
        // remember value of previous iteration
        oldRe = newRe;
        oldIm = newIm;

        // the actual iteration, the real and imaginary part are calculated
        newRe = oldRe * oldRe - oldIm * oldIm + cRe;
        newIm = 2 * oldRe * oldIm + cIm;

        // if the point is outside the circle with radius 2: stop
        if ((newRe * newRe + newIm * newIm) > 4) break;
      }

      // use color model conversion to get rainbow palette, 
      // make brightness black if maxIterations reached
      RgbColor color = HSVtoRGB(i % 256, 255, 255 * (i < maxIterations));
      r[y][x] = color.r;
      g[y][x] = color.g;
      b[y][x] = color.b;     
    }
  }

  timer_stop(0);
  wtime = timer_read(0);
  printf( "\n" );
  printf( "  Time = %lf seconds.\n", wtime );

  // Write data to an ASCII PPM file.
  output_unit = fopen( output_filename, "wt" );

  fprintf( output_unit, "P3\n" );
  fprintf( output_unit, "%d  %d\n", h, w );
  fprintf( output_unit, "%d\n", 255 );
  for ( int i = 0; i < h; i++ )
  {
    for ( int jlo = 0; jlo < w; jlo = jlo + 4 )
    {
      int jhi = MIN( jlo + 4, w );
      for ( int j = jlo; j < jhi; j++ )
      {
        fprintf( output_unit, "  %d  %d  %d", r[i][j], g[i][j], b[i][j] );
      }
      fprintf( output_unit, "\n" );
    }
  }

  fclose( output_unit );
  printf( "\n" );
  printf( "  Graphics data written to \"%s\".\n\n", output_filename );

  // Terminate.
  free(r);
  free(g);
  free(b);
}


RgbColor HSVtoRGB(unsigned h, unsigned s, unsigned v)
{
  RgbColor rgb;
  unsigned char region, remainder, p, q, t;

  if (s == 0)
  {
    rgb.r = v;
    rgb.g = v;
    rgb.b = v;
    return rgb;
  }

  region = h / 43;
  remainder = (h - (region * 43)) * 6; 

  p = (v * (255 - s)) >> 8;
  q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

  switch (region)
  {
    case 0:
      rgb.r = v; rgb.g = t; rgb.b = p;
      break;
    case 1:
      rgb.r = q; rgb.g = v; rgb.b = p;
      break;
    case 2:
      rgb.r = p; rgb.g = v; rgb.b = t;
      break;
    case 3:
      rgb.r = p; rgb.g = q; rgb.b = v;
      break;
    case 4:
      rgb.r = t; rgb.g = p; rgb.b = v;
      break;
    default:
      rgb.r = v; rgb.g = p; rgb.b = q;
      break;
  }

  return rgb;
}

/*
 * OpenCL Boilerplate by Hyunseok Cho (i@hoyajigi.com)
 * First created at 2013.10.04
 * All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <math.h>
#include <CL/cl.h>
#include "cl_util.h"
#include "timers.h"

#define ERROR(err) fprintf(stderr, "[%s:%d] ERROR: %s\n",__FILE__,__LINE__,err);exit(EXIT_FAILURE);

int main()
{
	/* Host data structures */
	cl_platform_id   *platforms;
	cl_uint          num_platforms;
	cl_device_type   dev_type = CL_DEVICE_TYPE_DEFAULT;
	cl_device_id     dev;
	cl_context       context;
	// NOTE : You might have multiple cmd_queue but whatever
	cl_command_queue cmd_queue;
	cl_program       program;
	cl_kernel        kernel;
	// TODO : define your variables
//	cl_mem           ;
	cl_int           err;
	cl_uint          num_dev = 0;
	cl_event         ev_bp;
	
	// TODO : 
//	size_t lws[2]={64,4};
//	size_t gws[2]={1024,1024};


	int i;


	timer_init();

	srand(time(NULL));


	// Platform
	err = clGetPlatformIDs(0, NULL, &num_platforms);
	CHECK_ERROR(err);
	if(num_platforms == 0) {
		ERROR("No OpenCl platform");
	}
	printf("Number of platforms: %u\n",num_platforms);
	platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * num_platforms);
	err = clGetPlatformIDs(num_platforms,platforms,NULL);
	CHECK_ERROR(err);

	//Device
	for(i=0;i<num_platforms;i++) {
		// FIXME : something wrong
		err = clGetDeviceIDs(platforms[i],dev_type,1,&dev,&num_dev);
		if(err != CL_DEVICE_NOT_FOUND) CHECK_ERROR(err);
		if(num_dev == 1) break;
	}
	if(num_dev<1) {
		ERROR("No device");
	}
	
	// Print the device name.
	size_t name_size;
	clGetDeviceInfo(dev, CL_DEVICE_NAME, 0, NULL, &name_size);
	char *dev_name = (char *)malloc(name_size + 1);
	err = clGetDeviceInfo(dev,CL_DEVICE_NAME,name_size,dev_name,NULL);
	CHECK_ERROR(err);
	printf("Device: %s\n",dev_name);
	free(dev_name);

	// Context
	context = clCreateContext(NULL, 1, &dev, NULL, NULL, &err);
	CHECK_ERROR(err);

	// Command queue
	cmd_queue = clCreateCommandQueue(context, dev, 0, &err);
	CHECK_ERROR(err);

	// Create a program
	// TODO : Get source code in your favor
	char * source_code=get_source_code("hello.cl");
	
	
	size_t source_len=strlen(source_code);
	program = clCreateProgramWithSource(context, 1, (const char **)&source_code, &source_len, &err);
	CHECK_ERROR(err);

	// Callback data for clBuildProgram
	ev_bp=clCreateUserEvent(context,&err);
	CHECK_ERROR(err);
	bp_data_t bp_data;
	bp_data.dev=dev;
	bp_data.event=&ev_bp;

	// Build the program.
	err = clBuildProgram(program, 1, &dev, NULL, build_program_callback, &bp_data);
	if (err != CL_SUCCESS) {
		// Print the build log.
		size_t log_size;
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
				0, NULL, &log_size);
		char *log = (char *)malloc(log_size + 1);
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
				log_size, log, NULL);
		fprintf(stderr,"\n");
		fprintf(stderr,"---------- BUILD LOG ----------\n");
		fprintf(stderr,"%s\n",log);
		fprintf(stderr,"-------------------------------\n");
		free(log);

		CHECK_ERROR(err);
	}
	
	// Buffers
	// TODO: make and buffers
	/*
	clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
			sizeof(float) * N, A, &err);
	err=clEnqueueWriteBuffer(cmd_queue, m_array,CL_FALSE,0,size*sizeof(int),array,0,NULL,NULL);
	*/
	CHECK_ERROR(err);

	clWaitForEvents(1,bp_data.event);
	
	// Kernel
	kernel = clCreateKernel(program,"blank",&err);
	CHECK_ERROR(err);

	clFinish(cmd_queue);

	// TODO : Set the arguments.
	//err=clSetKernelArg(kernel,0,sizeof(),);

	// Enqueue the kernel.
	//err=clEnqueueNDRangeKernel(cmd_queue,kernel,1,NULL,gws,lws,0,NULL,NULL);
	CHECK_ERROR(err);

	// Read the result.
	/*
	err = clEnqueueReadBuffer(cmd_queue,
			mem_C,
			CL_TRUE, 0,
			sizeof(float) * N,
			C,
			0, NULL, NULL);
	*/
	CHECK_ERROR(err);

	// Release
	clReleaseEvent(ev_bp);
	//clReleaseMemObject();
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);
	free(platforms);

	return EXIT_SUCCESS;
}
