#include <kotaka/assert.h>
#include <kotaka/checkarg.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

#include <status.h>
#include <kotaka/bigstruct.h>

inherit SECOND_AUTO;

object garbage;
int callout;

private void schedule();

static void create()
{
	callout = -1;
}

void enable()
{
	ACCESS_CHECK(SYSTEM());
	
	garbage = new_object(BIGSTRUCT_DEQUE_LWO);
}

int get_mass()
{
	return garbage->get_mass();
}

private void schedule()
{
	if (callout != -1) {
		remove_call_out(callout);
		callout = -1;
	}
	
	if (garbage->empty()) {
		return;
	}
	
	callout = call_out("process", 0);
}

void reap(object turkey)
{
	CHECKARG(turkey, 1, "reap");
	ACCESS_CHECK(SYSTEM() || previous_object()->query_owner() == turkey->query_owner());
	CHECKARG(!sscanf(object_name(turkey), "%*s#-1"), 1, "reap");
	
	garbage->push_back(turkey);
	schedule();
}

static void process()
{
	object turkey;

	callout = -1;
	
	if (garbage->empty()) {
		return;
	}
	
	turkey = garbage->get_front();
	garbage->pop_front();
	
	catch {
		if (turkey) {
			destruct_object(turkey);
		}
	}
	
	schedule();
}
