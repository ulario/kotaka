#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <status.h>

inherit SECOND_AUTO;
inherit "../base/node";

#define MAX_MASS 256

mixed *sub;
object prev;
object next;

int begin;
int end;

static void create()
{
	::create();
	
	sub = allocate(MAX_MASS);
	begin = end = MAX_MASS / 2;
}

static void destruct()
{
	if (next) {
		discard_node(next);
	}
}

object get_prev()
{
	check_caller();
	return prev;
}

object get_next()
{
	check_caller();
	return next;
}

int get_mass()
{
	check_caller();
	return end - begin;
}

int empty()
{
	check_caller();
	return begin == end;
}

int get_begin()
{
	check_caller();
	return begin;
}

int get_end()
{
	check_caller();
	return end;
}

int full_front()
{
	check_caller();
	return begin == 0;
}

int full_back()
{
	check_caller();
	return end == sizeof(sub);
}

void set_prev(object new)
{
	check_caller();
	prev = new;
}

void set_next(object new)
{
	check_caller();
	next = new;
}

mixed get_front()
{
	check_caller();
	
	if (begin >= end) {
		error("Subscript out of range");
	}
	
	return sub[begin];
}

mixed get_back()
{
	check_caller();

	if (begin >= end) {
		error("Subscript out of range");
	}
	
	return sub[end - 1];
}

void set_front(mixed value)
{
	check_caller();
	ASSERT(begin < end);
	
	sub[begin] = value;
}

void set_back(mixed value)
{
	check_caller();
	ASSERT(begin < end);
	
	sub[end - 1] = value;
}

void push_front(mixed value)
{
	check_caller();

	ASSERT(begin > 0);

	sub[--begin] = value;
}

void push_back(mixed value)
{
	check_caller();

	ASSERT(end < sizeof(sub));

	sub[end++] = value;
}

void pop_front()
{
	check_caller();
	ASSERT(begin < end);

	sub[begin++] = nil;
}

void pop_back()
{
	check_caller();
	ASSERT(begin < end);

	sub[--end] = nil;
}

mixed get_element(int index)
{
	check_caller();

	if (index < begin || index >= end) {
		error("Subscript out of range");
	}

	return sub[index];
}

void set_element(int index, mixed data)
{
	check_caller();

	if (index < begin || index >= end) {
		error("Subscript out of range");
	}

	sub[index] = data;
}

void dump()
{
	int index;
	
	LOGD->post_message("dump", LOG_DEBUG, "[" + begin + ", " + end + ")");
	
	for (index = begin; index < end; index++) {
		LOGD->post_message("dump", LOG_DEBUG, index + ": " + sub[index]);
	}

	LOGD->post_message("dump", LOG_DEBUG, index + ": <end>");
}
