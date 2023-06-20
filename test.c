#include <stdio.h>

void main()
{
    long num = 16;
    long* ptr = &num; // 0x7fffffffe018
    long** dptr = &ptr; // 0x7fffffffe020
    long** a = &dptr; // 0x7fffffffe028

    long i = &dptr;

    printf("*ptr : %d\n", *ptr);
    printf("ptr : %p\n", ptr);

    printf("**a : %p\n", **a);
    printf("a : %p\n", a);
    ++a;
    printf("++a : %p\n", a);
    printf("i : %p\n", i);
    printf("**(long **)i : %p\n", **(long **)i);

    char* b = "bcde";

    printf("b : %d\n",b);

    ++b;
    ++b;
    
    printf("b : %d\n",b);
    printf("b : %s\n",b);

    printf("b : %d\n",b);
}
