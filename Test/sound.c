#include <linux/vermagic.h> 
int main()
{
    printf("magic is [%s]\n", VERMAGIC_STRING);
}