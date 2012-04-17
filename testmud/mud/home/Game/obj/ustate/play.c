#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/constant.h>
#include <kotaka/assert.h>

#include <game/paths.h>

inherit LIB_USTATE;

int stopped;
int reading;
int introed;

object body;

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

private void prompt()
{
	send_out("(\033[32mprimitive human\033[37m) > ");
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	prompt();
	reading = 1;
}

void stop()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 1;
}

void go()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 0;

	if (!reading) {
		prompt();
	}
}

static void self_destruct()
{
	destruct_object(this_object());
}

private void do_help()
{
	send_out(read_file("~/data/doc/guest_help"));
}

void receive_in(string input)
{
	string first;

	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (!sscanf(input, "%s %s", first, input)) {
		first = input;
		input = "";
	}

	if (first == "quit") {
		pop_state();
		return;
	} else if (first == "recompile") {
		OBJECTD->klib_recompile();
		OBJECTD->global_recompile();
	} else {
		mixed tree;
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}
