#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char *s;
    char *animPath;
    animPath = "resources/character-idle.gif";
    s = malloc(sizeof(strlen(animPath)));
    strcpy(s, animPath);
    printf("%s", s);
    printf("%lu", strlen(s));
    printf("%i", s == NULL);
}