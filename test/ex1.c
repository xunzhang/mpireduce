#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "type.h"

void myadd(int *a, int *b, int n) {
  int i = 0;
  for(; i < n; ++i)
    b[i] += a[i];
}

typedef struct {
  double real,imag;
} complex;

void cprod(complex *in, complex *inout, int len) {
  int i;
  complex c;
  //printf("in complex is %d + %d * i\n", in->real, in->imag);
  //printf("inout complex is %d + %d * i\n", inout->real, inout->imag);
  for (i = 0; i < len; ++i) {
    c.real = in->real * inout->real - in->imag * inout->imag;
    c.imag = in->real * inout->imag + in->imag * inout->real;
    *inout=c;
    in++;
    inout++;
  }
}

int main(int argc, char *argv[])
{
  int np, rank;
  int size = 0;
  MPI_Init(&argc, &argv);
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  
  int i = rank + 1, sum = 0;
  
  MPI_Datatype ctype;
  MPI_Type_contiguous(2,MPI_DOUBLE,&ctype);
  MPI_Type_commit(&ctype);
  
  complex src = {1, 1};
  complex dst = {0, 0};
  
  //MPI_Type_size(ctype, &size);
  MPI_TReduce(&i, &sum, 1, MPI_INT, (union_func)myadd, MPI_COMM_WORLD);
  //printf("leng cnt is %d\n", size);
  MPI_TReduce(&src, &dst, 1, ctype, (union_func)cprod, MPI_COMM_WORLD); 
  
  // output result
  if(rank == 0) { 
    printf("reduce result is %d\n", sum);
    printf("complex reduce result is %lf + %lf * i\n", dst.real, dst.imag);
  }
  MPI_Finalize();
  return 0;
}
