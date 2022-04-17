/*
 * �ļ�����    mpi_odd_even.c
 * ����:      ʵ�ֶԷǸ��������еĲ�����ż����
 * ���룺
 *    A:     ����Ԫ�� (��ѡ)
 * �����
 *    A:     ������Ԫ��
 *
 * �������  mpicc -g -Wall -o mpi_odd_even mpi_odd_even.c
 * �������
 *    mpiexec -n <p> mpi_odd_even <g|i> <global_n> 
 *       - p: ������
 *       - g: ��������ҷֲ�������
 *       - i: �û��ڽ���0����������
 *       - global_n: ȫ�������е�Ԫ�ظ���
 *
 * ע�ͣ�
 * 1.  global_n�����ܱ�p����
 * 2.  ���˵�������������������0�Ž��̸����������
 * 3.  ����ѡ�� -DDEBUG ������������ʶ��
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>

const int RMAX = 100;

/* ���غ��� */
void Usage(char* program);
void Print_list(int local_A[], int local_n, int rank);
void Merge_low(int local_A[], int temp_B[], int temp_C[], 
         int local_n);
void Merge_high(int local_A[], int temp_B[], int temp_C[], 
        int local_n);
void Generate_list(int local_A[], int local_n, int my_rank);
int  Compare(const void* a_p, const void* b_p);

/* ����ͨ�ŵĺ��� */
void Get_args(int argc, char* argv[], int* global_n_p, int* local_n_p, 
         char* gi_p, int my_rank, int p, MPI_Comm comm);
void Sort(int local_A[], int local_n, int my_rank, 
         int p, MPI_Comm comm);
void Odd_even_iter(int local_A[], int temp_B[], int temp_C[],
         int local_n, int phase, int even_partner, int odd_partner,
         int my_rank, int p, MPI_Comm comm);
void Print_local_lists(int local_A[], int local_n, 
         int my_rank, int p, MPI_Comm comm);
void Print_global_list(int local_A[], int local_n, int my_rank,
         int p, MPI_Comm comm);
void Read_list(int local_A[], int local_n, int my_rank, int p,
         MPI_Comm comm);


/*-------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   int my_rank, p;
   char g_i;
   int *local_A;
   int global_n;
   int local_n;
   MPI_Comm comm;

   MPI_Init(&argc, &argv);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &my_rank);

   Get_args(argc, argv, &global_n, &local_n, &g_i, my_rank, p, comm);
   local_A = (int*) malloc(local_n*sizeof(int));
   if (g_i == 'g') {
      Generate_list(local_A, local_n, my_rank);
      Print_local_lists(local_A, local_n, my_rank, p, comm);
   } else {
      Read_list(local_A, local_n, my_rank, p, comm);
#     ifdef DEBUG
      Print_local_lists(local_A, local_n, my_rank, p, comm);
#     endif
   }

#  ifdef DEBUG
   printf("���� %d > ����ǰ\n", my_rank);
   fflush(stdout);
#  endif
   Sort(local_A, local_n, my_rank, p, comm);

#  ifdef DEBUG
   Print_local_lists(local_A, local_n, my_rank, p, comm);
   fflush(stdout);
#  endif

   Print_global_list(local_A, local_n, my_rank, p, comm);

   free(local_A);

   MPI_Finalize();

   return 0;
}  /* main */


/*-------------------------------------------------------------------
 * ��������   Generate_list
 * ���ܣ�     �����������������
 * ���������  local_n, my_rank
 * ���������  local_A
 */
void Generate_list(int local_A[], int local_n, int my_rank) {
   int i;

    srand(time(NULL)+my_rank);
    for (i = 0; i < local_n; i++)
       local_A[i] = rand() % RMAX;

}  /* Generate_list */


/*-------------------------------------------------------------------
 * ��������   Usage
 * ���ܣ�     ��������е����������
 * ���������  ��ִ�г�������
 * ע�ͣ�     �����أ�ֻ�ɽ���0ִ�У�
 */
void Usage(char* program) {
   fprintf(stderr, "�÷���  mpirun -np <p> %s <g|i> <global_n>\n",
       program);
   fprintf(stderr, "   - p: ������Ŀ \n");
   fprintf(stderr, "   - g: �Զ���������ҷֲ�������\n");
   fprintf(stderr, "   - i: ���û���0�Ž����ֹ���������\n");
   fprintf(stderr, "   - global_n: ȫ�������е�Ԫ�ظ���");
   fprintf(stderr, " (�����ܹ���p����)\n");
   fflush(stderr);
}  /* Usage */


/*-------------------------------------------------------------------
 * ��������   Get_args
 * ���ܣ�     ��ȡ����������в���
 * ���������  argc, argv, my_rank, p, comm
 * ���������  global_n_p, local_n_p, gi_p
 */
void Get_args(int argc, char* argv[], int* global_n_p, int* local_n_p, 
         char* gi_p, int my_rank, int p, MPI_Comm comm) {

   if (my_rank == 0) {
      if (argc != 3) {    /* ��������в�������3�� */
         Usage(argv[0]);
         *global_n_p = -1;  /* ���������˳� */
      } else {
         *gi_p = argv[1][0];
         if (*gi_p != 'g' && *gi_p != 'i') {   /* ����ڶ�����������g����i */
            Usage(argv[0]);
            *global_n_p = -1;  /* ���������˳� */
         } else {
            *global_n_p = atoi(argv[2]);  /* ����������ת������ */
            if (*global_n_p % p != 0) {   /* �粻�ܱ�p���� */
               Usage(argv[0]);
               *global_n_p = -1;    /* ������ֵ�����˳� */
            }
         }
      }
   }  /* my_rank == 0 */

   MPI_Bcast(gi_p, 1, MPI_CHAR, 0, comm);
   MPI_Bcast(global_n_p, 1, MPI_INT, 0, comm);

   if (*global_n_p <= 0) {
      MPI_Finalize();
      exit(-1);
   }

   *local_n_p = *global_n_p/p;
#  ifdef DEBUG
   printf("���� %d > gi = %c, global_n = %d, local_n = %d\n",
      my_rank, *gi_p, *global_n_p, *local_n_p);
   fflush(stdout);
#  endif

}  /* Get_args */


/*-------------------------------------------------------------------
 * ��������   Read_list
 * ���ܣ�     0�Ž��̴�stdin��ȡ���У���ɢ�����������̡�
 * ���������    local_n, my_rank, p, comm
 * ���������    local_A
 */
void Read_list(int local_A[], int local_n, int my_rank, int p,
         MPI_Comm comm) {
   int i;
   int *temp;

   if (my_rank == 0) {
      temp = (int*) malloc(p*local_n*sizeof(int));
      printf("��������������Ԫ��\n");
	  fflush(stdout);
      for (i = 0; i < p*local_n; i++)
         scanf("%d", &temp[i]);
   } 

   MPI_Scatter(temp, local_n, MPI_INT, local_A, local_n, MPI_INT,
       0, comm);

   if (my_rank == 0)
      free(temp);
}  /* Read_list */


/*-------------------------------------------------------------------
 * ��������   Print_global_list
 * ���ܣ�     ��ӡ���ȫ������A
 * ���������  
 *    n, Ԫ�ظ���the number of elements 
 *    A, ����
 * ע�ͣ�     �����أ���0�Ž��̵���
 */
void Print_global_list(int local_A[], int local_n, int my_rank, int p, 
      MPI_Comm comm) {
   int* A;
   int i, n;

   if (my_rank == 0) {
      n = p*local_n;
      A = (int*) malloc(n*sizeof(int));
      MPI_Gather(local_A, local_n, MPI_INT, A, local_n, MPI_INT, 0,
            comm);
      printf("ȫ������:\n");
      for (i = 0; i < n; i++)
         printf("%d ", A[i]);
      printf("\n\n");
      free(A);
   } else {
	  n = p*local_n;
      A = (int*) malloc(n*sizeof(int));
      MPI_Gather(local_A, local_n, MPI_INT, A, local_n, MPI_INT, 0,
            comm);
	  free(A);
   }

}  /* Print_global_list */

/*-------------------------------------------------------------------
 * ��������   Compare
 * ���ܣ�     �Ƚ�������������һ�����ȵڶ�����С�򷵻�-1����ȷ���0��
 *           ��һ�����ȵڶ��������򷵻�1����qsort���á�
 */
int Compare(const void* a_p, const void* b_p) {
   int a = *((int*)a_p);
   int b = *((int*)b_p);

   if (a < b)
      return -1;
   else if (a == b)
      return 0;
   else /* a > b */
      return 1;
}  /* Compare */

/*-------------------------------------------------------------------
 * ��������      Sort
 * ���ܣ�        �Ա������н�������ʹ����ż�����㷨��ȫ�����н�������
 * ���������     local_n, my_rank, p, comm
 * ����/��������� local_A 
 */
void Sort(int local_A[], int local_n, int my_rank, 
         int p, MPI_Comm comm) {
   int phase;
   int *temp_B, *temp_C;
   int even_partner;  /* ż���׶λ�������phase is even or left-looking */
   int odd_partner;   /* �����׶λ������ҿ�phase is odd or right-looking */

   /* �ϲ�-�ָ�ʱ����������ݵ���ʱ��Ԫ */
   temp_B = (int*) malloc(local_n*sizeof(int));
   temp_C = (int*) malloc(local_n*sizeof(int));

   /* ����ͬ�飺  ����̺߳�Ϊ�� => �˽׶�ʲôҲ���� */
   if (my_rank % 2 != 0) {   // ��������
      even_partner = my_rank - 1;  // ż��ͬ��
      odd_partner = my_rank + 1;   // ����ͬ��
      if (odd_partner == p) odd_partner = MPI_PROC_NULL;  // ���ұߵĽ���������������������ͬ�飬����
   } else {                  // ż������
      even_partner = my_rank + 1;  // ����ͬ��
      if (even_partner == p) even_partner = MPI_PROC_NULL;  // ���ұߵĽ���������ż����������ͬ�飬����
      odd_partner = my_rank-1;     // ż��ͬ��
   }

   /* Sort local list using built-in quick sort */
   qsort(local_A, local_n, sizeof(int), Compare);

#  ifdef DEBUG
   printf("���� %d > ��ʼ��������ѭ��֮ǰ\n", my_rank);
   fflush(stdout);
#  endif

   for (phase = 0; phase < p; phase++){
      Odd_even_iter(local_A, temp_B, temp_C, local_n, phase, 
             even_partner, odd_partner, my_rank, p, comm);
	  Print_local_lists(local_A, local_n, my_rank, p, comm);
   }

   free(temp_B);
   free(temp_C);
}  /* Sort */


/*-------------------------------------------------------------------
 * ��������       Odd_even_iter
 * ���ܣ�        һ����ż��������
 * ���������     local_n, phase, my_rank, p, comm
 * ����/��������� local_A
 * ��ʱ������     temp_B, temp_C
 */
void Odd_even_iter(int local_A[], int temp_B[], int temp_C[],
        int local_n, int phase, int even_partner, int odd_partner,
        int my_rank, int p, MPI_Comm comm) {
   MPI_Status status;

   if (phase % 2 == 0) {
      if (even_partner >= 0) {
         MPI_Sendrecv(local_A, local_n, MPI_INT, even_partner, 0, 
            temp_B, local_n, MPI_INT, even_partner, 0, comm,
            &status);
         if (my_rank % 2 != 0)
            Merge_high(local_A, temp_B, temp_C, local_n);
         else
            Merge_low(local_A, temp_B, temp_C, local_n);
      }
   } else { /* odd phase */
      if (odd_partner >= 0) {
         MPI_Sendrecv(local_A, local_n, MPI_INT, odd_partner, 0, 
            temp_B, local_n, MPI_INT, odd_partner, 0, comm,
            &status);
         if (my_rank % 2 != 0)
            Merge_low(local_A, temp_B, temp_C, local_n);
         else
            Merge_high(local_A, temp_B, temp_C, local_n);
      }
   }
}  /* Odd_even_iter */


/*-------------------------------------------------------------------
 * ��������       Merge_low
 * ���ܣ�        ����С��local_n��Ԫ�غϲ���my_keys�У�Ȼ���recv_keys
 *              ����temp_keys������ٰ�temp_keys������my_keys��
 * ���������     local_n, recv_keys
 * ����/��������� my_keys
 * ��ʱ������     temp_keys
 */
void Merge_low(
      int  my_keys[],     /* in/out    */
      int  recv_keys[],   /* in        */
      int  temp_keys[],   /* scratch   */
      int  local_n        /* = n/p, in */) {
   int m_i, r_i, t_i;
   
   m_i = r_i = t_i = 0;
   while (t_i < local_n) {
      if (my_keys[m_i] <= recv_keys[r_i]) {
         temp_keys[t_i] = my_keys[m_i];
         t_i++; m_i++;
      } else {
         temp_keys[t_i] = recv_keys[r_i];
         t_i++; r_i++;
      }
   }

   memcpy(my_keys, temp_keys, local_n*sizeof(int));
}  /* Merge_low */

/*-------------------------------------------------------------------
 * ��������       Merge_high
 * ���ܣ�        ������local_n��Ԫ�غϲ���local_A������temp_B
 *              ����temp_C��Ȼ���ٽ�temp_C������local_A��
 * ���������     local_n, temp_B
 * ����/��������� local_A
 * ��ʱ������     temp_C
 */
void Merge_high(int local_A[], int temp_B[], int temp_C[], 
        int local_n) {
   int ai, bi, ci;
   
   ai = local_n-1;
   bi = local_n-1;
   ci = local_n-1;
   while (ci >= 0) {
      if (local_A[ai] >= temp_B[bi]) {
         temp_C[ci] = local_A[ai];
         ci--; ai--;
      } else {
         temp_C[ci] = temp_B[bi];
         ci--; bi--;
      }
   }

   memcpy(local_A, temp_C, local_n*sizeof(int));
}  /* Merge_high */


/*-------------------------------------------------------------------
 * Only called by process 0
 */
void Print_list(int local_A[], int local_n, int rank) {
   int i;
   printf("%d: ", rank);
   for (i = 0; i < local_n; i++)
      printf("%d ", local_A[i]);
   printf("  ");
}  /* Print_list */

/*-------------------------------------------------------------------
 * ��������   Print_local_lists
 * ���ܣ�     ��ӡ���ÿ�����̵�ǰ��������
 * ���������  ȫ��
 * ע�ͣ�
 * 1.  �ٶ����в�������Ľ��̶�������local_n��Ԫ��
 
 */
void Print_local_lists(int local_A[], int local_n, 
         int my_rank, int p, MPI_Comm comm) {
   int*       A;
   int        q;
   MPI_Status status;

   if (my_rank == 0) {
      A = (int*) malloc(local_n*sizeof(int));
      Print_list(local_A, local_n, my_rank);
      for (q = 1; q < p; q++) {
         MPI_Recv(A, local_n, MPI_INT, q, 0, comm, &status);
         Print_list(A, local_n, q);
      }
	  printf("\n");
      free(A);
   } else {
      MPI_Send(local_A, local_n, MPI_INT, 0, 0, comm);
   }
}  /* Print_local_lists */
