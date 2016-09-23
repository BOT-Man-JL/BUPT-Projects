
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct tagStudent
{
	int score;
	char name[20], id[20];
} Student;

void StuCpy (Student *a, Student *b)
{
	a->score = b->score;
	strcpy (a->name, b->name);
	strcpy (a->id, b->id);
}

int StuCmp (Student *a, Student *b)
{
	if (a->score > b->score) return 0;
	else if (a->score < b->score) return 1;
	else
	{
		if (strlen (a->id) < strlen (b->id)) return 0;
		else if (strlen (a->id) > strlen (b->id)) return 1;
		else
		{
			if (strcmp (a->id, b->id) > 0) return 1;
			else return 0;
		}
	}
}

void QuickSort (Student stu[], int L, int R)
{
	int l = L, r = R;
	Student tmp;

	if (l < r)
	{
		StuCpy (&tmp, &stu[l]);
		while (l < r)
		{
			while (l < r && StuCmp (&stu[r], &tmp))	r--;
			StuCpy (&stu[l], &stu[r]);
			while (l < r && StuCmp (&tmp, &stu[l]))	l++;
			StuCpy (&stu[r], &stu[l]);
		}
		StuCpy (&stu[l], &tmp);

		QuickSort (stu, L, l - 1);
		QuickSort (stu, l + 1, R);	
	}
}

int main ()
{
	int n, i;
	Student *stu;
	FILE *pf;

	pf = fopen ("file1.dat", "rb");
	if (pf == NULL)
	{
		printf ("Bad Open...\n");
		return 0;
	}

	fread (&n, sizeof (int), 1, pf);
	stu = (Student *) malloc (sizeof (Student) * n);
	fread (stu, sizeof (Student), n, pf);
	fclose (pf);

	pf = fopen ("file2.dat", "wb");
	if (pf == NULL)
	{
		printf ("Bad Create...\n");
		return 0;
	}
	QuickSort (stu, 0, n - 1);

	fwrite (&n, sizeof (int), 1, pf);
	fwrite (stu, sizeof (Student), n, pf);
	fclose (pf);
	printf ("Saved to file2.dat successfully.\n");

	free (stu);
	return 0;
}
