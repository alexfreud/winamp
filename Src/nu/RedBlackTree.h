#pragma once
// http://web.mit.edu/~emin/www/source_code/cpp_trees/index.html

class RedBlackTreeNode
{
public:
	typedef int key_t;
	typedef void *val_t;
	friend class RedBlackTree;
public:
	RedBlackTreeNode();
	RedBlackTreeNode(key_t, val_t);
	val_t GetEntry() const;
	~RedBlackTreeNode();
protected:
	val_t storedEntry;
	key_t key;
	int red; /* if red=0 then the node is black */
	RedBlackTreeNode *left;
	RedBlackTreeNode *right;
	RedBlackTreeNode *parent;
};
class RedBlackTree;

class RedBlackTreeIterator
{
public:
	friend RedBlackTree;
	typedef int key_t;
	typedef void *val_t;
	RedBlackTreeIterator() : node(0), tree(0) {}
	RedBlackTreeIterator(RedBlackTreeNode *_node, RedBlackTree *_tree) : node(_node), tree(_tree) {}
	void next();
	bool get(val_t *val);
private:
	RedBlackTreeNode *node;
	RedBlackTree *tree;
};

class RedBlackTree
{
public:
	typedef int key_t;
	typedef void *val_t;
public:
	RedBlackTree();
	~RedBlackTree();

	RedBlackTreeIterator end();
	RedBlackTreeIterator Insert(key_t, val_t);
	RedBlackTreeIterator Search(key_t key);
	RedBlackTreeIterator begin();
// semi-public:
	void Delete(key_t key);
	val_t DeleteNode(RedBlackTreeNode *);
	RedBlackTreeNode *GetPredecessorOf(RedBlackTreeNode *) const;
	RedBlackTreeNode *GetSuccessorOf(RedBlackTreeNode *) const;
	size_t size() const;

	//TemplateStack<RedBlackTreeNode *> * Enumerate(int low, int high) ;
protected:
	/*  A sentinel is used for root and for nil.  These sentinels are */
	/*  created when RedBlackTreeCreate is caled.  root->left should always */
	/*  point to the node which is the root of the tree.  nil points to a */
	/*  node which should always be black but has aribtrary children and */
	/*  parent and no key or info.  The point of using these sentinels is so */
	/*  that the root and nil nodes do not require special cases in the code */
	RedBlackTreeNode *root;
	RedBlackTreeNode *nil;
	void LeftRotate(RedBlackTreeNode *);
	void RightRotate(RedBlackTreeNode *);
	void TreeInsertHelp(RedBlackTreeNode *);
	void TreePrintHelper(RedBlackTreeNode *) const;
	void FixUpMaxHigh(RedBlackTreeNode *);
	void DeleteFixUp(RedBlackTreeNode *);
	size_t numElements;
};
