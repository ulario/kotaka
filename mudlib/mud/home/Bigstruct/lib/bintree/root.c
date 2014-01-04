/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <kernel/access.h>
#include <kotaka/assert.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/privilege.h>

inherit "../base/root";

object top;

static object prev_node(object node);
static object next_node(object node);

static void rotate_left(object node);
static void rotate_right(object node);

private void purge_node(object node)
{
	object subnode;

	if (subnode = node->query_left()) {
		purge_node(subnode);
	}

	if (subnode = node->query_right()) {
		purge_node(subnode);
	}

	discard_node(node);
}

static void destruct()
{
	if (top) {
		purge_node(top);
	}
}

static object leftest(object node)
{
	object left;

	left = node;

	while (left) {
		node = left;
		left = node->query_left();
	}

	return node;
}

static object rightest(object node)
{
	object right;

	right = node;

	while (right) {
		node = right;
		right = node->query_right();
	}

	return node;
}

static object parent(object node)
{
	return node->query_parent();
}

static object grandparent(object node)
{
	return node->query_parent()->query_parent();
}

static object uncle(object node)
{
	object grandparent;
	object uncle;
	object parent;

	parent = node->query_parent();
	grandparent = parent->query_parent();

	uncle = grandparent->query_left();
	parent = node->query_parent();

	if (uncle == parent) {
		uncle = grandparent->query_right();
	}

	return uncle;
}

static object sibling(object node)
{
	object parent;
	object left;

	parent = node->query_parent();
	left = parent->query_left();

	if (node == left) {
		return parent->query_right();
	} else {
		return left;
	}
}

static atomic void rotate_left(object node)
{
	object parent;
	object right;
	object swivel;

	parent = node->query_parent();
	right = node->query_right();

	ASSERT(right);

	swivel = right->query_left();

	if (parent) {
		if (node == parent->query_left()) {
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
	object parent;
	object left;
	object swivel;

	parent = node->query_parent();
	left = node->query_left();

	ASSERT(left);

	swivel = left->query_right();

	if (parent) {
		if (node == parent->query_left()) {
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

	al = a->query_left();
	ar = a->query_right();
	ap = a->query_parent();

	bl = b->query_left();
	br = b->query_right();
	bp = b->query_parent();

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
		if (ap->query_left() == a) {
			ap->set_left(b);
		} else {
			ap->set_right(b);
		}
	} else {
		top = b;
	}
	if (bp) {
		if (bp->query_left() == b) {
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

		next_left = next->query_left();

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

	left = node->query_left();
	right = node->query_right();

	if (left && right) {
		swap_nodes(node, next_node(node));

		left = node->query_left();
		right = node->query_right();
	}

	parent = node->query_parent();

	if (left) {
		/* slide left branch up */
		if (parent) {
			if (parent->query_left() == node) {
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
			if (parent->query_left() == node) {
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
			if (parent->query_left() == node) {
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

	left = node->query_left();

	if (left) {
		return rightest(left);
	}

	parent = node->query_parent();

	while (parent) {
		if (parent->query_right() == node) {
			return parent;
		}

		node = parent;
		parent = parent->query_parent();
	}
}

static object next_node(object node)
{
	object right;
	object parent;

	ASSERT(node);

	right = node->query_right();

	if (right) {
		return leftest(right);
	}

	parent = node->query_parent();

	while (parent) {
		if (parent->query_left() == node) {
			return parent;
		}

		node = parent;
		parent = parent->query_parent();
	}
}

object query_first_node()
{
	ACCESS_CHECK(BIGSTRUCT());

	return rightest(top);
}

object query_prev_node(object node)
{
	ACCESS_CHECK(BIGSTRUCT());

	return prev_node(node);
}

object query_next_node(object node)
{
	ACCESS_CHECK(BIGSTRUCT());

	return next_node(node);
}

static void rebalance_step_gather(object node, object array)
{
	object left;
	object right;
	int sz;

	left = node->query_left();
	right = node->query_right();

	if (left) {
		rebalance_step_gather(left, array);
	}

	sz = array->query_size();
	array->set_size(sz + 1);

	array->set_element(sz, node);

	if (right) {
		rebalance_step_gather(right, array);
	}
}

static void rotate_up_to(object node, object ceiling)
{
	object parent;

	while (parent = node->query_parent(),
		parent != nil && parent != ceiling) {

		if (node == parent->query_left()) {
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

	high = array->query_size() - 1;
	mid = high / 2;

	node = array->query_element(mid);

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
