#include <stdio.h>
#include <mpi.h>

// not SPMD code
int tree_reduce(const int rank_in, const int np, void *sendbuf, void *recvbuf, int cnt, MPI_Datatype datatype, MPI_Comm comm) {
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
        printf("Process %d Send to %d\n", rank_in, (rank - 1) * depth);
        rank *= num;
        // one process can only send once
        break;
      } else {
        i = 0;
        if(rank != (num - 1)) {
          //MPI_Recv(from (rank + 1) * depth)
          MPI_Recv(recvbuf, cnt, datatype, (rank + 1) * depth, 2013, comm, MPI_STATUS_IGNORE);
          //Union_opt();
          for(; i < cnt; ++i) {
            *((int *)sendbuf + i) += *((int *)recvbuf + i);
          }
          printf("Process %d Recv from %d with value %d\n", rank_in, (rank + 1) * depth, *(int *)recvbuf);
          printf("Process %d update send value with %d\n", rank_in, *(int *)sendbuf);
        }
        rank /= 2;
      }
      depth *= 2;
    }
    num = num / 2 + isodd;
  }
  if(rank_in == 0) {
    for(i = 0; i < cnt; ++i) {
      *((int *)recvbuf + i) = *((int *)sendbuf + i);
    }
  }
  return 0;
}

// SPMD code
int MPI_TReduce(void *sendbuf, void *recvbuf, int cnt, MPI_Datatype datatype, MPI_Comm comm) {
  int rank, np; 
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  tree_reduce(rank, np, sendbuf, recvbuf, cnt, datatype, comm);
}

int main(int argc, char *argv[])
{
  int np, rank;
  MPI_Init(&argc, &argv);
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  
  int i = rank + 1, sum = 0;
  
  MPI_TReduce(&i, &sum, 1, MPI_INT, MPI_COMM_WORLD);
  
  if(rank == 0) 
    printf("reduce result is %d\n", sum);
  MPI_Finalize();
  return 0;
}
