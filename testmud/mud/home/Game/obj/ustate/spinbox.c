#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>
#include <status.h>

inherit LIB_USTATE;

int stopped;
int reading;
int introed;
int frames;
int due;
float pi;
float otime;
string path;

static void create(int clone)
{
	::create();
	pi = atan(1.0) * 4.0;
}

static void destruct(int clone)
{
	::destruct();
}

void begin()
{
	int i;
	mixed *times;

	ACCESS_CHECK(previous_object() == query_user());

	send_out("\033[1;1H\033[2J");

	call_out("particle", 0);
	call_out("second", 1);

	times = millitime();
	otime = (float)times[0] + times[1];
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
}

static void particle()
{
	int x, y, i;
	string buffer;
	float time, diff, hand;
	object paint;
	mixed *times;

	times = millitime();
	time = (float)times[0] + times[1];
	diff = time - otime;
	otime = time;

	call_out("particle", 0);
	send_out("\033[1;1HCanvas test:\n");

	paint = new_object(LWO_PAINTER);
	paint->start(80, 20);
	paint->set_color(0xC);

	send_out(status(ST_TICKS) + " ticks starting.\n");

	paint->set_color(0x2);
	paint->move_pen(20, 0);
	paint->draw("+-----------+");
	paint->move_pen(20, 1);
	paint->draw("|     /     |");
	paint->move_pen(20, 2);
	paint->draw("+-----------+");

	buffer = "   " + random(250);
	buffer = buffer[strlen(buffer) - 3 ..];
	buffer += " / 250";

	hand = time * 4.0;
	hand -= floor(hand);

	paint->set_color(0xC);
	for (i = 0; i < 8; i++) {
		float fx, fy;
		fx = sin(((float)i + hand) * pi / 4.0) * 20.0 + 40.0;
		fy = -cos(((float)i + hand) * pi / 4.0) * 5.0 + 5.0;
		paint->move_pen((int)fx, (int)fy);
		paint->draw("*");

		if (fy > 5.0) {
			for (y = 0; y <= 5; y++) {
				paint->move_pen((int)fx, (int)fy + y);
				paint->draw("*");
			}
		}
	}

	frames++;

	send_out(paint->render_color());

	if (due) {
		due = 0;
		send_out("FPS: " + frames);
		frames = 0;
	}
}

static void second()
{
	call_out("second", 1);
	due = 1;
}

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

	destruct_object(this_object());
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

	switch(first) {
	case "quit":
		pop_state();
		return;
	default:
		send_out(first + ": command not recognized.\n");
	}

	reading = 0;
}
