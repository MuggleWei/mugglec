/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "avl_tree.h"
#include <string.h>
#include <stdlib.h>
#include "muggle/c/log/log.h"

#define AVL_TREE_SIDE_LEFT  0
#define AVL_TREE_SIDE_RIGHT 1

static muggle_avl_tree_node_t* muggle_avl_tree_allocate_node(muggle_avl_tree_t *p_avl_tree)
{
	muggle_avl_tree_node_t *node = NULL;
	if (p_avl_tree->pool)
	{
		node = (muggle_avl_tree_node_t*)muggle_memory_pool_alloc(p_avl_tree->pool);
	}
	else
	{
		node = (muggle_avl_tree_node_t*)malloc(sizeof(muggle_avl_tree_node_t));
	}

	if (node)
	{
		memset(node, 0, sizeof(*node));
	}

	return node;
}

static void muggle_avl_tree_erase_node(
	muggle_avl_tree_t *p_avl_tree, muggle_avl_tree_node_t *node, 
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool)
{
	if (node == NULL)
	{
		return;
	}

	if (node->left)
	{
		muggle_avl_tree_erase_node(p_avl_tree, node->left, key_func_free, key_pool, value_func_free, value_pool);
	}

	if (node->right)
	{
		muggle_avl_tree_erase_node(p_avl_tree, node->right, key_func_free, key_pool, value_func_free, value_pool);
	}

	// erase data
	if (node->key)
	{
		if (key_func_free)
		{
			key_func_free(key_pool, node->key);
		}
		node->key = NULL;
	}

	if (node->value)
	{
		if (value_func_free)
		{
			value_func_free(value_pool, node->value);
		}
		node->value = NULL;
	}

	// erase node
	muggle_avl_tree_node_t *parent = node->parent;
	if (parent)
	{
		if (parent->left == node)
		{
			parent->left = NULL;
		}
		else
		{
			parent->right = NULL;
		}
	}

	if (p_avl_tree->pool)
	{
		muggle_memory_pool_free(p_avl_tree->pool, node);
	}
	else
	{
		free(node);
	}
}

// rotate left node
// @return deep change? true - deep decrease, false - deep not change
static bool muggle_avl_tree_rotate_left(muggle_avl_tree_t *p_avl_tree, muggle_avl_tree_node_t *node)
{
	/////////////////////////////////////////////
	// Right-Right
	//
	//      |                        |
	//      X(2)                     Z(0|-1)
	//     / \                      / \
	//    /   \                    /   \
	//  {t1}   Z(1|0)    =>  (0|1)X    {t4}
	//  {  }  / \                / \   {  }
	//       /   \              /   \
	//     {t23} {t4}	      {t1}  {t23}
	//     {   } {  }	      {  }  {   }
	/////////////////////////////////////////////

	muggle_avl_tree_node_t *parent = node->parent;
	muggle_avl_tree_node_t *x = node;
	muggle_avl_tree_node_t *z = x->right;

	MUGGLE_ASSERT(z != NULL);
	MUGGLE_ASSERT(x->balance == 2);
	MUGGLE_ASSERT(z->balance == 1 || z->balance == 0);

	if (parent)
	{
		if (x == parent->left)
		{
			parent->left = z;
		}
		else
		{
			parent->right = z;
		}
	}
	z->parent = parent;

	if (p_avl_tree->root == x)
	{
		p_avl_tree->root = z;
	}

	x->right = z->left;
	if (z->left)
	{
		z->left->parent = x;
	}

	x->parent = z;
	z->left = x;

	if (z->balance == 0)
	{
		x->balance = 1;
		z->balance = -1;
		return false;
	}
	else
	{
		x->balance = 0;
		z->balance = 0;
		return true;
	}
}

// rotate right node
// @return deep change? true - deep decrease, false - deep not change
static bool muggle_avl_tree_rotate_right(muggle_avl_tree_t *p_avl_tree, muggle_avl_tree_node_t *node)
{
	/////////////////////////////////////////////
	// Left-Left
	//
	//          |                    |
	//          X(-2)                Z(0|1)
	//         / \                  / \
	//        /   \				   /   \
	// (-1|0)Z    {t1}    =>   	 {t4}   X(0|-1)
	//      / \   {  }			 {  }  / \
	//     /   \				      /   \
	//   {t4} {t23}				    {t23} {t1}
	//   {  } {   }				    {   } {  }
	/////////////////////////////////////////////

	muggle_avl_tree_node_t *parent = node->parent;
	muggle_avl_tree_node_t *x = node;
	muggle_avl_tree_node_t *z = x->left;

	MUGGLE_ASSERT(z != NULL);
	MUGGLE_ASSERT(x->balance == -2);
	MUGGLE_ASSERT(z->balance == -1 || z->balance == 0);

	if (parent)
	{
		if (x == parent->left)
		{
			parent->left = z;
		}
		else
		{
			parent->right = z;
		}
	}
	z->parent = parent;

	if (p_avl_tree->root == x)
	{
		p_avl_tree->root = z;
	}

	x->left = z->right;
	if (z->right)
	{
		z->right->parent = x;
	}

	x->parent = z;
	z->right = x;

	if (z->balance == 0)
	{
		x->balance = -1;
		z->balance = 1;
		return false;
	}
	else
	{
		x->balance = 0;
		z->balance = 0;
		return true;
	}
}

static void muggle_avl_tree_rotate_right_left(muggle_avl_tree_t *p_avl_tree, muggle_avl_tree_node_t *node)
{
	//////////////////////////////////////////////////////////////////////////////////////////
	// Right-Left
	//
	//        |                            |                             |
	//        X(2)                         X                             Y
	//       /  \                         / \                           / \
	//      /    \                       /   \                         /   \
	//    {t1}    Z (-1)      ==>      {t1}   Y            ==>  (-1|0)X     Z(1|0)
	//    {  }   / \                   {  }  / \                     / \   / \
	//          /   \                       /   \                   /  |   |  \
	// (-1|0|1)Y     {t4}                 {t2}   Z               {t1}{t2} {t3}{t4}
	//        / \    {  }                 {  }  / \              {  }{  } {  }{  }
	//       /   \                             /   \
	//     {t2}  {t3}                        {t3}  {t4}
	//     {  }  {  }                        {  }  {  }
	//////////////////////////////////////////////////////////////////////////////////////////

	muggle_avl_tree_node_t *parent = node->parent;
	muggle_avl_tree_node_t *x = node;
	muggle_avl_tree_node_t *z = x->right;
	MUGGLE_ASSERT(z != NULL);
	muggle_avl_tree_node_t *y = z->left;
	MUGGLE_ASSERT(y != NULL);

	MUGGLE_ASSERT(x->balance == 2);
	MUGGLE_ASSERT(z->balance == -1);

	muggle_avl_tree_node_t *t2 = y->left;
	muggle_avl_tree_node_t *t3 = y->right;

	if (parent)
	{
		if (x == parent->left)
		{
			parent->left = y;
		}
		else
		{
			parent->right = y;
		}
	}
	y->parent = parent;

	if (p_avl_tree->root == x)
	{
		p_avl_tree->root = y;
	}

	x->right = t2;
	if (t2)
	{
		t2->parent = x;
	}

	z->left = t3;
	if (t3)
	{
		t3->parent = z;
	}

	y->left = x;
	x->parent = y;

	y->right = z;
	z->parent = y;

	if (y->balance > 0)  // t3 was higher
	{
		x->balance = -1;
		z->balance = 0;
	}
	else if (y->balance == 0)
	{
		x->balance = 0;
		z->balance = 0;
	}
	else // t2 was higher
	{
		x->balance = 0;
		z->balance = 1;
	}
	y->balance = 0;
}

static void muggle_avl_tree_rotate_left_right(muggle_avl_tree_t *p_avl_tree, muggle_avl_tree_node_t *node)
{
	//////////////////////////////////////////////////////////////////////////////////////////
	// Left-Right
	//
	//        |                            |                            |
	//        X(-2)                        X                            Y
	//       / \                          / \                          / \
	//      /   \                        /   \                        /   \
	//   (1)Z   {t1}        ==>         Y    {t1}       ==>          Z     X
	//     / \  {  }                   / \   {  }                   / \   / \
	//    /   \                       /   \                        /  |   |  \
	//  {t4}   Y(-1|0|1)             Z   {t2}                   {t4}{t3} {t2}{t1}
	//  {  }  / \                   / \  {  }                   {  }{  } {  }{  }
	//       /   \                 /   \
	//     {t3}  {t2}            {t4}  {t3}
	//     {  }  {  }            {  }  {  }
	//////////////////////////////////////////////////////////////////////////////////////////

	muggle_avl_tree_node_t *parent = node->parent;
	muggle_avl_tree_node_t *x = node;
	muggle_avl_tree_node_t *z = x->left;
	MUGGLE_ASSERT(z != NULL);
	muggle_avl_tree_node_t *y = z->right;
	MUGGLE_ASSERT(y != NULL);

	MUGGLE_ASSERT(x->balance == -2);
	MUGGLE_ASSERT(z->balance == 1);

	muggle_avl_tree_node_t *t2 = y->right;
	muggle_avl_tree_node_t *t3 = y->left;

	if (parent)
	{
		if (x == parent->left)
		{
			parent->left = y;
		}
		else
		{
			parent->right = y;
		}
	}
	y->parent = parent;

	if (p_avl_tree->root == x)
	{
		p_avl_tree->root = y;
	}

	x->left = t2;
	if (t2)
	{
		t2->parent = x;
	}

	z->right = t3;
	if (t3)
	{
		t3->parent = z;
	}

	y->left = z;
	z->parent = y;

	y->right = x;
	x->parent = y;

	if (y->balance > 0) // t2 was higher
	{
		x->balance = 0;
		z->balance = -1;
	}
	else if (y->balance == 0)
	{
		x->balance = 0;
		z->balance = 0;
	}
	else // t3 was higher
	{
		x->balance = 1;
		z->balance = 0;
	}
	y->balance = 0;
}

// rebalance node
// @return deep change? true - deep decrease, false - deep not change
static bool muggle_avl_tree_rebalance(muggle_avl_tree_t *p_avl_tree, muggle_avl_tree_node_t *node)
{
	if (node->balance < -1)
	{
		// left
		muggle_avl_tree_node_t *child = node->left;
		if (child->balance <= 0)
		{
			// left-left: rotate right
			return muggle_avl_tree_rotate_right(p_avl_tree, node);
		}
		else
		{
			// left-right: rotate left right
			muggle_avl_tree_rotate_left_right(p_avl_tree, node);
			return true;
		}
	}
	else if (node->balance > 1)
	{
		// right
		muggle_avl_tree_node_t *child = node->right;
		if (child->balance >= 0)
		{
			// right-right
			return muggle_avl_tree_rotate_left(p_avl_tree, node);
		}
		else
		{
			// right-left: rotate right left
			muggle_avl_tree_rotate_right_left(p_avl_tree, node);
			return true;
		}
	}
	
	return false;
}

bool muggle_avl_tree_init(muggle_avl_tree_t *p_avl_tree, muggle_dsaa_data_cmp cmp, size_t capacity)
{
	if (cmp == NULL)
	{
		return false;
	}

	memset(p_avl_tree, 0, sizeof(*p_avl_tree));

	if (capacity > 0)
	{
		if (!MUGGLE_DS_CAP_IS_VALID(capacity))
		{
			return false;
		}

		p_avl_tree->pool = (muggle_memory_pool_t*)malloc(sizeof(muggle_memory_pool_t));
		if (p_avl_tree->pool == NULL)
		{
			return false;
		}

		if (!muggle_memory_pool_init(p_avl_tree->pool, capacity, sizeof(muggle_avl_tree_node_t)))
		{
			free(p_avl_tree->pool);
			return false;
		}
	}

	p_avl_tree->cmp = cmp;

	return true;
}

void muggle_avl_tree_destroy(muggle_avl_tree_t *p_avl_tree, 
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool)
{
	// clear avl tree
	muggle_avl_tree_clear(p_avl_tree, key_func_free, key_pool, value_func_free, value_pool);

	// destroy node memory pool
	if (p_avl_tree->pool)
	{
		muggle_memory_pool_destroy(p_avl_tree->pool);
		free(p_avl_tree->pool);
	}
}

void muggle_avl_tree_clear(muggle_avl_tree_t *p_avl_tree, 
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool)
{
	muggle_avl_tree_node_t *node = p_avl_tree->root;
	if (node)
	{
		muggle_avl_tree_erase_node(p_avl_tree, node, key_func_free, key_pool, value_func_free, value_pool);
	}
}

muggle_avl_tree_node_t* muggle_avl_tree_find(muggle_avl_tree_t *p_avl_tree, void *data)
{
	muggle_avl_tree_node_t *node = p_avl_tree->root;
	muggle_avl_tree_node_t *result = NULL;
	int ret = 0;
	while (node)
	{
		ret = p_avl_tree->cmp(data, node->key);
		if (ret == 0)
		{
			result = node;
			break;
		}
		else if (ret < 0)
		{
			node = node->left;
		}
		else if (ret > 0)
		{
			node = node->right;
		}
	}

	return result;
}

muggle_avl_tree_node_t* muggle_avl_tree_insert(muggle_avl_tree_t *p_avl_tree, void *key, void *value)
{
	// avl tree is empty, insert as root
	muggle_avl_tree_node_t *node = p_avl_tree->root;
	if (node == NULL)
	{
		node = muggle_avl_tree_allocate_node(p_avl_tree);
		if (node == NULL)
		{
			return NULL;
		}

		node->key = key;
		node->value = value;
		node->balance = 0;
		p_avl_tree->root = node;

		return node;
	}

	// insert into avl tree
	int ret = 0;
	muggle_avl_tree_node_t *new_node = NULL;
	int insert_side = AVL_TREE_SIDE_LEFT;
	while (node)
	{
		ret = p_avl_tree->cmp(key, node->key);
		if (ret == 0)
		{
			return NULL;
		}
		else if (ret < 0)
		{
			if (node->left)
			{
				node = node->left;
				continue;
			}
			else
			{
				new_node = muggle_avl_tree_allocate_node(p_avl_tree);
				if (new_node == NULL)
				{
					return NULL;
				}

				node->left = new_node;
				insert_side = AVL_TREE_SIDE_LEFT;
				break;
			}
		}
		else if (ret > 0)
		{
			if (node->right)
			{
				node = node->right;
				continue;
			}
			else
			{
				new_node = muggle_avl_tree_allocate_node(p_avl_tree);
				if (new_node == NULL)
				{
					return NULL;
				}

				node->right = new_node;
				insert_side = AVL_TREE_SIDE_RIGHT;
				break;
			}
		}
	}

	new_node->parent = node;
	new_node->key = key;
	new_node->value = value;
	new_node->balance = 0;

	// retracing
	while (node)
	{
		if (insert_side == AVL_TREE_SIDE_LEFT)
		{
			node->balance--;
		}
		else
		{
			node->balance++;
		}

		// if balance factor == 0, height of that subtree remain unchanged
		// if balance factor == 1 or -1, the height of the subtree increased and retracing needs to continue
		// if balance factor == 2 or -2, rotate node, and subtree has the same height as before
		if (node->balance == 0)
		{
			break;
		}
		else if (node->balance == 1 || node->balance == -1)
		{
			if (node->parent)
			{
				if (node->parent->left == node)
				{
					insert_side = AVL_TREE_SIDE_LEFT;
				}
				else
				{
					insert_side = AVL_TREE_SIDE_RIGHT;
				}
				node = node->parent;
			}
			else
			{
				break;
			}
		}
		else
		{
			MUGGLE_ASSERT(node->balance == 2 || node->balance == -2);
			muggle_avl_tree_rebalance(p_avl_tree, node);
			break;
		}
	}

	return new_node;
}

void muggle_avl_tree_remove(
	muggle_avl_tree_t *p_avl_tree, muggle_avl_tree_node_t *node,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool)
{
	// move data into leaf
	muggle_avl_tree_node_t *target = NULL;
	void *tmp = NULL;
	while (true)
	{
		if (node->left == NULL && node->right == NULL)
		{
			break;
		}

		if (node->left)
		{
			target = node->left;
			while (target->right)
			{
				target = target->right;
			}
		}
		else if (node->right)
		{
			target = node->right;
			while (target->left)
			{
				target = target->left;
			}
		}

		if (target)
		{
			tmp = node->key;
			node->key = target->key;
			target->key = tmp;

			tmp = node->value;
			node->value = target->value;
			target->value = tmp;

			node = target;
		}
	}

	// if node is root, then erase node and return
	if (node->parent == NULL)
	{
		muggle_avl_tree_erase_node(p_avl_tree, node, key_func_free, key_pool, value_func_free, value_pool);
		p_avl_tree->root = NULL;
		return;
	}

	// get remove side
	int remove_side = AVL_TREE_SIDE_LEFT;
	if (node->parent->left == node)
	{
		remove_side = AVL_TREE_SIDE_LEFT;
	}
	else
	{
		remove_side = AVL_TREE_SIDE_RIGHT;
	}

	muggle_avl_tree_node_t *parent = node->parent;
	muggle_avl_tree_erase_node(p_avl_tree, node, key_func_free, key_pool, value_func_free, value_pool);

	node = parent;
	while (node)
	{
		if (remove_side == AVL_TREE_SIDE_LEFT)
		{
			node->balance++;
		}
		else
		{
			node->balance--;
		}

		// if balance factor == 1 or -1, the height of the subtree remain unchanged
		// if balance factor == 0 (it must have been +1 or -1), then height of the subtree decreased and retracing needs to continue
		// if balance factor == 2 or -2, rotate node, and if the height of subtree changed, retracing needs to continue
		if (node->balance == 1 || node->balance == -1)
		{
			break;
		}
		else if (node->balance == 0)
		{
			if (node->parent)
			{
				if (node->parent->left == node)
				{
					remove_side = AVL_TREE_SIDE_LEFT;
				}
				else
				{
					remove_side = AVL_TREE_SIDE_RIGHT;
				}
				node = node->parent;
			}
			else
			{
				break;
			}
		}
		else
		{
			MUGGLE_ASSERT(node->balance == 2 || node->balance == -2);

			parent = node->parent;
			if (parent)
			{
				if (parent->left == node)
				{
					remove_side = AVL_TREE_SIDE_LEFT;
				}
				else
				{
					remove_side = AVL_TREE_SIDE_RIGHT;
				}
			}

			bool depth_decrease = muggle_avl_tree_rebalance(p_avl_tree, node);
			if (depth_decrease)
			{
				node = parent;
			}
			else
			{
				break;
			}
		}
	}
}
