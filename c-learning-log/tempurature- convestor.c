#include <stdio.h>
#include <stdlib.h>
int main(){
     float a,b;
    printf("请输入摄氏温度");
    scanf("%f",&a);
    b=(a*9/5)+32;
    printf("%.1f",b);
    return 0;
}