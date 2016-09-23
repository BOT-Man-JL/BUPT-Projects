#include <stdio.h>
#include <stdlib.h>

typedef struct _List
{
	int n;
	struct _List *next;
} List;

List *BuildList ()
{
	int x;
	List *head = NULL, *cur, *pt;
	
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
	return head;
}

void FreeList (List *head)
{
	List *cur, *pt;
	
	for (cur = head; cur != NULL; )
	{
		pt = cur;
		cur = cur->next;
		free (pt);
	}
}

int main ()
{
	int x;
	List *pha = NULL, *phb = NULL, *pca, *pcb, *pt;
	
	printf ("Please input List A (ended by -1):");
	pha = BuildList ();
	printf ("Please input List B (ended by -1):");
	phb = BuildList ();
	
	for (pt = pha; pt != NULL; pt = pt->next)
	{
		for (pca = pt, pcb = phb; pca && pcb; pca = pca->next, pcb = pcb->next)
			if (pca->n != pcb->n) break;
		if (pca && pcb) continue;
		else
		{
			printf ("List B is a sub sequence of List A");
			break;
		}
	}
	if (pt == NULL) printf ("List B is not a sub sequence of List A");
	
	FreeList (pha);
	FreeList (phb);
	
	return 0;
}

