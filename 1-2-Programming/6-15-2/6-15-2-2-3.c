
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct tagStudent
{
	int score;
	char name[20], id[20];
} Student;

int main ()
{
	int n, i;
	Student *stu;
	FILE *pf;

	printf ("Please input the number of new students: ");
	scanf ("%d", &n);
	stu = (Student *) malloc (sizeof (Student) * n);
	if (stu == NULL)
	{
		printf ("Bad Alloc...\n");
		return 0;
	}

	printf ("Please input the info. listed by\n\tname\n\tid\n\tscore:\n");
	for (i = 0; i < n; i++)
	{
		fflush (stdin);
		gets (stu[i].name);
		gets (stu[i].id);
		scanf ("%d", &stu[i].score);
	}

	pf = fopen ("file1.dat", "ab");
	if (pf == NULL)
	{
		printf ("Bad Open...\n");
		return 0;
	}

	fwrite (&n, sizeof (int), 1, pf);
	fwrite (stu, sizeof (Student), n, pf);
	printf ("Saved to file1.dat successfully.\n");

	free (stu);
	fclose (pf);
	return 0;
}
