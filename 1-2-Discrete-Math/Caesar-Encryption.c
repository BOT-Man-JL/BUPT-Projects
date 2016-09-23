
#include <stdio.h>
#include <ctype.h>

void fnEncryption ()
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

	fscanf (pf, "%d", &key);
	ch = fgetc (pf);
	while (ch != EOF)
	{
		if (isalpha (ch))
		{
			ch = tolower (ch) - 'a';
			ch = (ch + key + 26) % 26;
			printf ("%c", ch + 'A');
			fprintf (pfo, "%c", ch + 'A');
		}
		ch = fgetc (pf);
	}
}

int main ()
{
	fnEncryption ();
	return 0;
}
