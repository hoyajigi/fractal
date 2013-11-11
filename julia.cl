__kernel julia(int w,int h,cRe,cIm,int R,int G,int B,int zoom,int moveX,int moveY,int maxIterations)
{
	int x;
	int y;
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
      r[y][x] = color.r;
      g[y][x] = color.g;
      b[y][x] = color.b;	
}

