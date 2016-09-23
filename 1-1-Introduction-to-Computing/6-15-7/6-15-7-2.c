#include <stdio.h>

int main ()
{
	int score[100], n, max, min, sum, i;
	
	printf ("Please input the number of the courses:");
	scanf ("%d", &n);
	printf ("Please input %d scores:", n);
	for (i = 0; i < n; i++)
		scanf ("%d", &score[i]);
	sum = max = min = score[0];
	for (i = 1; i < n; i++)
	{
		sum += score[i];
		max = max > score[i] ? max : score[i];
		min = min < score[i] ? min : score[i];
	}
	printf ("The average score is:%.2f\n", (double) sum / n);
	printf ("The max score:%d\n", max);
	printf ("The min score:%d\n", min);
	return 0;
}
