#include <kernel/access.h>

#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

#include <kotaka/bigstruct.h>

inherit SECOND_AUTO;
inherit "../base/root";

object first;
object last;

void dump();

atomic static void create()
{
	::create();

	first = last = new_node();
}

static void destruct()
{
	if (first) {
		object turkey;
		turkey = first;
		first = last = nil;
		discard_node(turkey);
	}
}

mixed get_front()
{
	check_caller(READ_ACCESS);

	return first->get_front();
}

mixed get_back()
{
	check_caller(READ_ACCESS);

	return last->get_back();
}

void set_front(mixed value)
{
	check_caller(WRITE_ACCESS);

	first->set_front(value);
}

void set_back(mixed value)
{
	check_caller(WRITE_ACCESS);

	last->set_back(value);
}

atomic void push_front(mixed value)
{
	check_caller(WRITE_ACCESS);

	if (first->full_front()) {
		object new;

		new = new_node();

		first->set_prev(new);
		new->set_next(first);
		first = new;
	}

	first->push_front(value);
}

atomic void push_back(mixed value)
{
	check_caller(WRITE_ACCESS);

	if (last->full_back()) {
		object new;

		new = new_node();

		last->set_next(new);
		new->set_prev(last);
		last = new;
	}

	last->push_back(value);
}

atomic void pop_front()
{
	check_caller(WRITE_ACCESS);

	first->pop_front();

	if (first->get_mass() == 0 && first != last) {
		object old;

		old = first;
		first = first->get_next();
		old->set_next(nil);

		discard_node(old);
	}
}

atomic void pop_back()
{
	check_caller(WRITE_ACCESS);

	last->pop_back();

	if (last->get_mass() == 0 && first != last) {
		object old;

		old = last;
		last = last->get_prev();

		discard_node(old);
	}
}

int empty()
{
	check_caller(READ_ACCESS);

	return first->empty();
}

atomic void clear()
{
	check_caller(WRITE_ACCESS);

	if (first) {
		object turkey;

		turkey = first;
		first = last = nil;

		discard_node(turkey);
	}
}

int get_mass()
{
	object node;
	int mass;

	check_caller(READ_ACCESS);

	mass = 0;

	node = first;

	while (node) {
		mass += node->get_mass();
		node = node->get_next();
	}

	return mass;
}

object begin()
{
	object iterator;

	check_caller(READ_ACCESS);

	iterator = new_object(BIGSTRUCT_DEQUE_ITERATOR);

	iterator->set_node(first);
	iterator->set_index(first->get_begin());
}

object end()
{
	object iterator;

	check_caller(READ_ACCESS);

	iterator = new_object(BIGSTRUCT_DEQUE_ITERATOR);

	iterator->set_node(nil);
}

void iterator_increment()
{
	object iterator;
	object node;
	int index;

	check_caller(0);

	iterator = previous_object();

	node = iterator->root_get_node();

	ASSERT(node);
	ASSERT(node->get_root() == this_object());

	index = iterator->root_get_index();

	if (index + 1 < node->get_end()) {
		index++;
		iterator->root_set_index(index);
	} else {
		node = node->get_next();

		if (node) {
			index = node->get_begin();
		} else {
			index = -1;
		}

		iterator->root_set_node(node);
		iterator->root_set_index(index);
	}
}

void iterator_decrement()
{
	object iterator;
	object node;
	int index;

	check_caller(0);

	iterator = previous_object();

	node = iterator->root_get_node();

	ASSERT(node);
	ASSERT(node->get_root() == this_object());

	index = iterator->root_get_index();

	if (index > node->get_begin()) {
		index--;
		iterator->root_set_index(index);
	} else {
		node = node->get_prev();

		if (node) {
			index = node->get_end() - 1;
		} else {
			index = -1;
		}

		iterator->root_set_node(node);
		iterator->root_set_index(index);
	}
}

mixed iterator_get_data()
{
	object iterator;
	object node;
	int index;

	check_caller(0);

	iterator = previous_object();

	node = iterator->root_get_node();

	ASSERT(node);
	ASSERT(node->get_root() == this_object());

	index = iterator->root_get_index();

	return node->get_data(index);
}

void iterator_set_data(mixed data)
{
	object iterator;
	object node;
	int index;
	
	check_caller(0);

	iterator = previous_object();

	node = iterator->root_get_node();
	
	ASSERT(node);
	ASSERT(node->get_root() == this_object());
	
	index = iterator->root_get_index();
	
	node->set_data(index, data);
}

void dump()
{
	object node;
	
	ACCESS_CHECK(SYSTEM());
	
	node = first;
	
	while (node) {
		LOGD->post_message("dump", LOG_DEBUG, "Dumping node " + object_name(node));
		node->dump();
		node = node->get_next();
	}
}
