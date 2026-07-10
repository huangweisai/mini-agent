//写一个简单的程序用来验证带输入的exec命令是否能够正确执行
#include<stdio.h>
int main (){
    int a ;
    int b ;
    printf("a:");
    scanf ("%d",&a);
    printf("b:");
    scanf("%d",&b);
    printf("a+b=%d\n",a+b);
    return 0;
}