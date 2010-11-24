#include <kernel/access.h>
#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <status.h>

inherit SECOND_AUTO;
inherit "../base/iterator";

mixed index;

static void create()
{
	::create();
}

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

mixed get_value()
{
	check_caller(READ_ACCESS);
	
	return query_root()->iterator_get_value();
}

void set_value(mixed value)
{
	check_caller(WRITE_ACCESS);
	
	query_root()->iterator_set_value(value);
}

mixed get_index()
{
	check_caller(READ_ACCESS);
	
	return index;
}

void set_index(mixed new_index)
{
	check_caller(READ_ACCESS);
	
	index = new_index;
}
