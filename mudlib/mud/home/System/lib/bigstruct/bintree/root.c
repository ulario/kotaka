#include <kernel/access.h>

#include <kotaka/bigstruct.h>
#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;
inherit "../base/root";

object top;

static object prev_node(object node);
static object next_node(object node);

static void rotate_left(object node);
static void rotate_right(object node);

static void destruct()
{
	if (top) {
		discard_node(top);
	}
}

static object leftest(object node)
{
	object left;
	
	left = node;
	
	while (left) {
		node = left;
		left = node->get_left();
	}
	
	return node;
}

static object rightest(object node)
{
	object right;
	
	right = node;
	
	while (right) {
		node = right;
		right = node->get_right();
	}
	
	return node;
}

static object parent(object node)
{
	return node->get_parent();
}

static object grandparent(object node)
{
	return node->get_parent()->get_parent();
}

static object uncle(object node)
{
	object grandparent;
	object uncle;
	object parent;
	
	parent = node->get_parent();
	grandparent = parent->get_parent();

	uncle = grandparent->get_left();
	parent = node->get_parent();
	
	if (uncle == parent) {
		uncle = grandparent->get_right();
	}
	
	return uncle;
}

static object sibling(object node)
{
	object parent;
	object left;
	
	parent = node->get_parent();
	left = parent->get_left();
	
	if (node == left) {
		return parent->get_right();
	} else {
		return left;
	}
}

static atomic void rotate_left(object node)
{
	/*   P        P   */
	/*                */
	/*   N        R   */
	/* a   R    N   c */
	/*    b c  a b    */
	
	object parent;
	object right;
	object swivel;
	
	parent = node->get_parent();
	right = node->get_right();
	
	ASSERT(right);
	
	swivel = right->get_left();
	
	if (parent) {
		if (node == parent->get_left()) {
			parent->set_left(right);
		} else {
			parent->set_right(right);
		}
	} else {
		top = right;
	}
	
	if (swivel) {
		swivel->set_parent(node);
	}
	
	node->set_right(swivel);
	node->set_parent(right);
	
	right->set_parent(parent);
	right->set_left(node);
}





static atomic void rotate_right(object node)
{
	/*   P        P   */
	/*                */
	/*   L        N   */
	/* a   N    L   c */
	/*    b c  a b    */
	
	object parent;
	object left;
	object swivel;
	
	parent = node->get_parent();
	left = node->get_left();
	
	ASSERT(left);

	swivel = left->get_right();
	
	if (parent) {
		if (node == parent->get_left()) {
			parent->set_left(left);
		} else {
			parent->set_right(left);
		}
	} else {
		top = left;
	}
	
	if (swivel) {
		swivel->set_parent(node);
	}
	
	node->set_left(swivel);
	node->set_parent(left);
	
	left->set_parent(parent);
	left->set_right(node);
}

static atomic void swap_nodes(object a, object b)
{
	object al, ar, ap;
	object bl, br, bp;
	
	al = a->get_left();
	ar = a->get_right();
	ap = a->get_parent();
	
	bl = b->get_left();
	br = b->get_right();
	bp = b->get_parent();
	
	if (bl) {
		bl->set_parent(a);
	}
	if (br) {
		br->set_parent(a);
	}
	
	if (al) {
		al->set_parent(b);
	}
	if (ar) {
		ar->set_parent(b);
	}
	
	if (ap) {
		if (ap->get_left() == a) {
			ap->set_left(b);
		} else {
			ap->set_right(b);
		}
	} else {
		top = b;
	}
	if (bp) {
		if (bp->get_left() == b) {
			bp->set_left(a);
		} else {
			bp->set_right(a);
		}
	} else {
		top = a;
	}
	
	if (al == b) {
		al = a;
	}
	if (ar == b) {
		ar = a;
	}
	if (ap == b) {
		ap = a;
	}
	
	if (bl == a) {
		bl = b;
	}
	if (br == a) {
		br = b;
	}
	if (bp == a) {
		bp = b;
	}

	a->set_left(bl);
	a->set_right(br);
	a->set_parent(bp);

	b->set_left(al);
	b->set_right(ar);
	b->set_parent(ap);
}

static object insert_node(object next)
{
	object node;
	
	node = new_node();
	
	if (next) {
		object next_left;
		
		next_left = next->get_left();
		
		if (next_left) {
			object next_left_rightest;
			
			next_left_rightest = rightest(next_left);
			
			next_left_rightest->set_right(node);
			node->set_parent(next_left_rightest);
		} else {
			next->set_left(node);
			node->set_parent(next);
		}
		
		ASSERT(next_node(node) == next);
		ASSERT(prev_node(next) == node);
	} else {
		/* needs to go on extreme right */
		/* zip right and glue it on */
		
		if (top) {
			object parent;
			
			parent = rightest(top);
			
			parent->set_right(node);
			node->set_parent(parent);
		} else {
			/* tree was empty */
			top = node;
		}

		ASSERT(next_node(node) == nil);
		ASSERT(rightest(top) == node);
	}
	
	if (next) {
		ASSERT(prev_node(next) == node);
		ASSERT(next_node(node) == next);
	} else {
		ASSERT(node == rightest(top));
	}

	return node;
}

static void delete_node(object node)
{
	object left, right;
	object parent;
	
	left = node->get_left();
	right = node->get_right();
	
	if (left && right) {
		swap_nodes(node, next_node(node));
		
		left = node->get_left();
		right = node->get_right();
	}

	parent = node->get_parent();
	
	if (left) {
		/* slide left branch up */
		if (parent) {
			if (parent->get_left() == node) {
				parent->set_left(left);
			} else {
				parent->set_right(left);
			}
		} else {
			top = left;
		}
		
		left->set_parent(parent);
		
		node->set_parent(nil);
		node->set_left(nil);
	} else if (right) {
		if (parent) {
			if (parent->get_left() == node) {
				parent->set_left(right);
			} else {
				parent->set_right(right);
			}
		} else {
			top = right;
		}
		
		right->set_parent(parent);
		
		node->set_parent(nil);
		node->set_right(nil);
	} else {
		/* just disappear */
		if (parent) {
			if (parent->get_left() == node) {
				parent->set_left(nil);
			} else {
				parent->set_right(nil);
			}
		} else {
			top = nil;
		}
		
		node->set_parent(nil);
	}
	
	discard_node(node);
}

void clear()
{
	check_caller(WRITE_ACCESS);
	
	discard_node(top);
	
	top = nil;
}

static object prev_node(object node)
{
	object left;
	object parent;
	
	ASSERT(node);
	
	left = node->get_left();
	
	if (left) {
		return rightest(left);
	}
	
	parent = node->get_parent();
	
	while (parent) {
		if (parent->get_right() == node) {
			return parent;
		}
		
		node = parent;
		parent = parent->get_parent();
	}
}

static object next_node(object node)
{
	object right;
	object parent;
	
	ASSERT(node);
	
	right = node->get_right();
	
	if (right) {
		return leftest(right);
	}
	
	parent = node->get_parent();
	
	while (parent) {
		if (parent->get_left() == node) {
			return parent;
		}
		
		node = parent;
		parent = parent->get_parent();
	}
}

static void rebalance_step_gather(object node, object array)
{
	object left;
	object right;
	int sz;
	
	left = node->get_left();
	right = node->get_right();
	
	if (left) {
		rebalance_step_gather(left, array);
	}
	
	sz = array->get_size();
	array->set_size(sz + 1);
	
	array->set_element(sz, node);

	if (right) {
		rebalance_step_gather(right, array);
	}
}

static void rotate_up_to(object node, object ceiling)
{
	object parent;
	
	while (parent = node->get_parent(),
		parent != nil && parent != ceiling) {

		if (node == parent->get_left()) {
			rotate_right(parent);
		} else {
			rotate_left(parent);
		}
	}
}

static void rebalance_step_scatter(object array, object ceiling)
{
	int mid;
	int high;
	
	object node;
	
	high = array->get_size() - 1;
	mid = high / 2;
	
	node = array->get_element(mid);
	
	rotate_up_to(node, ceiling);
	
	if (mid > 1) {
		rebalance_step_scatter(array->slice(0, mid - 1), node);
	}
	
	if (mid < high - 1) {
		rebalance_step_scatter(array->slice(mid + 1, high), node);
	}
}

static void rebalance()
{
	object array;
	
	array = new_object(BIGSTRUCT_ARRAY_LWO);
	
	rebalance_step_gather(top, array);
	rebalance_step_scatter(array, nil);
}
