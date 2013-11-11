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

#define COUNT_MAX   5000
#define MIN(x,y)    ((x) < (y) ? (x) : (y))


void mandelbrot(int m, int n) {
  int count_max = COUNT_MAX;
  int i, j, jhi, jlo, k;
  char *output_filename = "mandelbrot.ppm";
  FILE *output_unit;
  double wtime;

  float x_max =   1.25;
  float x_min = - 2.25;
  float y_max =   1.75;
  float y_min = - 1.75;
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
	cl_mem           R,G,B;
	cl_int           err;
	cl_uint          num_dev = 0;
	cl_event         ev_bp;
	
	// TODO : 
	size_t lws[2]={16,16};
	size_t gws[2]={m/16,n/16};


  int (*r) = (int (*))calloc(m * n, sizeof(int));
  int (*g) = (int (*))calloc(m * n, sizeof(int));
  int (*b) = (int (*))calloc(m * n, sizeof(int));

  printf( "  Parallel OpenCL version\n" );
  printf( "\n" );
  printf( "  Create an ASCII PPM image of the Mandelbrot set.\n" );
  printf( "\n" );
  printf( "  For each point C = X + i*Y\n" );
  printf( "  with X range [%g,%g]\n", x_min, x_max );
  printf( "  and  Y range [%g,%g]\n", y_min, y_max );
  printf( "  carry out %d iterations of the map\n", count_max );
  printf( "  Z(n+1) = Z(n)^2 + C.\n" );
  printf( "  If the iterates stay bounded (norm less than 2)\n" );
  printf( "  then C is taken to be a member of the set.\n" );
  printf( "\n" );
  printf( "  An ASCII PPM image of the set is created using\n" );
  printf( "    M = %d pixels in the X direction and\n", m );
  printf( "    N = %d pixels in the Y direction.\n", n );

  timer_init();
  timer_start(0);

	// Platform
	err = clGetPlatformIDs(0, NULL, &num_platforms);
	CHECK_ERROR(err);
	if(num_platforms == 0) {
		ERROR("No OpenCl platform");
	}
//	printf("Number of platforms: %u\n",num_platforms);
	platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * num_platforms);
	err = clGetPlatformIDs(num_platforms,platforms,NULL);
	CHECK_ERROR(err);

	//Device
	for(i=0;i<num_platforms;i++) {
		err = clGetDeviceIDs(platforms[i],dev_type,1,&dev,&num_dev);
		if(err != CL_DEVICE_NOT_FOUND) CHECK_ERROR(err);
		if(num_dev == 1) break;
	}
	if(num_dev<1) {
		ERROR("No device");
	}
	
	// Context
	context = clCreateContext(NULL, 1, &dev, NULL, NULL, &err);
	CHECK_ERROR(err);

	// Command queue
	cmd_queue = clCreateCommandQueue(context, dev, 0, &err);
	CHECK_ERROR(err);

	// Create a program
	// TODO : Get source code in your favor
	char * source_code=get_source_code("mandelbrot.cl");
	
	
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

  R=clCreateBuffer(context, CL_MEM_READ_WRITE,
      sizeof(int) * w*h, NULL, &err);
  CHECK_ERROR(err);
  err=clEnqueueWriteBuffer(cmd_queue, R,CL_FALSE,0,m*n*sizeof(int),r,0,NULL,NULL);
  CHECK_ERROR(err);
  G=clCreateBuffer(context, CL_MEM_READ_WRITE,
      sizeof(int) * w*h, NULL, &err);
  CHECK_ERROR(err);
  err=clEnqueueWriteBuffer(cmd_queue, G,CL_FALSE,0,m*n*sizeof(int),g,0,NULL,NULL);
  CHECK_ERROR(err);
  B=clCreateBuffer(context, CL_MEM_READ_WRITE,
		         sizeof(int) * w*h, NULL, &err);
 CHECK_ERROR(err);
  err=clEnqueueWriteBuffer(cmd_queue, B,CL_FALSE,0,m*n*sizeof(int),b,0,NULL,NULL);
  CHECK_ERROR(err);
	
	CHECK_ERROR(err);

	clWaitForEvents(1,bp_data.event);
	
	// Kernel
	kernel = clCreateKernel(program,"mandelbrot",&err);
	CHECK_ERROR(err);

	clFinish(cmd_queue);

	// TODO : Set the arguments.
	err=clSetKernelArg(kernel,0,sizeof(int),&m);
	err=clSetKernelArg(kernel,0,sizeof(int),&n);
	err=clSetKernelArg(kernel,0,sizeof(cl_mem),&R);
	err=clSetKernelArg(kernel,0,sizeof(cl_mem),&G);
	err=clSetKernelArg(kernel,0,sizeof(cl_mem),&B);
	err=clSetKernelArg(kernel,0,sizeof(int),&x_min);
	err=clSetKernelArg(kernel,0,sizeof(int),&x_max);
	err=clSetKernelArg(kernel,0,sizeof(int),&y_min);
	err=clSetKernelArg(kernel,0,sizeof(int),&y_max);
	err=clSetKernelArg(kernel,0,sizeof(int),&count_max);

	// Enqueue the kernel.
	err=clEnqueueNDRangeKernel(cmd_queue,kernel,1,NULL,gws,lws,0,NULL,NULL);
	CHECK_ERROR(err);

	// Read the result.
	
  err = clEnqueueReadBuffer(cmd_queue,
      R,
      CL_TRUE, 0,
      sizeof(int) * m*n,
      r,
      0, NULL, NULL);
  
  CHECK_ERROR(err);
  err = clEnqueueReadBuffer(cmd_queue,
      G,
      CL_TRUE, 0,
      sizeof(int) * m*n,
      g,
      0, NULL, NULL);
  
  CHECK_ERROR(err);
  err = clEnqueueReadBuffer(cmd_queue,
      B,
      CL_TRUE, 0,
      sizeof(int) * m*n,
      b,
      0, NULL, NULL);
  
  CHECK_ERROR(err);


  timer_stop(0);
  wtime = timer_read(0);
  printf( "\n" );
  printf( "  Time = %lf seconds.\n", wtime );

  // Write data to an ASCII PPM file.
  output_unit = fopen( output_filename, "wt" );

  fprintf( output_unit, "P3\n" );
  fprintf( output_unit, "%d  %d\n", n, m );
  fprintf( output_unit, "%d\n", 255 );
  for ( i = 0; i < m; i++ )
  {
    for ( jlo = 0; jlo < n; jlo = jlo + 4 )
    {
      jhi = MIN( jlo + 4, n );
      for ( j = jlo; j < jhi; j++ )
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
	clReleaseEvent(ev_bp);
	//clReleaseMemObject();
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);
	free(platforms);

  free(r);
  free(g);
  free(b);
}