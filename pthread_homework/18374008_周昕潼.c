#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>

/*
    y = x^2 + x
    区间: [a, b]
    n : 分为n个区间 
    method==1 busy-wait
    method==2 mutex
    method==3 semaphore
*/

void *init(void *thread_func); 
void *_busy_wait(void *rank);
void *_mutex(void *rank);
void *_semaphore(void *rank);
double f(double x);
double cal(double a, double b);

char *method_name[] = {"busy-wait", "mutex", "semaphore"};
pthread_mutex_t mutex;
sem_t sem;


long method;
long thread_count;
long long n;
double a;
double b;
double h;
int flag;
int each_count;
long double sum = 0.0;



int main(int argc, char *argv[])
{
    thread_count = atoi(argv[1]);
    method = atoi(argv[2]);
    printf("Using method: %s\n\n", method_name[method-1]);
    printf("Input the value of a,b,n:\n");
    scanf("%lf%lf%lld", &a, &b, &n);

    flag = 0; // for busy waiting
    h = (b - a) / (double)n; 
    each_count = n / thread_count; 

    if(method == 1) {
        init(_busy_wait);
    }
    else if(method == 2) {
        pthread_mutex_init(&mutex, NULL);
        init(_mutex);
    }
    else if(method == 3) {
        sem_init(&sem, 0, 1);
        init(_semaphore);
    }
    else {
        printf("Invalid method!\n");
    }

    printf("Estimate of the integral: %Lf\n", sum);
    return 0;

}

double f(double x) {
    return x * x + x;
}

double cal(double a, double b) {
    return (f(a) + f(b)) * h / 2;
}

void *init( void *thread_func ) {
    pthread_t *threads = (pthread_t *)malloc(thread_count * sizeof(pthread_t));
    for(int i=0; i<thread_count; i++) {
        pthread_create(&threads[i], NULL, thread_func, (void *)i);
    }
    for(int i=0; i<thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    free(threads);
}

void *_busy_wait(void *rank) {
    long my_rank = (long)rank;
    double my_sum = 0.0;
    long double start = a + each_count * my_rank * h;
    long double end = a + each_count * (my_rank + 1) * h;   
    for(long double x = start; x < end; x += h) {
        my_sum += cal(x, x + h);
    }
    while(flag != my_rank);
    sum += my_sum;
    flag = (flag + 1) % thread_count;
    return NULL;
}

void *_mutex(void *rank) {
    long my_rank = (long)rank;
    long double my_sum = 0.0;
    long double start = a + each_count * my_rank * h;
    long double end = a + each_count * (my_rank + 1) * h;
    for(long double x = start; x < end; x += h) {
        my_sum += cal(x, x + h);
    }
    pthread_mutex_lock(&mutex);
    sum += my_sum;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *_semaphore(void *rank) {
    long my_rank = (long)rank;
    long double my_sum = 0.0;
    long double start = a + each_count * my_rank * h;
    long double end = a + each_count * (my_rank + 1) * h;
    for(long double x = start; x < end; x += h) {
        my_sum += cal(x, x + h);
    }
    sem_wait(&sem);
    sum += my_sum;
    sem_post(&sem);
    return NULL;
}