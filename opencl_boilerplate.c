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
