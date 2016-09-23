
#include <stdio.h>
#include <math.h>
#include <ctype.h>

int IsPrime (unsigned long i);
unsigned long gcd (unsigned long i, unsigned long j);
unsigned long mexp (unsigned long b, unsigned int n, unsigned long m);
unsigned long strTolong (const char *str);

void fnEncryption ()
{
	FILE *pf, *pfo;
	unsigned int ch, i;
	unsigned long p, q, n, e, m, c;
	char str[5];

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

	fscanf (pf, "%d%d%d%d", &p, &q, &n, &e);
	//printf ("%d %d %d %d", p, q, n, e);
	if (p * q != n || !IsPrime (p) || !IsPrime (q) ||
		gcd ((p - 1) * (q - 1), e) != 1 || n <=25252525)
	{
		printf ("Invalid data\n");
		fclose (pfo);
		fclose (pf);
		return;
	}

	ch = fgetc (pf);
	i = 0;
	while (ch != EOF)
	{
		if (isalpha (ch))
		{
			str[i] = tolower(ch);
			i++;
		}
		if (i == 4)
		{
			str[i] = 0;
			i = 0;
			m = strTolong (str);
			c = mexp (m, e, n);
			fprintf (pfo, "%08d", c);
			printf ("%08d\n", c);
		}
		ch = fgetc (pf);
	}
	if (i != 0)
	{
		while (i < 4)
			str[i++] = 'z';
		str[i] = 0;
		m = strTolong (str);
		c = mexp (m, e, n);
		fprintf (pfo, "%08d", c);
		printf ("%08d\n", c);
	}

	fclose (pfo);
	fclose (pf);
}

int main ()
{
	fnEncryption ();
	return 0;
}

int IsPrime (unsigned long n)
{
	int i;

	if (n == 2 || n == 1) return 0;
	if (n % 2 == 0) return 0;

	for (i = 3; i <= sqrtl (n); i += 2)
		if (n % i == 0) return 0;

	return 1;
}

unsigned long gcd (unsigned long i, unsigned long j)
{
	unsigned int r;

	if (i < j)
	{
         r = i;
         i = j;
         j = r;
    }

    while(j > 0)
    {
         r = i % j;
         i = j;
         j = r;
    }
    return i;
}

unsigned long mexp (unsigned long b, unsigned int n, unsigned long m)
{
	unsigned long long x = 1;
	unsigned long long p = b % m;

	while (n)
	{
		if (n & 1)
			x = ((unsigned long long) (x % m) * (unsigned long long) (p % m)) % (unsigned long long) m;
		p = ((unsigned long long) (p % m) * (unsigned long long) (p % m)) % (unsigned long long) m;
		n >>= 1;
	}
	return (unsigned long) x;
}

unsigned long strTolong (const char *str)
{
	unsigned long ret = 0;
	unsigned int i;

	for (i = 0; i < 4; i++)
	{
		ret += (str[i] - 'a');
		if (i != 3) ret *= 100;
	}

	return ret;
}
