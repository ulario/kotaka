#include <kotaka/paths.h>

inherit LIB_USTATE;

int callout;
float interval;

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

private void schedule()
{
	if (callout) {
		remove_call_out(callout);
	}

	if (interval > 0.0) {
		callout = call_out("report", interval);
	}
}

private void do_report()
{
	send_out("\033[1;1H\033[2J"
		+ STATUSD->status_message()
		+ "\n[status] ");
}

static void report()
{
	callout = 0;
	schedule();
	
	do_report();
}

static void begin()
{
	interval = 60.0;

	do_report();
	schedule();
}

static void end()
{
	destruct_object(this_object());
}

static void receive_in(string input)
{
	if (input == "stop") {
		pop_state();
		return;
	} else {
		if (sscanf(input, "%f", interval)) {
			schedule();
		} else {
			do_report();
		}
	}
}
