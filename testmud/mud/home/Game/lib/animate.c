#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>
#include <status.h>

inherit LIB_USTATE;

int stopped;
int reading;
int introed;

mixed *oldtime;
int tsec;
int tframe;
int framerate;

static void create()
{
	::create();
}

static void destruct()
{
	::destruct();
}

static void do_frame(float diff);

void begin()
{
	oldtime = millitime();
	framerate = 10;

	query_user()->set_mode(MODE_NOECHO);

	tsec = oldtime[0];
	tframe = (int)floor(oldtime[1] * (float)framerate);

	ACCESS_CHECK(previous_object() == query_user());

	call_out("frame", 0);
}

void stop()
{
	ACCESS_CHECK(previous_object() == query_user());

	query_user()->set_mode(MODE_ECHO);

	stopped = 1;
}

void go()
{
	ACCESS_CHECK(previous_object() == query_user());

	query_user()->set_mode(MODE_NOECHO);

	stopped = 0;
}

static void frame()
{
	string buffer;
	object paint;
	mixed *time;
	float diff;
	mixed *frametime;

	time = millitime();

	tframe++;
	while (tframe >= framerate) {
		tframe -= framerate;
		tsec++;
	}

	frametime = ({ tsec, (float)tframe / (float)framerate });

	if (frametime[0] < time[0] || (frametime[0] == time[0] && frametime[1] < time[1])) {
		tsec = time[0];
		tframe = (int)floor(time[1] * (float)framerate);
		call_out("frame", 0);
		frametime = nil;
		diff = 0.0;
	} else {
		diff = frametime[1] - time[1];
		diff += (float)(frametime[0] - time[0]);

		call_out("frame", diff);
	}

	diff = time[1] - oldtime[1];
	diff += (float)(time[0] - oldtime[0]);
	oldtime = time;

	do_frame(diff);
}

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

	query_user()->set_mode(MODE_ECHO);

	destruct_object(this_object());
}

void receive_in(string input)
{
	int test;
	string first;
	float target;

	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (!sscanf(input, "%s %s", first, input)) {
		first = input;
		input = "";
	}

	switch(first) {
	case "frame":
		if (!sscanf(input, "%d", test)) {
			send_out("Invalid number.\n");
			break;
		}

		if (test <= 0) {
			send_out("Illegal number.\n");
			break;
		}

		target = (float)tframe / (float)framerate;
		framerate = test;
		tframe = (int)(target * (float)framerate);

		break;

	case "quit":
		pop_state();
		return;

	default:
		send_out(first + ": command not recognized.\n");
	}

	reading = 0;
}
