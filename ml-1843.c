#include <stdio.h>

int main() {
//  24 is printed
double x = 8;
double y = 3;
if ((x * y) == (int)(x * y)) printf("%d\n", (int)(x * y)); else printf("%.6f\n", (double)(x * y));
return 0;
}
