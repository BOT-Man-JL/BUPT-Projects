#include <stdio.h>
#include <string.h>

int main ()
{
	char buf[1024], tmp[1024], *pch, *pchtmp;
	FILE *pfi, *pfo;
	fpos_t pos;
	enum State
	{
		FREE,
		BLOCK,
		QUOTE,
		TEST
	} state = FREE;

	printf ("Please input the file name (with no .c): ");
	gets (buf);
	strcpy (tmp, buf);
	strcat (buf, ".c");
	strcat (tmp, "2.c");

	pfi = fopen (buf, "r");
	if (pfi == NULL)
	{
		printf ("No such file in current directory.");
		getchar ();
		return 0;
	}
	
	pfo = fopen (tmp, "w");
	if (pfo == NULL)
	{
		printf ("Can not create the output file.");
		fclose (pfi);
		getchar ();
		return 0;
	}

	while (1)  //scan every line
	{
		if (state != TEST)
		{
			fgetpos (pfi, &pos);
			if (!fgets (buf, 1024, pfi)) break;  //== feof
		}

		switch (state)
		{
		case FREE:
			pch = strchr (buf, '\"');  // Find "
			if (pch != NULL)  // Found
			{
				(*pch) = 0;
				if (pchtmp = strchr (buf, '/'))  // Find / before "
					if (*(pchtmp + 1) == '*' || *(pchtmp + 1) == '/')
					{
						(*pch) = '\"';  // Ungetc
						state = TEST;  // Trans State
						break;
					}
				fputs (buf, pfo);
				fputs ("\"", pfo);
				fsetpos (pfi, &pos);  // Ungets
				fseek (pfi, strlen (buf) + 1, SEEK_CUR);
				state = QUOTE;  // Trans State
			}
			else
				state = TEST;  // Trans State
			break;

		case QUOTE:
			pch = strchr (buf, '\"');  // Find:"
			if (pch != NULL)
			{
				(*pch) = 0;
				fputs (buf, pfo);
				fputs ("\"", pfo);
				fsetpos (pfi, &pos);  // Ungets
				fseek (pfi, strlen (buf) + 1, SEEK_CUR);
				state = FREE;  // Trans State
			}
			else
				state = TEST;  // Trans State
			break;

		case TEST:
			pch = strchr (buf, '/');  // Find /
			while (pch)
			{
				if (*(pch + 1) == '/')  // Found //
				{
					(*pch) = 0;
					fputs (buf, pfo);
					fputs ("\n", pfo);
					state = FREE;  //Trans State
					break;
				}
				else if(*(pch + 1) == '*')  // Found /*
				{
					(*pch) = 0;
					fputs (buf, pfo);
					fsetpos (pfi, &pos);  // Ungets
					fseek (pfi, strlen (buf) + 2, SEEK_CUR);
					state = BLOCK;  // Trans State
					break;
				}
				else  // Bad finding
					pch = strchr (pch + 1, '/');
			}
			if (pch == NULL)  // Not Found
			{
				fputs (buf, pfo);
				state = FREE;  //Trans State
			}
			break;

		case BLOCK:
			pch = strchr (buf, '*');  // Find */
			while (pch)
			{
				if (*(pch + 1) == '/')  // Found
				{
					fsetpos (pfi, &pos);
					fseek (pfi, strlen (buf) - strlen (pch + 2), SEEK_CUR);
					state = FREE;  // Trans State
					break;
				}
				else  // Bad finding
					pch = strchr (pch + 1, '*');
			}
			if (pch == NULL)  // Not Found
				fputs ("\n", pfo);
			break;
		}
	}

	fclose (pfi);
	fclose (pfo);

	return 0;
}
