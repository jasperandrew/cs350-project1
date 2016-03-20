#include <stdio.h>
#include <stdlib.h>



int main(int argc, char** argv)
{
  int array[] = {1,2,3};
  int* point[2048];
  point[0] = array;
  printf("%d", point[0][1]);
  return 0;
}


