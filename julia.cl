typedef struct {
  unsigned int r;
  unsigned int g;
  unsigned int b;
} RgbColor;

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

__kernel void julia(int w,int h,int cRe,int cIm,__global int *R,__global int *G,__global int *B,int zoom,int moveX,int moveY,int maxIterations)
{
	int x=get_global_id(0);
	int y=get_global_id(1);
	int newRe,newIm,oldRe,oldIm;
	
    // calculate the initial real and imaginary part of z,
    // based on the pixel location and zoom and position values
      newRe = 1.5 * (x - w / 2) / (0.5 * zoom * w) + moveX;
      newIm = (y - h / 2) / (0.5 * zoom * h) + moveY;

      // idd will represent the number of iterations
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
      R[y*h+x] = color.r;
      G[y*h+x] = color.g;
      B[y*h+x] = color.b;	
	  R[y*h+x]=255;
	  G[y*h+x]=255;
	  B[y*h+x]=255;
}
