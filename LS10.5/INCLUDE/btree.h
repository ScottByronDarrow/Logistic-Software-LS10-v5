#include	<malloc.h>

/* add node to tree		*/
BTREE	*add_node(BTREE *tptr, int (*comp)(BTREE *), int (*init)(BTREE *));
/* find node in tree		*/
BTREE	*find_node (BTREE *tptr, int (*comp)(BTREE *));
BTREE	*tree_alloc (int (*init)(BTREE *));
void	init_btree(BTREE *psUnused);
void	print_tree(BTREE *tptr, int (*node)(BTREE *));

/*=======================================
| Initialise btree			|
=======================================*/
void
init_btree(BTREE *psUnused)
{
	tree_head = TNUL;
}

/*=======================================
| Print btree				|
=======================================*/
void
print_tree(BTREE *tptr, int (*node)(BTREE *))
{
	/*-------------------------------
	| Search Left Subtree First	|
	-------------------------------*/
	if (tptr->_left != TNUL)
		print_tree(tptr->_left,node);

	/*---------------
	| Print Node	|
	---------------*/
	(*node)(tptr);

	/*-------------------------------
	| Search Right Subtree		|
	-------------------------------*/
	if (tptr->_right != TNUL)
		print_tree(tptr->_right,node);
}

/*=======================================
| Recursively search tree from tptr	|
| if tree is empty add node at root.	|
| else add node appropriately.		|
=======================================*/
BTREE	*add_node(BTREE *tptr, int (*comp)(BTREE *), int (*init)(BTREE *))
{
	int	i;
	BTREE	*xptr;

	/*---------------
	| Tree is Null	|
	---------------*/
	if (tptr == TNUL)
	{
		xptr = tree_alloc(init);
		errno = (xptr == TNUL) ? 12 : 0;
		return(xptr);
	}

	/*-------------------------------
	| Use Comparison Routine.	|
	-------------------------------*/
	i = (*comp)(tptr);
	/*---------------
	| Exact Match	|
	---------------*/
	if (i == 0)
		return(tptr);

	/*-----------------------
	| Check Left Subtree	|
	-----------------------*/
	if (i < 0)
	{
		/*-----------------------
		| At edge of Tree.	|
		-----------------------*/
		if (tptr->_left == TNUL)
		{
			tptr->_left = tree_alloc(init);
			errno = (tptr->_left == TNUL) ? 12 : 0;
			return(tptr->_left);
		}
		/*-----------------------
		| Continue Recursing	|
		-----------------------*/
		return(add_node(tptr->_left,comp,init));
	}
	else
	{
		if (tptr->_right == TNUL)
		{
			tptr->_right = tree_alloc(init);
			errno = (tptr->_right == TNUL) ? 12 : 0;
			return(tptr->_right);
		}
		return(add_node(tptr->_right,comp,init));
	}
}

/*=======================================
| Recursively search tree from tptr	|
| If valid node is found then return	|
| pointer to node, else TNUL.		|
=======================================*/
BTREE	*find_node (BTREE *tptr, int (*comp)(BTREE *))
{
	int	i;

	/*---------------
	| Tree is Null	|
	---------------*/
	if (tptr == TNUL)
		return(TNUL);

	/*-------------------------------
	| Use Comparison Routine.	|
	-------------------------------*/
	i = (*comp)(tptr);
	/*---------------
	| Exact Match	|
	---------------*/
	if (i == 0)
		return(tptr);

	/*-----------------------
	| Check Left Subtree	|
	-----------------------*/
	if (i < 0)
		return(find_node(tptr->_left,comp));
	else
		return(find_node(tptr->_right,comp));
}

/*=======================================
| Allocate tree node an initialise it	|
=======================================*/
BTREE	*tree_alloc (int (*init)(BTREE *))
{
	BTREE	*tptr;

	tptr = (struct _btree *) malloc((unsigned) sizeof(struct _btree));
	if (tptr != TNUL)
	{
		(*init)(tptr);
		tptr->_left = TNUL;
		tptr->_right = TNUL;
	}
	return(tptr);
}
