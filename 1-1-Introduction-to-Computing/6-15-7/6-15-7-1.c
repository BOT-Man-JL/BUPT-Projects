#include <stdio.h>

int isWaveNum (int num)
{
	static int odd = -1, even = -1, count = 1;
	
	if (!num) return 1;
	if (count != 1 && even == odd) return 0;
	if (count % 2)
	{
		if (odd == -1) odd = num % 10;
		else if (odd != num % 10) return 0;
	}
	else
	{
		if (even == -1) even = num % 10;
		else if (even != num % 10) return 0;
	}
	count++;
	return isWaveNum (num / 10);
}

int main ()
{
	int n;
	scanf ("%d", &n);
	if (isWaveNum (n)) printf ("YES\n");
	else printf ("NO\n");
	return 0;
}
