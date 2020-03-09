
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* itoa(int x) {
	int k = 1, aux = x;
	while (aux != 0) {
		k++;
		aux /= 10;
	}
	if (x == 0) {
		k++;
	}
	printf("%d.\n", k);
	char* res = (char*)malloc(k * sizeof(char));
	memset(res, '\0', k);
	k--;
	while (k--) {
		res[k] = (x % 10) + '0';
		x = x / 10;
	}
	return res;
}


int main(void) {
	printf("%s, 10.\n", itoa(10));
	printf("%s, 1234.\n", itoa(1234));
	printf("%s, 0.\n", itoa(0));
	return 0;
}
