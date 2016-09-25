#include <stdio.h>
#include <string.h>

int InfixToPostfix (const char *infix,
					char *postfix,
					size_t *topP)
{
	size_t len = strlen (infix);
	char stackOp[1024];
	size_t topOp = 0;
	for (size_t i = 0; i < len; i++)
	{
		// Test:
		// 80*3-200+((3-2)*500/10)-2
		switch (infix[i])
		{
		case '=':
			if (i + 1 != len)
				return 0;
			break;

		case '+':
		case '-':
			// Pop All
			for (; topOp > 0 &&
				 stackOp[topOp - 1] != '('; topOp--)
				postfix[(*topP)++] = stackOp[topOp - 1];
			// Push
			stackOp[topOp] = infix[i];
			topOp++;
			break;

		case '*':
		case '/':
			// Pop All
			for (; topOp > 0 &&
				 stackOp[topOp - 1] != '+' &&
				 stackOp[topOp - 1] != '-' &&
				 stackOp[topOp - 1] != '('; topOp--)
				postfix[(*topP)++] = stackOp[topOp - 1];
			// Push
			stackOp[topOp] = infix[i];
			topOp++;
			break;

		case '(':
			// Push
			stackOp[topOp] = infix[i];
			topOp++;
			break;

		case ')':
			// Pop until '('
			for (; topOp > 0 && stackOp[topOp - 1] != '('; topOp--)
				postfix[(*topP)++] = stackOp[topOp - 1];

			// Clear '('
			if (topOp == 0)
				return 0;
			topOp--;
			break;

		default:
			// Cut through
			if (infix[i] >= '0' && infix[i] <= '9')
				postfix[(*topP)++] = infix[i];
			// Seg
			// Todo: Error Detection
			if ((i + 1 < len && !(infix[i + 1] >= '0' && infix[i + 1] <= '9')) ||
				(i + 1 == len))
				postfix[(*topP)++] = ' ';
			break;
		}
	}

	// Pop All
	for (; topOp > 0; topOp--)
		postfix[(*topP)++] = stackOp[topOp - 1];

	return 1;
}

int chToint (char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else
		return -1;
}

int CalcPostfix (const char *postfix,
				 int *result)
{
	size_t len = strlen (postfix);
	int stackNum[16];
	size_t top = 0;
	int numRead = 0;

	for (size_t i = 0; i < len; i++)
	{
		switch (postfix[i])
		{
		case '+':
			if (top < 2)
				return 0;
			stackNum[top - 2] = stackNum[top - 2] + stackNum[top - 1];
			top--;
			break;

		case '-':
			if (top < 2)
				return 0;
			stackNum[top - 2] = stackNum[top - 2] - stackNum[top - 1];
			top--;
			break;

		case '*':
			if (top < 2)
				return 0;
			stackNum[top - 2] = stackNum[top - 2] * stackNum[top - 1];
			top--;
			break;

		case '/':
			if (top < 2)
				return 0;
			if (stackNum[top - 1] == 0)
			{
				printf ("Divide 0\n");
				return 0;
			}
			stackNum[top - 2] = stackNum[top - 2] / stackNum[top - 1];
			top--;
			break;

		case ' ':
			stackNum[top++] = numRead;
			numRead = 0;
			break;

		default:
			if (postfix[i] >= '0' && postfix[i] <= '9')
				numRead = numRead * 10 + chToint (postfix[i]);
			else
				return 0;
			break;
		}
	}

	if (top != 1)
		return 0;

	*result = stackNum[0];
	return 1;
}

int main (int argc, char *argv[])
{
	char infix[1024];
	char postfix[1024];
	size_t top = 0;

	printf ("Input Infix (Ended with \\n):\n");
	scanf ("%s", infix);

	if (!InfixToPostfix (infix, postfix, &top))
	{
		printf ("Error Syntax\n");
		return 1;
	}

	postfix[top] = 0;
	printf ("Postfix: %s\n", postfix);

	int result;
	if (!CalcPostfix (postfix, &result))
	{
		printf ("Error Postfix\n");
		return 1;
	}

	printf ("Result: %d\n", result);

	getchar ();
	getchar ();
	return 0;
}
