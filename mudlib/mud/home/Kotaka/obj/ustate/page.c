#include <kotaka/paths.h>

inherit LIB_USTATE;

object linebuf;

static void create(int clone)
{
	if (clone) {
		::create();
		linebuf = new_object("~/lwo/linebuf");
	}
}

private void pour()
{
	string line;
	int count;
	
	count = 0;
	
	for (;;) {
		if (linebuf->empty() || count >= 20) {
			break;
		}

		line = linebuf->getline();
		send_out(line + "\n");
		
		count++;
	}
	
	if (linebuf->empty()) {
		pop_state();
	} else {
		send_out("(more ...)");
	}
}

void set_text(string text)
{
	linebuf->load(text);
}

static void begin()
{
	pour();
}

static void end()
{
	destruct_object(this_object());
}

static void receive_in(string str)
{
	pour();
}
