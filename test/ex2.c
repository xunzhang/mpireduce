#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "type.h"

typedef void (*castmpifunc)(void *, void *, int *, MPI_Datatype *);

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

void mpicprod(complex *in, complex *inout, int *len, MPI_Datatype *dptr) {
  int i;
  complex c;
  for (i=0; i<*len; ++i) {
    c.real=(*in).real * (*inout).real - (*in).imag * (*inout).imag;
    c.imag=(*in).real * (*inout).imag + (*in).imag * (*inout).real;
    *inout=c;
    in++;
    inout++;
  }
}

int main(int argc, char *argv[])
{
  int np, rank;
  MPI_Init(&argc, &argv);
  
  int size = 0;
  complex source = {1, 1}, src = {1, 1};
  complex result = {0, 0}, dst = {0, 0};
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  
  MPI_Datatype ctype;
  MPI_Type_contiguous(2,MPI_DOUBLE,&ctype);
  MPI_Type_commit(&ctype);
  
  MPI_Op myop;
  MPI_Op_create((castmpifunc)mpicprod, 1, &myop);
  MPI_TReduce(&src, &dst, 1, ctype, (union_func)cprod, MPI_COMM_WORLD); 
  MPI_Reduce(&source, &result, 1, ctype, myop, 0, MPI_COMM_WORLD);
  
  // output result
  if(rank == 0) { 
    printf("complex reduce result is %lf + %lf * i\n", dst.real, dst.imag);
    printf("complex reduce result(MPI_Reduce) is %lf + %lf * i\n", result.real, result.imag);
  }
  MPI_Finalize();
  return 0;
}
