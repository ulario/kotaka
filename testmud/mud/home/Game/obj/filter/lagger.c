#include <kernel/user.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>

inherit LIB_FILTER;
string outbuf;
int callout;
int escape;

private void schedule()
{
	if (callout == -1 && strlen(outbuf)) {
		switch(outbuf[0]) {
		case '\033': escape = 1; break;
		case 'A' .. 'Z': escape = 0; break;
		case 'a' .. 'z': escape = 0; break;
		}
		
		if (escape) {
			callout = call_out("char", 0.01);
		} else {
			callout = call_out("char", 0.05);
		}
	}
}

static void create(int clone)
{
	if (clone) {
		outbuf = "";
		callout = -1;
		::create();
	}
}

int message(string str)
{
	ACCESS_CHECK(previous_program() == LIB_USER);

	outbuf += str;
	schedule();
	
	return 1;
}

static void char()
{
	callout = -1;
	
	::message(outbuf[0 .. 0]);
	outbuf = outbuf[1 ..];
	
	schedule();
}
