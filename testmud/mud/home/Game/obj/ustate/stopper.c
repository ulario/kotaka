#include <kotaka/paths.h>

inherit LIB_USTATE;

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

void abort()
{
	pop_state();
}

static void begin()
{
	send_out("Stopper on\n");
	
	call_out("pop_state", 3);
}

static void end()
{
	send_out("Stopper off\n");
	destruct_object(this_object());
}

static void receive_in(string input)
{
	send_out("You picked " + input + "\n");
}

static void receive_out(string output)
{
	send_out("Receive out:\n");
	send_out(output);
}
