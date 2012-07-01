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
float **particles;
string path;

#define NPARTICLES 36
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

	particles = allocate(NPARTICLES);

	for (i = 0; i < NPARTICLES; i++) {
		particles[i] = allocate_float(4);
		particles[i][1] = 20.0;
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

private void do_particles(object paint, float diff)
{
	int x, y, i;

	for (i = 0; i < NPARTICLES; i++) {
		float ovy, nvy;
		mixed *particle;

		particle = particles[i];

		ovy = particle[3];
		nvy = ovy + diff * 80.0;

		particle[0] += particle[2] * diff;
		particle[1] += (ovy + nvy) * 0.5 * diff;
		particle[3] = nvy;

		if (particle[1] >= 20.0) {
			particle[0] = 20.0;
			particle[1] = 20.0;

			particle[2] = (float)random(1000) / 1000.0 * 80.0 - 20.0;
			particle[3] = (float)random(1000) / 1000.0 * 40.0 - 60.0;
		}

		x = (int)floor(particle[0]);
		y = (int)floor(particle[1]);
		paint->move_pen(x, y);

		switch(i % 6) {
		case 0: paint->set_color(0x89); break;
		case 1: paint->set_color(0x8B); break;
		case 2: paint->set_color(0x8A); break;
		case 3: paint->set_color(0x8E); break;
		case 4: paint->set_color(0x8C); break;
		case 5: paint->set_color(0x8D); break;
		}

		paint->draw("+");
	}
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

	do_particles(paint, diff);

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
