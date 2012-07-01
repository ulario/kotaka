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

#define CLK_X 45
#define CLK_Y 9

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

private void do_clock(object paint, float time)
{
	int i, x, y;
	float hand;

	paint->set_color(0x8F);
	for (i = 0; i < 120; i++) {
		x = (int)((sin((float)i * pi / 60.0)) * 8.0) + CLK_X;
		y = (int)((-cos((float)i * pi / 60.0)) * 8.0) + CLK_Y;

		paint->move_pen(x, y);
		paint->draw("@");
	}

	paint->set_color(0x8B);
	for (i = 0; i < 12; i++) {
		x = (int)((sin((float)i * pi / 6.0)) * 7.25) + CLK_X;
		y = (int)((-cos((float)i * pi / 6.0)) * 7.25) + CLK_Y;

		paint->move_pen(x, y);
		
		switch(i) {
		case 0:
		case 6: paint->draw("|"); break;
		case 1:
		case 2:
		case 7:
		case 8: paint->draw("/"); break;
		case 3:
		case 9: paint->draw("-"); break;
		case 4:
		case 5:
		case 10:
		case 11: paint->draw("\\"); break;
		}
	}

	hand = time / 60.0;
	hand -= floor(hand);

	paint->set_color(0x89);

	for (i = 0; i < 7; i++) {
		x = (int)((sin(hand * pi * 2.0)) * (float)i) + CLK_X;
		y = (int)((-cos(hand * pi * 2.0)) * (float)i) + CLK_Y;

		paint->move_pen(x, y);
		paint->draw("S");
	}

	hand = time / 3600.0;
	hand -= floor(hand);

	paint->set_color(0x8A);

	for (i = 0; i < 5; i++) {
		x = (int)((sin(hand * pi * 2.0)) * ((float)i + 0.5)) + CLK_X;
		y = (int)((-cos(hand * pi * 2.0)) * ((float)i + 0.5)) + CLK_Y;

		paint->move_pen(x, y);
		paint->draw("M");
	}

	hand = time / (3600.0 * 12.0);
	hand -= floor(hand);

	paint->set_color(0x8C);

	for (i = 0; i < 4; i++) {
		x = (int)((sin(hand * pi * 2.0)) * (float)i) + CLK_X;
		y = (int)((-cos(hand * pi * 2.0)) * (float)i) + CLK_Y;

		paint->move_pen(x, y);
		paint->draw("H");
	}

	paint->set_color(0x8F);
	paint->move_pen(CLK_X, CLK_Y);
	paint->draw("A");
}

static void frame()
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

	call_out("frame", 0);

	paint = new_object(LWO_PAINTER);
	paint->start(80, 20);
	paint->set_color(0xC);

	do_clock(paint, time);

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
