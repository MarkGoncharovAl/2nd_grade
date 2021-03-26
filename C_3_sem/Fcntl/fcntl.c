#include <stdlib.h>
#include <stdio.h>

#define SIZE 100

int main()
{
    // FIXIT: создайте отдельную именованную константу для числа 100
    char digits[SIZE];
    int k, m;
    printf("Write number, old system and new system\n");

    scanf("%s%d%d", digits, &k, &m);
    char *endptr;
    long long int number = strtol(digits, &endptr, k);
    if (*endptr != '\0')
    {
        printf("It is not number!");
        return 1;
    }

    printf("%s", digits);
    return 0;
}