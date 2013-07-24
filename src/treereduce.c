#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

typedef void (*union_func)(void *a, void *b, int len);

// not SPMD code
int tree_reduce(const int rank_in, const int np, void *sendbuf, void *recvbuf, int cnt, MPI_Datatype datatype, union_func op, MPI_Comm comm) {
  int rank = rank_in;
  int num = np;
  int isodd = 0;
  int depth = 1;
  int i;
  // many sends during a reduce process
  while(num > 1) {
    // check send scope
    if(rank < num) {
      isodd = num % 2;
      // odd send to (odd - 1)
      if(rank % 2 != 0) {
        //MPI_Send(to (rank - 1) * depth)
        MPI_Send(sendbuf, cnt, datatype, (rank - 1) * depth, 2013, comm);
        printf("Process %d Send to %d with value %lf + %lf * i\n", rank_in, (rank - 1) * depth, ((double *)sendbuf)[0], ((double *)sendbuf)[1]);
        rank *= num;
        // one process can only send once
        break;
      } else {
        i = 0;
        if(rank != (num - 1)) {
          //MPI_Recv(from (rank + 1) * depth)
          MPI_Recv(recvbuf, cnt, datatype, (rank + 1) * depth, 2013, comm, MPI_STATUS_IGNORE);
          //Union_opt();
          op(recvbuf, sendbuf, cnt);
          //printf("recv value is %lf + %lf * i\n", ((double *)recvbuf)[0], ((double *)recvbuf)[1]);
          //printf("send value is %lf + %lf * i\n", ((double *)sendbuf)[0], ((double *)sendbuf)[1]);
          printf("Process %d Recv from %d with value %lf + %lf * i\n", rank_in, (rank + 1) * depth, ((double *)recvbuf)[0], ((double *)recvbuf)[1]);
          printf("Process %d update send value with %lf + %lf * i\n", rank_in, ((double *)sendbuf)[0], ((double *)sendbuf)[1]);
        }
        rank /= 2;
      }
      depth *= 2;
    }
    num = num / 2 + isodd;
  }
  // last reduce
  if(rank_in == 0) {
    int typesize;
    MPI_Type_size(datatype, &typesize);
    memcpy(recvbuf, sendbuf, cnt * typesize);
    printf("recv value is %lf + %lf * i\n", ((double *)recvbuf)[0], ((double *)recvbuf)[1]);
  }
  /*
  if(rank_in == 0) {
    for(i = 0; i < cnt * type_num; ++i) {
      *((double *)recvbuf + i) = *((double *)sendbuf + i);
    }
  }
  */
  return 0;
}

// SPMD code
int MPI_TReduce(void *sendbuf, void *recvbuf, int cnt, MPI_Datatype datatype, union_func op, MPI_Comm comm) {
  int rank, np; 
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  tree_reduce(rank, np, sendbuf, recvbuf, cnt, datatype, op, comm);
}

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
