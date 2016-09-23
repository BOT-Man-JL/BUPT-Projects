
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int IsPrime (unsigned long i);
unsigned long mexp (unsigned long b, unsigned int n, unsigned long m);
long g_x, g_y;
void extend_Eulid (unsigned long a, unsigned long b);
void longTostr (unsigned long n, char *str);

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

void fnDecryption ()
{
	FILE *pf, *pfo;
	unsigned int ch, i;
	unsigned long d, p, q, n, e, m, c;
	char str[9];

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

	fscanf (pf, "%d%d", &n, &e);
	for (p = 2; p < sqrt(n); p++)
	{
		if (!IsPrime (p)) continue;
		q = n / p;
		if (p * q == n) break;
	}

	if (e > (p - 1) * (q - 1))
	{
		extend_Eulid (e, (p - 1) * (q - 1));
		d = g_x;
	}
	else
	{
		extend_Eulid ((p - 1) * (q - 1), e);
		d = g_y;
	}
	printf ("%d\n", d);
	fprintf (pfo, "%d\n", d);

	fgetc (pf);
	ch = fgetc (pf);
	i = 0;
	while (ch != EOF)
	{
		str[i++] = ch;
		if (i == 8)
		{
			str[i] = 0;
			i = 0;
			c = atol (str);
			m = mexp (c, d, n);
			longTostr (m, str);
			printf ("%08d %08d ", c, m);
			printf ("%s\n", str);
			fprintf (pfo, "%s", str);
		}
		ch = fgetc (pf);
	}

	fclose (pfo);
	fclose (pf);
}

int main ()
{
	fnDecryption ();
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

void extend_Eulid (unsigned long a, unsigned long b)
{
	unsigned long t;

	if (b == 0)
	{
		g_x = 1;
		g_y = 0;
		return;
	}

	extend_Eulid (b, a % b);
	t = g_x;
	g_x = g_y;
	g_y = t - a / b * g_y;
	return;
}

void longTostr (unsigned long n, char *str)
{
	int i;

	str[4] = 0;
	for (i = 3; i >= 0; i--)
	{
		str[i] = n % 100 + 'A';
		n /= 100;
	}
}
