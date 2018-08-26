#include <stdio.h>
#include <math.h>

int main(void)
{
  int n = 0;
  double avg = 0, epsilon = 1.96;
  
  long long current_number;
  long double var = 0;
  
  while (scanf("Converged with %lld steps.", &current_number) == 1)
  { 
    avg += current_number;
    var += current_number * current_number;
    n ++;
    getchar();
  }
  
  avg /= n;
  var = var - n*avg*avg;
  var /= (n-1);
  double sigma = sqrt(var);
  double delta = 2*epsilon*sigma / sqrt(n);
  
  printf("%f %f\n", avg, delta);
  //if (isnan(delta)) while (1) printf("aaaaa");
    
  return 0;
}

