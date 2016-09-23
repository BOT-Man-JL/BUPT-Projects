
#include <stdio.h>

void fnDecryption ()
{
	FILE *pf, *pfo;
	int key, ch;
	
	pfo = fopen ("out.dat", "w");
	if (!pfo)
	{
		printf ("Can not create the file\n");
		return;
	}

	pf = fopen ("in.dat", "r");
	if (!pf)
	{
		printf ("Can not open the file\n");
		fclose (pfo);
		return;
	}

	ch = fgetc (pf);
	key = (tolower (ch) - 'c' + 26) % 26;
	printf ("%d\n", key);
	fprintf (pfo, "%d\n", key);
	while (ch != EOF)
	{
		ch = tolower (ch) - 'a';
		ch = (ch - key + 26) % 26;
		printf ("%c", ch + 'A');
		fprintf (pfo, "%c", ch + 'A');
		ch = fgetc (pf);
	}
}

int main ()
{
	fnDecryption ();
	return 0;
}
