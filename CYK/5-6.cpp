#pragma GCC optimize("Ofast")
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<omp.h>
#include<pthread.h>
#include<semaphore.h>


#define MAX_PRODUCTION_TERMIAL_NUM 128
#define MAX_PRODUCTION_NONTERMIAL_NUM 512
#define MAX_VN_NUM 128
#define MAX_VT_NUM 128
#define MAX_STRING_LENGTH 1024
#define MAX_THREAD_NUM 32

typedef struct node {
    int begin;
    int end;
} node;

typedef struct PT {
    int parent;
    unsigned char child;
} PT;

typedef struct PN{
    int parent;
    int child1;
    int child2;
} PN;


char str[MAX_STRING_LENGTH];
int vn_num;
int pt_num;
int pn_num;
int n;
int i,j;
int P[MAX_STRING_LENGTH][MAX_STRING_LENGTH][MAX_VN_NUM];

int main() {
    
}




