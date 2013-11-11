// Carry out the iteration for each pixel, determining COUNT.

__kernel void mandelbrot(int m,int n,__global int *R,__global int *G,__global int *B,
  int x_min,int x_max,int y_min,int y_max,int count_max)
{
  int i=get_global_id(0);
  int j=get_global_id(1);
  int k;
  int x1,x2,y1,y2;
     int x = ( ( float ) (     j - 1 ) * x_max  
          + ( float ) ( m - j     ) * x_min )
          / ( float ) ( m     - 1 );

     int y = ( ( float ) (     i - 1 ) * y_max  
          + ( float ) ( n - i     ) * y_min )
          / ( float ) ( n     - 1 );

      int count = 0;

      x1 = x;
      y1 = y;

      for ( k = 1; k <= count_max; k++ )
      {
        x2 = x1 * x1 - y1 * y1 + x;
        y2 = 2.0 * x1 * y1 + y;

        if ( x2 < -2.0 || 2.0 < x2 || y2 < -2.0 || 2.0 < y2 )
        {
          count = k;
          break;
        }
        x1 = x2;
        y1 = y2;
      }

      if ( ( count % 2 ) == 1 )
      {
        R[i*n+j] = 255;
        G[i*n+j] = 255;
        B[i*n+j] = 255;
      }
      else
      {
       int c = ( int ) ( 255.0 * sqrtf ( sqrtf ( sqrtf (
          ( ( float ) ( count ) / ( float ) ( count_max ) ) ) ) ) );
        R[i*n+j] = 3 * c / 5;
        G[i*n+j] = 3 * c / 5;
        B[i*n+j] = c;
      }

}