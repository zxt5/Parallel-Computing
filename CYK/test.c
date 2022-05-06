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

PN pn[MAX_PRODUCTION_NONTERMIAL_NUM];
PT pt[MAX_PRODUCTION_TERMIAL_NUM];
node vnTable[MAX_VN_NUM][MAX_VN_NUM];
node vtTable[MAX_VT_NUM];
unsigned dpTable[MAX_STRING_LENGTH][MAX_STRING_LENGTH][MAX_VN_NUM];
int dpHelper[MAX_STRING_LENGTH][MAX_STRING_LENGTH][MAX_VN_NUM+2]; // 记录[i, j]字串能推出的非终结符列表
char str[MAX_STRING_LENGTH];
unsigned BC[MAX_THREAD_NUM][MAX_VN_NUM][MAX_VN_NUM];

// pthread
// pthread_t tid[MAX_STRING_LENGTH];
// sem_t sem[MAX_STRING_LENGTH];

int vn_num;
int pt_num;
int pn_num;
int n;
int i,j;


int cmp_pt(const void *a, const void *b) {
    PT* pa = (PT*)a;
    PT* pb = (PT*)b;
    if (pa->child == pb->child) {
        return pa->parent > pb->parent;
    }
    return pa->child > pb->child;
}

int cmp_pn(const void *a, const void *b) {
    PN* pa = (PN*)a;
    PN* pb = (PN*)b;
    if (pa->child1 == pb->child1) {
        if (pa->child2 == pb->child2) {
            return pa->parent > pb->parent;
        }
        return pa->child2 > pb->child2;
    }
    return pa->child1 > pb->child1;
}

void input() {
    freopen("input1.txt", "r", stdin);
    scanf("%d\n", &vn_num);
    scanf("%d\n", &pn_num);
    for (int i = 0; i < pn_num; i++) {
        scanf("<%d>::=<%d><%d>\n", &pn[i].parent, &pn[i].child1, &pn[i].child2);
    }
    scanf("%d\n", &pt_num);
    for (int i = 0; i < pt_num; i++) {
        scanf("<%d>::=%c\n", &pt[i].parent, &pt[i].child);
    }
    scanf("%d\n", &n);
    scanf("%s\n", str);
}

void init_vtTable() {
    int t;
    qsort(pt, pt_num, sizeof(PT), cmp_pt);
    memset(vtTable, -1, sizeof(node) * MAX_VT_NUM);
    for(i=0;i<pt_num; ++i) {
        t = pt[i].child;
        if(vtTable[t].begin == -1) vtTable[t].begin = vtTable[t].end = i;
        vtTable[t].end += 1;
    }
}

void init_vnTable() {
    int n1, n2;
    qsort(pn, pn_num, sizeof(PN), cmp_pn);
    for(i=0; i<vn_num; i++) {
        memset(vnTable[i], -1, sizeof(node) * vn_num);
    }
    for(i=0; i<pn_num; ++i) {
        n1 = pn[i].child1;
        n2 = pn[i].child2;
        if(vnTable[n1][n2].begin == -1) {
            vnTable[n1][n2].begin = vnTable[n1][n2].end = i;
        }
        vnTable[n1][n2].end += 1;
    }
}

void init_dpTable() {
    int t, vn;
    int begin, end;
    for(i=0 ; i<n ; ++i) {
        t = str[i];
        begin = vtTable[t].begin;
        end = vtTable[t].end;
        for(j=begin; j<end; ++j) {
            vn = pt[ j ].parent;
            if( dpTable[i][i][vn] == 0 ) {
                dpHelper[i][i][0]++;
                dpHelper[i][i][ dpHelper[i][i][0] ] = vn;
            }
            dpTable[i][i][vn]++;
        }
    }
}

void dp( int l, int r ) {
    int k, p, vn1, vn2, vn_l, vn_r;
    int vi, vj, t;
    int parent, child1, child2;
    int begin, end;
    int tid = omp_get_thread_num();
    memset(BC + tid , 0, sizeof(BC[0]));
    for(k=l; k<r; ++k) {
        vn1 = dpHelper[l][k][0];
        vn2 = dpHelper[k+1][r][0];
        // for(int p=1; p<=vn1; ++p) {
        //     for(int q=1; q<=vn2; ++q) {
        //         vn_l = dpHelper[l][k][p];
        //         vn_r = dpHelper[k+1][r][q];
        //         BC[tid][vn_l][vn_r] += dpTable[l][k][vn_l] * dpTable[k+1][r][vn_r];
        //         // printf("BC[%d][%d][%d]: %d\n", tid, p, q, BC[tid][p][q]);
        //     }
        // }  
        for(p=0; p<pn_num; p++ ) {
            parent = pn[p].parent;
            child1 = pn[p].child1;
            child2 = pn[p].child2;
            dpTable[l][r][parent] += dpTable[l][k][child1] * dpTable[k+1][r][child2];
        }
    }

    // for(vi=0; vi<vn_num; ++vi) {
    //     for(vj=0; vj<vn_num; ++vj ) {
    //         if( BC[tid][vi][vj] != 0 && vnTable[vi][vj].begin != vnTable[vi][vj].end ) {
    //             begin = vnTable[vi][vj].begin;
    //             end = vnTable[vi][vj].end;
    //             for(t=begin; t<end; ++t) {
    //                 int parent = pn[ t ].parent;
    //                 if( dpTable[l][r][parent] == 0) {
    //                     dpHelper[l][r][0]++;
    //                     dpHelper[l][r][ dpHelper[l][r][0] ] = parent;
    //                 }
    //                 dpTable[l][r][parent] += BC[tid][vi][vj];
    //             }
    //         }
    //     }
    // }
}

void dp_longstr( int l, int r ) {
    int k, vn1, vn2, vn_l, vn_r;
    int vi, vj, t;
    int p,q;
    int begin, end;
    int tid = omp_get_thread_num();
    memset(BC + tid , 0, sizeof(BC[0]));
    for(k=l; k<r; ++k) {
        vn1 = dpHelper[l][k][0];
        vn2 = dpHelper[k+1][r][0];
        for(p=1; p<=vn1; ++p) {
            for(q=1; q<=vn2; ++q) {
                vn_l = dpHelper[l][k][p];
                vn_r = dpHelper[k+1][r][q];
                BC[tid][vn_l][vn_r] += dpTable[l][k][vn_l] * dpTable[k+1][r][vn_r];
                // printf("BC[%d][%d][%d]: %d\n", tid, p, q, BC[tid][p][q]);
            }
        }  
        // for(int p=0; p<pn_num; p++ ) {
        //     int parent = pn[p].parent;
        //     int child1 = pn[p].child1;
        //     int child2 = pn[p].child2;
        //     dpTable[l][r][parent] += dpTable[l][k][child1] * dpTable[k+1][r][child2];
        // }
    }

    for(vi=0; vi<vn_num; ++vi) {
        for(vj=0; vj<vn_num; ++vj ) {
            if( BC[tid][vi][vj] != 0 && vnTable[vi][vj].begin != vnTable[vi][vj].end ) {
                begin = vnTable[vi][vj].begin;
                end = vnTable[vi][vj].end;
                for(t=begin; t<end; ++t) {
                    int parent = pn[ t ].parent;
                    if( dpTable[l][r][parent] == 0) {
                        dpHelper[l][r][0]++;
                        dpHelper[l][r][ dpHelper[l][r][0] ] = parent;
                    }
                    dpTable[l][r][parent] += BC[tid][vi][vj];
                }
            }
        }
    }
}

int main() {
    input();
    init_vtTable();
    init_vnTable();
    init_dpTable();
    int len;
    int l;

    printf("len: %d\n", n);

    void (*fp)(int a, int b) = (n>500 && vn_num > 100 ) ? dp_longstr : dp;


    for(len=2 ; len<=n ; len++ ) {
#pragma omp parallel for
        for(l=0 ; l<=n-len ; ++l) {
            // dp(l, l+len-1);
            fp(l, l+len-1);
        }
    }
    printf("%u\n",dpTable[0][n-1][0]);
}

