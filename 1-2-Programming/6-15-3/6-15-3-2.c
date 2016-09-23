#include <stdio.h>
#include <ctype.h>

//#define ENDLESS

int main ()
{
	int ch;
	enum State
	{
		HEAD,
		BLANK,
		INIT,
		FOLLOW,
		VALID,
		ERR,
		END
	} state = HEAD;

	printf ("Please input a string: ");

	while (state != END)
	{
		if (state != VALID && state != ERR)
			ch = getchar ();
		if (ch == '\n')
			state = ERR;

		switch (state)
		{
		case HEAD:
			if (!isblank (ch))
			{
				if (ch == 'i' && getchar () == 'n')
				{
					if (getchar () == 't')
						state = BLANK;
					else state = ERR;
				}
				else state = ERR;
			}
			break;

		case BLANK:
			if (isblank (ch))
				state = INIT;
			else state = ERR;
			break;

		case INIT:
			if (!isblank (ch))
			{
				if (ch == '_' || isalpha (ch))
					state = FOLLOW;
				else state = ERR;
			}
			break;

		case FOLLOW:
			if (ch == '_' || isalpha (ch) || isdigit (ch))
				break;
			else if (ch == ',')
				state = INIT;
			else if (ch == ';')
				state = VALID;
			else state = ERR;
			break;

		case VALID:
			printf ("The result is: YES\n");
#ifdef ENDLESS
			state = HEAD;
			fflush (stdin);
#else
			state = END;
#endif
			break;

		case ERR:
			printf ("The result is: NO\n");
#ifdef ENDLESS
			state = HEAD;
			fflush (stdin);
#else
			state = END;
#endif
			break;
		}
	}

	return 0;
}

