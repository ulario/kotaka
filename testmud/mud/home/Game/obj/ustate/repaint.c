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
float **sparks;
string path;

#define NSPARKS 50

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

	call_out("sparkle", 0);
	call_out("second", 1);
	times = millitime();
	otime = (float)times[0] + times[1];

	sparks = allocate(NSPARKS);

	for (i = 0; i < NSPARKS; i++) {
		sparks[i] = allocate_float(4);
		sparks[i][1] = 20.0;
	}
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

	paint->set_color(0x88);
	for (i = 0; i < 12; i++) {
		x = (int)((sin((float)i * pi / 6.0)) * 8.0) + 60;
		y = (int)((-cos((float)i * pi / 6.0)) * 8.0) + 8;

		paint->move_pen(x, y);
		paint->draw("@");
	}

	hand = time / 60.0;
	hand -= floor(hand);

	paint->set_color(0x89);

	for (i = 0; i < 8; i++) {
		x = (int)((sin(hand * pi * 2.0)) * (float)i) + 60;
		y = (int)((-cos(hand * pi * 2.0)) * (float)i) + 8;

		paint->move_pen(x, y);
		paint->draw("S");
	}

	hand = time / 3600.0;
	hand -= floor(hand);

	paint->set_color(0x8A);

	for (i = 0; i < 6; i++) {
		x = (int)((sin(hand * pi * 2.0)) * (float)i) + 60;
		y = (int)((-cos(hand * pi * 2.0)) * (float)i) + 8;

		paint->move_pen(x, y);
		paint->draw("M");
	}

	hand = time / (3600.0 * 12.0);
	hand -= floor(hand);

	paint->set_color(0x8C);

	for (i = 0; i < 4; i++) {
		x = (int)((sin(hand * pi * 2.0)) * (float)i) + 60;
		y = (int)((-cos(hand * pi * 2.0)) * (float)i) + 8;

		paint->move_pen(x, y);
		paint->draw("H");
	}

	paint->set_color(0x8F);
	paint->move_pen(40, 8);
	paint->draw("A");
}

private void do_sparks(object paint, float diff)
{
	int x, y, i;

	for (i = 0; i < NSPARKS; i++) {
		float ovy, nvy;
		mixed *spark;

		spark = sparks[i];

		ovy = spark[3];
		nvy = ovy + diff * 20.0;

		spark[0] += spark[2] * diff;
		spark[1] += (ovy + nvy) * 0.5 * diff;
		spark[3] = nvy;

		if (spark[1] >= 20.0) {
			spark[0] = 10.0;
			spark[1] = 20.0;

			spark[2] = (float)random(1000) / 1000.0 * 40.0 - 10.0;
			spark[3] = (float)random(1000) / 1000.0 * 20.0 - 30.0;
		}

		x = (int)floor(spark[0]);
		y = (int)floor(spark[1]);
		paint->move_pen(x, y);

		switch(i % 5) {
		case 0: paint->set_color(0x89); break;
		case 1: paint->set_color(0x8B); break;
		case 2: paint->set_color(0x8F); break;
		case 3: paint->set_color(0x8E); break;
		case 4: paint->set_color(0x8C); break;
		}

		paint->draw("+");
	}
}

static void sparkle()
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

	call_out("sparkle", 0);
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

	paint->set_color(0xB);
	paint->move_pen(22, 1);
	paint->draw(buffer);

	paint->set_color(0x2A);
	paint->move_pen(40, 10);
	paint->draw("/");
	paint->move_pen(39, 11);
	paint->draw("//");
	paint->move_pen(38, 12);
	paint->draw("///");
	paint->move_pen(37, 13);
	paint->draw("////");
	paint->move_pen(36, 14);
	paint->draw("/////");

	paint->set_color(0x20);
	paint->move_pen(41, 11);
	paint->draw("\\");
	paint->move_pen(41, 12);
	paint->draw("\\\\");
	paint->move_pen(41, 13);
	paint->draw("\\\\\\");
	paint->move_pen(41, 14);
	paint->draw("\\\\\\\\");

	paint->set_color(0x30);
	paint->move_pen(39, 15);
	paint->draw("|||");
	paint->move_pen(39, 16);
	paint->draw("|||");
	paint->move_pen(39, 17);
	paint->draw("|||");

	do_clock(paint, time);
	do_sparks(paint, diff);

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
