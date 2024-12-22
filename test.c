#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct A
{
    const int i;
    int j;
};

struct B
{
    struct A *a;
};

// struct B arr[1];
struct B b;
void createB()
{
    struct A *a = malloc(sizeof(struct A));
    a = &(struct A){.i = 3, .j = 2};
    // a->j = 5;
    // a->i = 2;
    b.a = a;
}

void mutateA(struct A *a)
{
    int i = a->i;
    a->j = i;
}

int main(int argc, char *argv[])
{
    createB();
    mutateA(b.a);
    printf("aaa %d", b.a->j);
    // free(b.a);
    return 0;
}