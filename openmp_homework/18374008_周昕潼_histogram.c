#include<stdio.h>
#include<stdlib.h>
#include<omp.h>

typedef struct scope {
    double left;
    double right;
    int num;
}scope;

int n;
double a[100005];
int thread_num;
int bucket_num;
double left, right;
scope list[100005];
double scope_length;

int main(int argc, char *argv[]) {
    bucket_num = atoi(argv[1]);
    thread_num = atoi(argv[2]);
    left = atof(argv[3]);
    right = atof(argv[4]);
    scope_length = (right - left) / bucket_num;
    for(int i=0;i<bucket_num;i++) {
        list[i].left = left + i * scope_length;
        list[i].right = left + (i+1) * scope_length;
        list[i].num = 0;
    }
    scanf("%d", &n);
    for (int i = 0; i < n; i++) {
        scanf("%lf", &a[i]);
    }
	omp_set_num_threads(thread_num);
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        // printf("%d\n", omp_get_thread_num());
        for (int j = 0; j < bucket_num; j++) {
            if (a[i] >= list[j].left && a[i] < list[j].right) {
                list[j].num++;
                break;
            }
        }
    }
    for (int i = 0; i < bucket_num; i++) {
        printf("From %.3lf to %.3lf: %d\n",list[i].left,list[i].right, list[i].num);
    }
    return 0;
}
