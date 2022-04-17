#include <stdio.h>
#include<stdlib.h>
#include<time.h>
#include <mpi.h>
#include <stdint.h>

// MPI_COMM_WORLD 总通信域

/* 
    int MPI_Send(type* buf, int count, MPI_Datatype, int dest, int tag, MPI_Comm comm)
    buf - 传递数据的首地址
    count - 传递变量数
    MPI_Datatype - 数据类型  eg：MPI_INT
    dest - 接收的进程号
    tag - 发送和接收需要tag一致
    MPI_Comm - 通信域
*/

/*
    int MPI_Recv(type* buf, int count, MPI_Datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) 
    source - 发送消息的进程号
    status - 返回状态信息，变量类型为MPI_Status
    分别有三个域，可以通过status.MPI_SOURCE，status.MPI_TAG和status.MPI_ERROR的方式调用这三个信息
    这三个信息分别返回的值是所收到数据发送源的进程号，该消息的tag值和接收操作的错误代码。
*/


long long int total_num_in_circle;


int main(int argc,char* argv[]){


    long long int number_of_tosses=0;
    long long int num_each_proc;
    long long int local_num_in_circle=0;
    int rank;
    int num;
    time_t t;

    MPI_Init(&argc,&argv);

    MPI_Comm_size(MPI_COMM_WORLD,&num);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    srand(time(&t)+rank);

    if(rank == 0) {
        printf("Enter the total number if tosses:");
        fflush(stdout);
        scanf("%lld",&number_of_tosses);
        num_each_proc = number_of_tosses%num==0 ? number_of_tosses/num : number_of_tosses/num+1;
    }

    MPI_Bcast(&num_each_proc, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);


    for(int i=0; i<num_each_proc; i++) {
        double x = -1.0 + 1.0 * rand() / RAND_MAX * ( 1.0 + 1.0 );
        double y = -1.0 + 1.0 * rand() / RAND_MAX * ( 1.0 + 1.0 );
        double distance = x*x + y*y;
        if( distance <= 1) local_num_in_circle++;
    }

    // printf("Process %d : pi = %lf\n",rank,4.0*(double)local_num_in_circle/num_each_proc);

    MPI_Reduce(&local_num_in_circle, &total_num_in_circle, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD); 

    if(rank == 0) {
        double ans = 4.0 * (double)total_num_in_circle / number_of_tosses;
        printf("PI = %lf\n",ans);
    }
    
    MPI_Finalize();

}

