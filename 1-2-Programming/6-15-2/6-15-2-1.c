
#include <stdio.h>
#include <string.h>

int main ()
{
	int fBlock = 0, fQuotation = 0;
	char buf[1024], tmp[1024], *pch, *pchtmp;
	FILE *pfi, *pfo;
	fpos_t pos;

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
		fgetpos (pfi, &pos);
		if (!fgets (buf, 1024, pfi)) break;  //== feof

		if (!fBlock)  //not in the block
		{
			if (!fQuotation)
			{
				pch = strchr (buf, '\"');
				if (pch != NULL)
				{
					(*pch) = 0;
					if (pchtmp = strchr (buf, '/'))  //there is comment before the '\"'
						if (*(pchtmp + 1) == '*' || *(pchtmp + 1) == '/')
						{
							(*pch) = '\"';
							goto tag;
						}
					fputs (buf, pfo);
					fputs ("\"", pfo);
					fsetpos (pfi, &pos);  //maybe another "\"" is in the same line
					fseek (pfi, strlen (buf) + 1, SEEK_CUR);
					fQuotation = 1;
					continue;
				}
			}
			else
			{	
				pch = strchr (buf, '\"');
				if (pch != NULL)
				{
					(*pch) = 0;
					fputs (buf, pfo);
					fputs ("\"", pfo);
					fsetpos (pfi, &pos);  //maybe another "\"" is in the same line
					fseek (pfi, strlen (buf) + 1, SEEK_CUR);
					fQuotation = 0;
					continue;
				}
			}
	tag:
			pch = strchr (buf, '/');
			while (pch)
			{
				if (*(pch + 1) == '/')  // "//"
				{
					(*pch) = 0;
					fputs (buf, pfo);
					fputs ("\n", pfo);
					break;
				}
				else if(*(pch + 1) == '*')  // "/*"
				{
					(*pch) = 0;
					fputs (buf, pfo);
					fsetpos (pfi, &pos);  //maybe the "*/" is in the same line
					fseek (pfi, strlen (buf) + 2, SEEK_CUR);
					fBlock = 1;
					break;
				}
				else
					pch = strchr (pch + 1, '/');  // other "/x"s
			}
			if (pch == NULL)  //no "//" or "/*" in this line, it has its '\n'
				fputs (buf, pfo);
		}
		else
		{
			pch = strchr (buf, '*');
			while (pch)
			{
				if (*(pch + 1) == '/')  // "*/"
				{
					fsetpos (pfi, &pos);  //maybe the "/*" is in the same line
					fseek (pfi, strlen (buf) - strlen (pch + 2), SEEK_CUR);
					fBlock = 0;
					break;
				}
				else
					pch = strchr (pch + 1, '*');  // other "*x"s
			}
			if (pch == NULL)
				fputs ("\n", pfo);
		}
	}

	fclose (pfi);
	fclose (pfo);

	return 0;
}
