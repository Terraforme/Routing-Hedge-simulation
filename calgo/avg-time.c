#include <stdio.h>
#include <math.h>

int main(void)
{
  int n = 0;
  double avg = 0, epsilon = 1.96, var = 0;
  double current_number;
  
  while (scanf("Time used : %lf", &current_number) == 1)
  { 
    getchar();    
    avg += current_number;
    var += current_number * current_number;
    n ++;
  }
  
  avg /= n;
  var = var - n*avg*avg;
  var /= (n-1);
  double sigma = sqrt(var);
  double delta = 2*epsilon*sigma / sqrt(n);
  
  printf("%f %f\n", avg, delta);
    
  return 0;
}
