#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include "type.h"

//typedef void (*union_func)(void *a, void *b, int len);

// not SPMD code
int tree_reduce(const int rank_in, const int np, void *sendbuf, void *recvbuf, int cnt, MPI_Datatype datatype, union_func op, MPI_Comm comm) {
  int rank = rank_in;
  int num = np;
  int isodd = 0;
  int depth = 1;
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
  return 0;
}
