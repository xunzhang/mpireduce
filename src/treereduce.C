#include <stdio.h>

// not SPMD code
int tree_reduce(const int rank_in, const int np) {
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
        printf("Send to %d\n", (rank - 1) * depth);
        // make sure to overflow
        rank *= num;
        // one process can recv many times, but can only send once
        break;
      } else {
        if(rank != (num - 1)) {
          //MPI_Recv(from (rank + 1) * depth)
          //Union_opt();
          printf("Recv from %d\n", (rank + 1) * depth);
        }
        rank /= 2;
      }
      depth *= 2;
    }
    num = num / 2 + isodd;
  }
  return 0;
}

int main(void)
{
  int np, rank;
  scanf("%d %d", &rank, &np);
  tree_reduce(rank, np);
  return 0;
}
