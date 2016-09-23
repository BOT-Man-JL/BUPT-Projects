#include <stdio.h>
#include <stdlib.h>

typedef struct _List
{
	int n;
	struct _List *next;
} List;

int main ()
{
	int x;
	List *head = NULL, *cur, *pre, *pt;
	
	printf ("Please input a series of integers:");
	
	//establish the list
	scanf ("%d", &x);
	while (x != -1)
	{
		if (head != NULL)
		{
			cur->next = (List *) malloc (sizeof (List));
			cur = cur->next;
			cur->n = x;
		}
		else
		{
			head = (List *) malloc (sizeof (List));
			head->n = x;
			cur = head;
		}
		scanf ("%d", &x);
	}
	if (head != NULL)
		cur->next = NULL;
	
	//print the list
	if (head == NULL)
	{
		printf ("There is no item in the list.\n");
		return 0;
	}
	else
	{
		printf ("The list is:");
		for (cur = head; cur != NULL; cur = cur->next)
			printf ("%d ", cur->n);
		printf ("\n");
	}		
	
	//delete x
	printf ("Please input an integer you want to delete:");
	scanf ("%d", &x);
	for (cur = pre = head; cur != NULL; )
	{
		if (cur->n == x)
		{
			if (cur == head)
			{
				pt = head;
				pre = head = cur->next;
				cur = cur->next;
				free (pt);
			}
			else
			{
				pre->next = cur->next;
				pt = cur;
				cur = cur->next;
				free (pt);
			}
		}
		else
		{
			if (cur != head)
			{
				pre = cur;
				cur = cur->next;
			}
			else
				cur = cur->next;
		}
	}
	
	//print the list
	if (head == NULL)
		printf ("There is no item in the list.\n");
	else
	{
		printf ("The new list is:");
		for (cur = head; cur != NULL; cur = cur->next)
			printf ("%d ", cur->n);
	}
	
	//free the mem.
	for (cur = head; cur != NULL; )
	{
		pt = cur;
		cur = cur->next;
		free (pt);
	}
	
	return 0;
}

