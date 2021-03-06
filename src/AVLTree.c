/* Shawn Hustins
 * ID: 0884015
 */

#include "AVLTree.h"

/* Public: ********************************************************************/

int newTree(Tree * tree, size_t dataSize, Comparator comparator, 
										Destructor destructor) {
	
	check(tree, "Init: No tree specified.");
	check(dataSize > 0, "Init: Invalid data size specified.");
	
	tree->dataSize = dataSize;
	tree->root = NULL;
	tree->destructor = destructor;
	tree->comparator = comparator;
	tree->size = 0;
	
	return 0;
	
error:
	return -1;
}

	
int destroyTree(Tree * tree) {

	check(tree, "Destroy: No tree specified.");
	check(tree->root, "Destroy: Tree is empty.");
	
	destroyNode(tree->root, tree->destructor);
	
	return 0;
	
error:
	return -1;
}


int insertNode(Tree * tree, void * data) {

	check(tree, "Insert: No tree specified.");
	
	if (!tree->root) {
		tree->root = newNode(data, tree->dataSize);
		check(tree->root, "Insert: Failed insertion.");
		++tree->size;
		return 0;
	} 
	int rc = insert(tree->root, data, tree);
	if (rc >= 0) ++tree->size;
	
	return rc;
	
error:
	return -1;
}


int removeNode(Tree * tree, void * key) {
	
	check(tree, "Remove: No tree specified.");
	check(tree->root, "Remove: Tree is empty.");
	
	int rc = findRemove(tree->root, key, tree);
	check(rc != -1, "Remove: Key not found.");
	
	--tree->size;
	if (tree->size == 0) tree->root = NULL;
	
	return 0;
		  
error:
	return -1;
}


void * retrieveNode(Tree * tree, void * key) {
	
	check(tree, "Retrieve: No tree specified.");
	check(tree->root, "Retrieve: Tree is empty.");
	
	return retrieve(tree->root, key, tree);
	
error:
	return NULL;
}


int printTree(Tree * tree, Printer printer) {
	
	check(tree, "Print: No tree specified.");
	check(tree->root, "Print: Tree is empty.");
	check(printer, "Print: No print method specified.");
	
	printNode(tree->root, printer, 0);
	
	return 0;
	
error:
	return -1;
}

/* Private: *******************************************************************/

void destroyNode(Node * node, Destructor destructor) {
	
	if (!node) return;
	
	destroyNode(node->left, destructor);
	destroyNode(node->right, destructor);
	
	if (destructor)
		destructor(node->data);

	free(node->data);
	
	free(node);
}


Node * newNode(void * data, size_t dataSize) {
	
	Node * new = calloc(1, sizeof(Node));
	check(new, "Memory error.");
	
	new->data = calloc(1, dataSize);
	check(new->data, "Memory error.");
	
	memcpy(new->data, data, dataSize);
	
	return new;
	
error:
	return NULL;
}


int insert(Node * node, void * data, Tree * tree) {
	
	int cmp = tree->comparator(data, node->data); // compare keys
	check(cmp != 0, "Duplicate key exists."); // equal key
	int rc;
	
	if (cmp > 0) { // key is greater, move right
		
		if (!node->right) {
			node->right = newNode(data, tree->dataSize);
			check(node->right, "Failed insertion.");
			node->right->parent = node;
			rc = 1;
		} else {
			rc = insert(node->right, data, tree);
		}
		
	} else { // key is less, move left
		
		if (!node->left) {
			node->left = newNode(data, tree->dataSize);
			check(node->left, "Failed insertion.");
			node->left->parent = node;
			rc = 1;
		} else {
			rc = insert(node->left, data, tree);
		}
	}
	
	if (rc >= 0) {
		if (rc > node->height) { // if height didn't change, no balance check
			node->height = rc;
			keepBalanced(node, data, tree, 0);
		}
		return rc + 1; // on successful entry, return height in path
	} else {
		return -1;
	}
	
error:
	return -1;
}


int findRemove(Node * node, void * key, Tree * tree) {
	
	int cmp = tree->comparator(key, node->data);
	
	if (cmp > 0) {
		
		if (!node->right) {
			return -1;
		} else {
			return findRemove(node->right, key, tree);
		} 
		
	} else if (cmp < 0) {
		
		if (!node->left) {
			return -1;
		} else {
			return findRemove(node->left, key, tree);
		}
		
	} else {
		
		if (!node->deleted) {
			node->deleted = 1;
			return 0;
		} else {
			return -1;
		}
	}	
}
	
	
void * retrieve(Node * node, void * key, Tree * tree) {
	
	int cmp = tree->comparator(key, node->data);
	
	if (cmp > 0) {
		if (!node->right) {
			return NULL;
		} else {
			return retrieve(node->right, key, tree);
		}
	} else if (cmp < 0) {
		if (!node->left) {
			return NULL;
		} else {
			return retrieve(node->left, key, tree);
		}
	} else {
		if (!node->deleted) {
			return node->data;
		} else {
			return NULL;
		}
	}
}


void printNode(Node * node, Printer printer, int depth) {
	
	if (!node) return;
	
	printNode(node->left, printer, depth + 1);
	
	for (int i = 0; i < depth; ++i)
		printf("            ");
	
	printf("-(%d)-|", node->height);
	
	if (!node->deleted) {
		printer(node->data);
	} else {
		printer(NULL);
	}
	
	putchar('\n');
	
	printNode(node->right, printer, depth + 1);
}

/* AVL Tree Balancing Procedures (Private): ***********************************/

void keepBalanced(Node * node, void * data, Tree * tree, int flip) {
	
	int cmp = checkBalance(node); // compare heights
	if (cmp > 1) { // left overflow
		if (tree->comparator(node->left->data, data) > 0) {
			if (flip) {
				doubleRotateFromLeft(node, tree); // double = inside rotation
			} else {
				rotateFromLeft(node, tree); // single = outside rotation
			}
		} else {
			if (flip) {
				rotateFromLeft(node, tree);
			} else {
				doubleRotateFromLeft(node, tree);
			}
		}
	} else if (cmp < -1) { // right overflow
		if (tree->comparator(node->right->data, data) < 0) {
			if (flip) {
				doubleRotateFromRight(node, tree);
			} else {
				rotateFromRight(node, tree); 
			}
		} else {
			if (flip) {
				rotateFromRight(node, tree);
			} else {
				doubleRotateFromRight(node, tree);
			}
		}
	}
	adjustHeight(node);
}	


void rotateFromLeft(Node * oldRoot, Tree * tree) {
	
	Node * newRoot = oldRoot->left;
	
	if(!newRoot) return;
	
	if (newRoot->right)
		newRoot->right->parent = oldRoot;
	
	oldRoot->left = newRoot->right;
	
	newRoot->right = oldRoot;
	
	if (oldRoot->parent == NULL) {
		tree->root = newRoot;
	} else {
		if (oldRoot->parent->left == oldRoot) {
			oldRoot->parent->left = newRoot;
		} else {
			oldRoot->parent->right = newRoot;
		}
	}
	
	newRoot->parent = oldRoot->parent;
	oldRoot->parent = newRoot;
	
	adjustHeight(oldRoot);
	adjustHeight(newRoot);
	
	return;
}


void doubleRotateFromLeft(Node * oldRoot, Tree * tree) {
	
	rotateFromRight(oldRoot->left, tree);
	rotateFromLeft(oldRoot, tree);
}


void rotateFromRight(Node * oldRoot, Tree * tree) {
	
	Node * newRoot = oldRoot->right;
	
	if (!newRoot) return;
	
	if (newRoot->left)
		newRoot->left->parent = oldRoot;
	
	oldRoot->right = newRoot->left;
	
	newRoot->left = oldRoot;
	
	
	if (oldRoot->parent == NULL) {
		tree->root = newRoot;
	} else {
		if (oldRoot->parent->left == oldRoot) {
			oldRoot->parent->left = newRoot;
		} else {
			oldRoot->parent->right = newRoot;
		}
	}
	
	newRoot->parent = oldRoot->parent;
	oldRoot->parent = newRoot;
	
	adjustHeight(oldRoot);
	adjustHeight(newRoot);
	
	return;
}


void doubleRotateFromRight(Node * oldRoot, Tree * tree) {
	
	rotateFromLeft(oldRoot->right, tree);
	rotateFromRight(oldRoot, tree);
}


int checkBalance(Node * node) {
	
	return (node->left ? node->left->height : -1)
			- (node->right ? node->right->height : -1);
}


void adjustHeight(Node * node) {
	
	node->height = node->left ? node->left->height : -1;
	if ((node->right ? node->right->height : -1) > node->height)
		node->height = node->right->height;
	++node->height;
}