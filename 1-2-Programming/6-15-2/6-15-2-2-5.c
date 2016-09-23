
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
	Student *stu, tstu;
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

	printf ("Please input the id of Student to delete: ");
	scanf ("%s", id);
	for (i = 0; i < n; i++)
	{
		fgetpos (pf, &pos);
		fread (&tstu, sizeof (Student), 1, pf);
		if (strcmp (tstu.id, id) == 0) break;
	}

	if (i == n)
	{
		printf ("Not found.\n");
		fclose (pf);
		return 0;
	}

	stu = (Student*) malloc (sizeof (Student) * (n - i - 1));
	if (stu == NULL)
	{
		printf ("Bad Alloc...\n");
		fclose (pf);
		return 0;
	}
	fread (stu, sizeof (Student), n - i - 1, pf);
	fsetpos (pf, &pos);
	fwrite (stu, sizeof (Student), n - i - 1, pf);
	free (stu);

	stu = (Student*) malloc (sizeof (Student));  //the last data to delete
	memset (stu, 0, sizeof (Student));
	if (stu == NULL)
	{
		printf ("Bad Alloc...\n");
		fclose (pf);
		return 0;
	}
	fwrite (stu, sizeof (Student), 1, pf);
	printf ("Saved to file1.dat successfully.\n");
	free (stu);

	n--;
	fseek (pf, 0, SEEK_SET);
	fwrite (&n, sizeof (int), 1, pf);

	fclose (pf);
	return 0;
}
