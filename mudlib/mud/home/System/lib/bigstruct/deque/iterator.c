#include <kernel/access.h>

#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <status.h>

inherit SECOND_AUTO;
inherit "../base/iterator";

object node;	/* current node */
int index;	/* index within current node */

void increment()
{
	check_caller(READ_ACCESS);

	query_root()->iterator_increment();
}

void decrement()
{
	check_caller(READ_ACCESS);

	query_root()->iterator_decrement();
}

mixed get_data()
{
	check_caller(READ_ACCESS);

	return query_root()->iterator_get_data();
}

void set_data(mixed data)
{
	check_caller(WRITE_ACCESS);

	query_root()->iterator_set_data(data);
}

object root_get_node()
{
	ACCESS_CHECK(previous_object() == query_root());

	return node;
}

int root_get_index()
{
	ACCESS_CHECK(previous_object() == query_root());

	return index;
}

void root_set_node(object new_node)
{
	ACCESS_CHECK(previous_object() == query_root());

	node = new_node;
}

void root_set_index(int new_index)
{
	ACCESS_CHECK(previous_object() == query_root());

	index = new_index;
}
