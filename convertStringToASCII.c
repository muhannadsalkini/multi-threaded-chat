#include <stdio.h>
#include <stdlib.h>

int main()
{
    char string = "string";
    int ascii[25] = {}, i = 0, j;
    char text[25] = "This is a test string.";
    scanf("%c", &string);
    while (text[i] != '\0')
    {
        ascii[i] = text[i];
        i++;
    }
    for (j = 0; j < i - 1; j++)
        printf("%d ", ascii[j]);
    return 0;
}