#pragma GCC optimize("Ofast")

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <time.h>
#include<omp.h>

#define MAX_PRODUCTION_TERMIAL_NUM 512
#define MAX_PRODUCTION_NONTERMIAL_NUM 128
#define MAX_VN_NUM 128
#define MAX_VT_NUM 128
#define MAX_STRING_LENGTH 1024

typedef struct VtTable {
    int vn_list[MAX_VN_NUM];
    int cnt;
} VtTable;

typedef struct VnTable {
    int vn_list[MAX_VN_NUM];
    int cnt;
} VnTable;

typedef struct Production_termial {
    int parent;
    char child;
} Production_termial;

typedef struct Production_nontermial {
    int parent;
    int child1;
    int child2;
} Production_nontermial;

typedef struct DpTable {
    int cnt;
    int vn_list[MAX_VN_NUM];
    unsigned vn_path_num[MAX_VN_NUM];
} DpTable;


Production_nontermial pn[MAX_PRODUCTION_NONTERMIAL_NUM];
Production_termial pt[MAX_PRODUCTION_TERMIAL_NUM];
VtTable vtTable[MAX_VT_NUM];
VnTable vnTable[MAX_VN_NUM][MAX_VN_NUM];
DpTable dpTable[MAX_STRING_LENGTH][MAX_STRING_LENGTH];

char str[MAX_STRING_LENGTH];
int vn_num;
int pt_num;
int pn_num;
int n;

int main() {
    freopen("input2.txt", "r", stdin);
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


    for(int i=0; i<pt_num; i++) {
        int t = pt[i].child;
        int p = pt[i].parent;
        vtTable[t].vn_list[ vtTable[t].cnt++ ] = p;
    }

    for(int i=0; i<pn_num; i++) {
        int p = pn[i].parent;
        int c1 = pn[i].child1;
        int c2 = pn[i].child2;
        vnTable[c1][c2].vn_list[ vnTable[c1][c2].cnt++ ] = p;
    }

    for(int i=0 ; i<n ; i++) {
        char t=str[i];
        for(int j=0; j<vtTable[t].cnt; j++) {
            dpTable[i][i].vn_list[ dpTable[i][i].cnt++ ] = vtTable[t].vn_list[j];
            dpTable[i][i].vn_path_num[ vtTable[t].vn_list[j] ] ++;
        }
    }

    for(int len=2 ; len<=n ; len++ ) {
        #pragma omp parallel for num_threads(16)
        for(int l=0 ; l<=n-len ; l++) {
            int r = l + len -1;
            // [l,r]区间
            unsigned BC[MAX_VN_NUM][MAX_VN_NUM];
            memset(BC, 0, sizeof(BC));
            #pragma omp parallel for num_threads(16)
            for(int k=l; k<r; k++) {
                int L = dpTable[l][k].cnt;
                int R = dpTable[k+1][r].cnt;

                for(int p=0; p<L; p++) {
                    for(int q=0; q<R; q++) {
                        int vn_l = dpTable[l][k].vn_list[p];
                        int vn_r = dpTable[k+1][r].vn_list[q];
                        BC[vn_l][vn_r] += dpTable[l][k].vn_path_num[vn_l] * dpTable[k+1][r].vn_path_num[vn_r];
                    }
                }
            }

            // for(int row = 0; row<vn_num;row++) {
            //     for(int col=0; col<vn_num; col++) {
            //         printf("%u ", BC[row][col]);
            //     }
            //     printf("\n");
            // }

            for( int i=0; i<vn_num; i++) {
                for( int j=0; j<vn_num; j++ ) {
                    if(BC[i][j] > 0) {
                        for(int t=0; t<vnTable[i][j].cnt; t++) {
                            int p = vnTable[i][j].vn_list[t];
                            if( dpTable[l][r].vn_path_num[p] == 0 ) {
                                dpTable[l][r].vn_list[ dpTable[l][r].cnt++ ] = p;
                                dpTable[l][r].vn_path_num[ p ] = BC[i][j];
                            } else {
                                dpTable[l][r].vn_path_num[ p ] += BC[i][j];    
                            }
                        }
                    }
                }
            }

                        



            // for(int k=l ; k<r ; k++) {
            //     int L = dpTable[l][k].cnt;
            //     int R = dpTable[k+1][r].cnt;
            //     for(int p=0; p<L ; p++) {
            //         for(int q=0 ; q<R ; q++) {
            //             int vn_l = dpTable[l][k].vn_list[p];
            //             int vn_r = dpTable[k+1][r].vn_list[q];
            //             for(int e = 0 ; e < vnTable[vn_l][vn_r].cnt ; e++) {
            //                 int Vn = vnTable[vn_l][vn_r].vn_list[e];
            //                 int tmp = dpTable[l][k].vn_path_num[ vn_l ] * dpTable[k+1][r].vn_path_num[ vn_r ];
            //                 if( dpTable[l][r].vn_path_num[ Vn ] == 0 ) {
            //                     dpTable[l][r].vn_list[ dpTable[l][r].cnt++ ] = Vn;
            //                     dpTable[l][r].vn_path_num[Vn] = tmp;
            //                 }
            //                 else {
            //                     dpTable[l][r].vn_path_num[Vn] += tmp;
            //                 }
            //             }
            //         }
            //     }    
            // }

        }
    }
    printf("%u\n",dpTable[0][n-1].vn_path_num[0]);
}

