#include <stdio.h>

double assignment() {
double y = 1;
	return y;
}
int main() {
double x = 2.5 ;
if ((double)((int)(x + assignment() )) == x + assignment() ) {
	printf("%.0f\n", (double)x + assignment() );
} else {
	printf("%.6f\n", (double)x + assignment() );
}
x;
if ((double)((int)(x)) == x) {
	printf("%.0f\n", (double)x);
} else {
	printf("%.6f\n", (double)x);
}
double x = 1;
l ;
if ((double)((int)(l)) == l) {
	printf("%.0f\n", (double)l);
} else {
	printf("%.6f\n", (double)l);
}
return 0;
}
