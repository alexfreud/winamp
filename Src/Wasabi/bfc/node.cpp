#include "precomp_wasabi_bfc.h"
#include "node.h"

// In debug mode, the node baseclass will keep track of the count
// of all nodes created, to help detect leakage of nodes.
#ifdef  _DEBUG
int										Node::count = 0;
#endif//_DEBUG
