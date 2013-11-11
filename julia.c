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
#define COUNT_MAX   2000
#define MIN(x,y)    ((x) < (y) ? (x) : (y))


// Main part of the below code is originated from Lode Vandevenne's code.
// Please refer to http://lodev.org/cgtutor/juliamandelbrot.html
void julia(int w, int h) {
  // each iteration, it calculates: new = old*old + c,
  // where c is a constant and old starts at current pixel

  // real and imaginary part of the constant c
  // determinate shape of the Julia Set
  double cRe, cIm;

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
  cl_mem           R,G,B;
  cl_int           err;
  cl_uint          num_dev = 0;
//  cl_event         ev_bp;
  int i; 
  // TODO : 
/*
    // loop through every pixel
  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
*/
  size_t lws[2]={16,16};
  size_t gws[2]={h/16,w/16};



  printf( "  Parallel OpenCL version\n" );
  printf( "\n" );
  printf( "  Create an ASCII PPM image of the Julia set.\n" );
  printf( "\n" );
  printf( "  An ASCII PPM image of the set is created using\n" );
  printf( "    W = %d pixels in the X direction and\n", w );
  printf( "    H = %d pixels in the Y direction.\n", h );

  timer_init();
  timer_start(0);
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
printf("-4");
  // Command queue
  cmd_queue = clCreateCommandQueue(context, dev, 0, &err);
  CHECK_ERROR(err);
printf("-3");
  // Create a program
  // TODO : Get source code in your favor
  char * source_code=get_source_code("julia.cl");
  
 printf("-2"); 
  program = clCreateProgramWithSource(context, 1, (const char **)&source_code, NULL, &err);
  CHECK_ERROR(err);

  // Callback data for clBuildProgram
  /*
  ev_bp=clCreateUserEvent(context,&err);
  CHECK_ERROR(err);
  bp_data_t bp_data;
  bp_data.dev=dev;
  bp_data.event=&ev_bp;
  */
printf("-1");
  // Build the program.
  err = clBuildProgram(program, 1, &dev, NULL, NULL, NULL);
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
  printf("0");
  // Buffers
  // TODO: make and buffers
  
  int (*r)[h] = (int (*)[h])calloc(w * h, sizeof(int));
  int (*g)[h] = (int (*)[h])calloc(w * h, sizeof(int));
  int (*b)[h] = (int (*)[h])calloc(w * h, sizeof(int));
printf("1");
return;
  clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
      sizeof(int) * w*h, R, &err);
  err=clEnqueueWriteBuffer(cmd_queue, R,CL_FALSE,0,w*h*sizeof(int),r,0,NULL,NULL);
  CHECK_ERROR(err);
  clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
      sizeof(int) * w*h, G, &err);
  err=clEnqueueWriteBuffer(cmd_queue, G,CL_FALSE,0,w*h*sizeof(int),g,0,NULL,NULL);
  CHECK_ERROR(err);
  clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		         sizeof(int) * w*h, B, &err);
  err=clEnqueueWriteBuffer(cmd_queue, B,CL_FALSE,0,w*h*sizeof(int),b,0,NULL,NULL);
  CHECK_ERROR(err);
printf("2");
  kernel=clCreateKernel(program,"julia",&err);
  CHECK_ERROR(err);
printf("3");
  err=clSetKernelArg(kernel,0,sizeof(int),&w);
  CHECK_ERROR(err);
  err=clSetKernelArg(kernel,1,sizeof(int),&h);
  CHECK_ERROR(err);
  err=clSetKernelArg(kernel,2,sizeof(int),&cRe);
  CHECK_ERROR(err);
  err=clSetKernelArg(kernel,3,sizeof(int),&cIm);
  CHECK_ERROR(err);
  err=clSetKernelArg(kernel,4,sizeof(cl_mem),&R);
  CHECK_ERROR(err);
  err=clSetKernelArg(kernel,5,sizeof(cl_mem),&G);
  CHECK_ERROR(err);
  err=clSetKernelArg(kernel,6,sizeof(cl_mem),&B);
  CHECK_ERROR(err);
  err=clSetKernelArg(kernel,7,sizeof(int),&zoom);
  CHECK_ERROR(err);
  err=clSetKernelArg(kernel,8,sizeof(int),&moveX);
  CHECK_ERROR(err);
  err=clSetKernelArg(kernel,9,sizeof(int),&moveY);
  CHECK_ERROR(err);
  err=clSetKernelArg(kernel,10,sizeof(int),&maxIterations);
  CHECK_ERROR(err);

  printf("4");
// Enqueue the kernel.
  err=clEnqueueNDRangeKernel(cmd_queue,kernel,1,NULL,gws,lws,0,NULL,NULL);
  CHECK_ERROR(err);
printf("5");
  // Read the result.
  
  err = clEnqueueReadBuffer(cmd_queue,
      R,
      CL_TRUE, 0,
      sizeof(int) * w*h,
      r,
      0, NULL, NULL);
  
  CHECK_ERROR(err);
  err = clEnqueueReadBuffer(cmd_queue,
      G,
      CL_TRUE, 0,
      sizeof(int) * w*h,
      g,
      0, NULL, NULL);
  
  CHECK_ERROR(err);
  err = clEnqueueReadBuffer(cmd_queue,
      B,
      CL_TRUE, 0,
      sizeof(int) * w*h,
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


  // Release
  //clReleaseEvent(ev_bp);
  clReleaseMemObject(R);
  clReleaseMemObject(G);
  clReleaseMemObject(B);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(cmd_queue);
  clReleaseContext(context);
  free(platforms);


  // Terminate.
  free(r);
  free(g);
  free(b);
}
