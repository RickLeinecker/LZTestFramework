#include <stdio.h>

using namespace std;

int main(int argc, char* argv[])
{
     long double numerator;
     long double denominator;
     long double result;
     if (argc == 3)
     {
          sscanf(argv[1],"%Lf", &numerator);
          sscanf(argv[2],"%Lf", &denominator);

          result = numerator / denominator;
          printf("  %.16Lf\n", result);
     }
     else if (argc == 5)
     {
          long double numerator2;
          long double denominator2;
          sscanf(argv[1],"%Lf", &numerator);
          sscanf(argv[2],"%Lf", &denominator);
          sscanf(argv[3],"%Lf", &numerator2);
          sscanf(argv[4],"%Lf", &denominator2);

          result = (numerator / denominator) / (numerator2 / denominator2);
          printf("  %.16Lf\n", result);
     }
     if (result > 1.0)
          return result;
     return 0;
}