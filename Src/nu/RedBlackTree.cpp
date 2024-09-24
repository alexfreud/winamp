#include "RedBlackTree.h"
#include <limits>
#include "PtrList.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

void RedBlackTreeIterator::next()
{
	if (node)
		node = tree->GetSuccessorOf(node);
}

bool RedBlackTreeIterator::get(RedBlackTreeIterator::val_t *val)
{
	if (node)
	{
		*val = node->GetEntry();
		return true;
	}
	return false;
}

RedBlackTreeNode::RedBlackTreeNode()
{
}

RedBlackTreeNode::RedBlackTreeNode(key_t _key, val_t newEntry)
		: storedEntry(newEntry) , key(_key)
{
}

RedBlackTreeNode::~RedBlackTreeNode()
{
}

RedBlackTreeNode::val_t RedBlackTreeNode::GetEntry() const
{
	return storedEntry;
}


RedBlackTree::RedBlackTree()
{
	nil = new RedBlackTreeNode;
	nil->left = nil->right = nil->parent = nil;
	nil->red = 0;
	nil->key = std::numeric_limits<key_t>::min();
	nil->storedEntry = NULL;

	root = new RedBlackTreeNode;
	root->parent = root->left = root->right = nil;
	root->key = std::numeric_limits<key_t>::max();
	root->red=0;
	root->storedEntry = NULL;

	numElements = 0;
}
RedBlackTreeIterator RedBlackTree::end()
{
	RedBlackTreeIterator temp;
	return temp;
}

RedBlackTreeIterator RedBlackTree::begin()
{
	return RedBlackTreeIterator(root->left, this);
}
/***********************************************************************/
/*  FUNCTION:  LeftRotate */
/**/
/*  INPUTS:  the node to rotate on */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input: this, x */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly.  */
/***********************************************************************/

void RedBlackTree::LeftRotate(RedBlackTreeNode* x)
{
	RedBlackTreeNode* y;

	/*  I originally wrote this function to use the sentinel for */
	/*  nil to avoid checking for nil.  However this introduces a */
	/*  very subtle bug because sometimes this function modifies */
	/*  the parent pointer of nil.  This can be a problem if a */
	/*  function which calls LeftRotate also uses the nil sentinel */
	/*  and expects the nil sentinel's parent pointer to be unchanged */
	/*  after calling this function.  For example, when DeleteFixUP */
	/*  calls LeftRotate it expects the parent pointer of nil to be */
	/*  unchanged. */

	y=x->right;
	x->right=y->left;

	if (y->left != nil) y->left->parent=x; /* used to use sentinel here */
	/* and do an unconditional assignment instead of testing for nil */

	y->parent=x->parent;

	/* instead of checking if x->parent is the root as in the book, we */
	/* count on the root sentinel to implicitly take care of this case */
	if (x == x->parent->left)
	{
		x->parent->left=y;
	}
	else
	{
		x->parent->right=y;
	}
	y->left=x;
	x->parent=y;

#ifdef CHECK_RB_TREE_ASSUMPTIONS
	CheckAssumptions();
#elif defined(DEBUG_ASSERT)
	Assert(!nil->red,"nil not red in RedBlackTree::LeftRotate");
#endif
}

/***********************************************************************/
/*  FUNCTION:  RighttRotate */
/**/
/*  INPUTS:  node to rotate on */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input?: this, y */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly.  */
/***********************************************************************/

void RedBlackTree::RightRotate(RedBlackTreeNode* y)
{
	RedBlackTreeNode* x;

	/*  I originally wrote this function to use the sentinel for */
	/*  nil to avoid checking for nil.  However this introduces a */
	/*  very subtle bug because sometimes this function modifies */
	/*  the parent pointer of nil.  This can be a problem if a */
	/*  function which calls LeftRotate also uses the nil sentinel */
	/*  and expects the nil sentinel's parent pointer to be unchanged */
	/*  after calling this function.  For example, when DeleteFixUP */
	/*  calls LeftRotate it expects the parent pointer of nil to be */
	/*  unchanged. */

	x=y->left;
	y->left=x->right;

	if (nil != x->right)  x->right->parent=y; /*used to use sentinel here */
	/* and do an unconditional assignment instead of testing for nil */

	/* instead of checking if x->parent is the root as in the book, we */
	/* count on the root sentinel to implicitly take care of this case */
	x->parent=y->parent;
	if (y == y->parent->left)
	{
		y->parent->left=x;
	}
	else
	{
		y->parent->right=x;
	}
	x->right=y;
	y->parent=x;

#ifdef CHECK_RB_TREE_ASSUMPTIONS
	CheckAssumptions();
#elif defined(DEBUG_ASSERT)
	Assert(!nil->red,"nil not red in RedBlackTree::RightRotate");
#endif
}

/***********************************************************************/
/*  FUNCTION:  TreeInsertHelp  */
/**/
/*  INPUTS:  z is the node to insert */
/**/
/*  OUTPUT:  none */
/**/
/*  Modifies Input:  this, z */
/**/
/*  EFFECTS:  Inserts z into the tree as if it were a regular binary tree */
/*            using the algorithm described in _Introduction_To_Algorithms_ */
/*            by Cormen et al.  This funciton is only intended to be called */
/*            by the Insert function and not by the user */
/***********************************************************************/

void RedBlackTree::TreeInsertHelp(RedBlackTreeNode* z)
{
	/*  This function should only be called by RedBlackTree::Insert */
	RedBlackTreeNode* x;
	RedBlackTreeNode* y;

	z->left=z->right=nil;
	y=root;
	x=root->left;
	while (x != nil)
	{
		y=x;
		if (x->key > z->key)
		{
			x=x->left;
		}
		else   /* x->key <= z->key */
		{
			x=x->right;
		}
	}
	z->parent=y;
	if ((y == root) ||
	    (y->key > z->key))
	{
		y->left=z;
	}
	else
	{
		y->right=z;
	}

#if defined(DEBUG_ASSERT)
	Assert(!nil->red,"nil not red in RedBlackTree::TreeInsertHelp");
#endif
}

RedBlackTreeIterator RedBlackTree::Search(key_t key)
{
	RedBlackTreeNode* x;

	x=root->left;
	while (x != nil)
	{
		if (x->key > key)
		{
			x=x->left;
		}
		else if (x->key < key)
		{
			x=x->right;
		}
		else
		{
			return RedBlackTreeIterator(x, this);
		}
	}
	return end();
}

/*  Before calling InsertNode  the node x should have its key set */

/***********************************************************************/
/*  FUNCTION:  InsertNode */
/**/
/*  INPUTS:  newEntry is the entry to insert*/
/**/
/*  OUTPUT:  This function returns a pointer to the newly inserted node */
/*           which is guarunteed to be valid until this node is deleted. */
/*           What this means is if another data structure stores this */
/*           pointer then the tree does not need to be searched when this */
/*           is to be deleted. */
/**/
/*  Modifies Input: tree */
/**/
/*  EFFECTS:  Creates a node node which contains the appropriate key and */
/*            info pointers and inserts it into the tree. */
/***********************************************************************/

RedBlackTreeIterator RedBlackTree::Insert(key_t _key, val_t newEntry)
{
	RedBlackTreeNode * y;
	RedBlackTreeNode * x;
	RedBlackTreeNode * newNode;

	x = new RedBlackTreeNode(_key, newEntry);
	TreeInsertHelp(x);
	newNode = x;
	x->red=1;
	while (x->parent->red)  /* use sentinel instead of checking for root */
	{
		if (x->parent == x->parent->parent->left)
		{
			y=x->parent->parent->right;
			if (y->red)
			{
				x->parent->red=0;
				y->red=0;
				x->parent->parent->red=1;
				x=x->parent->parent;
			}
			else
			{
				if (x == x->parent->right)
				{
					x=x->parent;
					LeftRotate(x);
				}
				x->parent->red=0;
				x->parent->parent->red=1;
				RightRotate(x->parent->parent);
			}
		}
		else   /* case for x->parent == x->parent->parent->right */
		{
			/* this part is just like the section above with */
			/* left and right interchanged */
			y=x->parent->parent->left;
			if (y->red)
			{
				x->parent->red=0;
				y->red=0;
				x->parent->parent->red=1;
				x=x->parent->parent;
			}
			else
			{
				if (x == x->parent->left)
				{
					x=x->parent;
					RightRotate(x);
				}
				x->parent->red=0;
				x->parent->parent->red=1;
				LeftRotate(x->parent->parent);
			}
		}
	}
	root->left->red=0;
	numElements++;
	return RedBlackTreeIterator(newNode, this);

#ifdef CHECK_RB_TREE_ASSUMPTIONS
	CheckAssumptions();
#elif defined(DEBUG_ASSERT)
	Assert(!nil->red,"nil not red in RedBlackTree::Insert");
	Assert(!root->red,"root not red in RedBlackTree::Insert");
#endif
}

RedBlackTree::~RedBlackTree()
{
	RedBlackTreeNode * x = root->left;
	nu::PtrList<RedBlackTreeNode> stuffToFree;

	if (x != nil)
	{
		if (x->left != nil)
		{
			stuffToFree.push_back(x->left);
		}
		if (x->right != nil)
		{
			stuffToFree.push_back(x->right);
		}
		// delete x->storedEntry;
		delete x;
		while (!stuffToFree.empty())
		{
			x = stuffToFree.back();
			stuffToFree.pop_back();
			if (x->left != nil)
			{
				stuffToFree.push_back(x->left);
			}
			if (x->right != nil)
			{
				stuffToFree.push_back(x->right);
			}
			// delete x->storedEntry;
			delete x;
		}
	}
	delete nil;
	delete root;
}

void RedBlackTree::DeleteFixUp(RedBlackTreeNode* x)
{
	RedBlackTreeNode * w;
	RedBlackTreeNode * rootLeft = root->left;

	while ((!x->red) && (rootLeft != x))
	{
		if (x == x->parent->left)
		{
			w=x->parent->right;
			if (w->red)
			{
				w->red=0;
				x->parent->red=1;
				LeftRotate(x->parent);
				w=x->parent->right;
			}
			if ((!w->right->red) && (!w->left->red))
			{
				w->red=1;
				x=x->parent;
			}
			else
			{
				if (!w->right->red)
				{
					w->left->red=0;
					w->red=1;
					RightRotate(w);
					w=x->parent->right;
				}
				w->red=x->parent->red;
				x->parent->red=0;
				w->right->red=0;
				LeftRotate(x->parent);
				x=rootLeft; /* this is to exit while loop */
			}
		}
		else   /* the code below is has left and right switched from above */
		{
			w=x->parent->left;
			if (w->red)
			{
				w->red=0;
				x->parent->red=1;
				RightRotate(x->parent);
				w=x->parent->left;
			}
			if ((!w->right->red) && (!w->left->red))
			{
				w->red=1;
				x=x->parent;
			}
			else
			{
				if (!w->left->red)
				{
					w->right->red=0;
					w->red=1;
					LeftRotate(w);
					w=x->parent->left;
				}
				w->red=x->parent->red;
				x->parent->red=0;
				w->left->red=0;
				RightRotate(x->parent);
				x=rootLeft; /* this is to exit while loop */
			}
		}
	}
	x->red=0;

#ifdef CHECK_RB_TREE_ASSUMPTIONS
	CheckAssumptions();
#elif defined(DEBUG_ASSERT)
	Assert(!nil->red,"nil not black in RedBlackTree::DeleteFixUp");
#endif
}
void RedBlackTree::Delete(RedBlackTree::key_t key)
{
	RedBlackTreeIterator itr = Search(key);
	DeleteNode(itr.node);
}

/***********************************************************************/
/*  FUNCTION:  DeleteNode */
/**/
/*    INPUTS:  tree is the tree to delete node z from */
/**/
/*    OUTPUT:  returns the RedBlackEntry stored at deleted node */
/**/
/*    EFFECT:  Deletes z from tree and but don't call destructor */
/**/
/*    Modifies Input:  z */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */
/***********************************************************************/

RedBlackTree::val_t RedBlackTree::DeleteNode(RedBlackTreeNode * z)
{
	RedBlackTreeNode* y;
	RedBlackTreeNode* x;
	val_t returnValue = z->storedEntry;

	y= ((z->left == nil) || (z->right == nil)) ? z : GetSuccessorOf(z);
	x= (y->left == nil) ? y->right : y->left;
	if (root == (x->parent = y->parent))   /* assignment of y->p to x->p is intentional */
	{
		root->left=x;
	}
	else
	{
		if (y == y->parent->left)
		{
			y->parent->left=x;
		}
		else
		{
			y->parent->right=x;
		}
	}
	if (y != z)   /* y should not be nil in this case */
	{

#ifdef DEBUG_ASSERT
		Assert((y!=nil),"y is nil in DeleteNode \n");
#endif
		/* y is the node to splice out and x is its child */

		y->left=z->left;
		y->right=z->right;
		y->parent=z->parent;
		z->left->parent=z->right->parent=y;
		if (z == z->parent->left)
		{
			z->parent->left=y;
		}
		else
		{
			z->parent->right=y;
		}
		if (!(y->red))
		{
			y->red = z->red;
			DeleteFixUp(x);
		}
		else
			y->red = z->red;
		delete z;
#ifdef CHECK_RB_TREE_ASSUMPTIONS
		CheckAssumptions();
#elif defined(DEBUG_ASSERT)
		Assert(!nil->red,"nil not black in RedBlackTree::Delete");
#endif
	}
	else
	{
		if (!(y->red)) DeleteFixUp(x);
		delete y;
#ifdef CHECK_RB_TREE_ASSUMPTIONS
		CheckAssumptions();
#elif defined(DEBUG_ASSERT)
		Assert(!nil->red,"nil not black in RedBlackTree::Delete");
#endif
	}
	numElements--;
	return returnValue;
}

size_t RedBlackTree::size() const
{
	return numElements;
}

/***********************************************************************/
/*  FUNCTION:  GetPredecessorOf  */
/**/
/*    INPUTS:  x is the node to get predecessor of */
/**/
/*    OUTPUT:  This function returns the predecessor of x or NULL if no */
/*             predecessor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/

RedBlackTreeNode *RedBlackTree::GetPredecessorOf(RedBlackTreeNode * x) const
{
	RedBlackTreeNode* y;

	if (nil != (y = x->left))   /* assignment to y is intentional */
	{
		while (y->right != nil)  /* returns the maximum of the left subtree of x */
		{
			y=y->right;
		}
		return(y);
	}
	else
	{
		y=x->parent;
		while (x == y->left)
		{
			if (y == root) return(nil);
			x=y;
			y=y->parent;
		}
		return(y);
	}
}


/***********************************************************************/
/*  FUNCTION:  GetSuccessorOf  */
/**/
/*    INPUTS:  x is the node we want the succesor of */
/**/
/*    OUTPUT:  This function returns the successor of x or NULL if no */
/*             successor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/

RedBlackTreeNode *RedBlackTree::GetSuccessorOf(RedBlackTreeNode * x) const
{
	RedBlackTreeNode* y;

	if (nil != (y = x->right))   /* assignment to y is intentional */
	{
		while (y->left != nil)  /* returns the minium of the right subtree of x */
		{
			y=y->left;
		}
		return(y);
	}
	else
	{
		y=x->parent;
		while (x == y->right)  /* sentinel used instead of checking for nil */
		{
			x=y;
			y=y->parent;
		}
		if (y == root) return(nil);
		return(y);
	}
}
