
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
	Student stu;
	FILE *pf;
	fpos_t pos;
	char id[20];

	pf = fopen ("file1.dat", "r+b");
	if (pf == NULL)
	{
		printf ("Bad Open...\n");
		return 0;
	}

	fread (&n, sizeof (int), 1, pf);

	printf ("Please input the id of Student to edit: ");
	scanf ("%s", id);
	for (i = 0; i < n; i++)
	{
		fgetpos (pf, &pos);
		fread (&stu, sizeof (Student), 1, pf);
		if (strcmp (stu.id, id) == 0)
		{
			fsetpos (pf, &pos);
			break;
		}
	}

	if (i == n) printf ("Not found.\n");
	else
	{
		printf ("Please input the info. listed by\n\tname\n\tid\n\tscore:\n");
		fflush (stdin);
		gets (stu.name);
		gets (stu.id);
		scanf ("%d", &stu.score);
		fwrite (&stu, sizeof (Student), 1, pf);
		printf ("Saved to file1.dat successfully.\n");
	}

	fclose (pf);
	return 0;
}
