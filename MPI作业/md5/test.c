#include<stdio.h>
#include "Md5.c"

int main() {
    char *s= "qwer";
    char *s_ ;
    s_ = MDString(s);
    printf("get : %s\n",s_);
}