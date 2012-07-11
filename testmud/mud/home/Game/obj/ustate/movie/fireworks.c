#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>
#include <status.h>

inherit "~/lib/animate";

float **particles;

#define NPARTICLES 30

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

void begin()
{
	int i;

	::begin();

	send_out("\033[1;1H\033[2J");

	particles = allocate(NPARTICLES);

	for (i = 0; i < NPARTICLES; i++) {
		particles[i] = allocate_float(4);
		particles[i][1] = 20.0;
	}
}

private void do_particles(object paint, float diff)
{
	int x, y, i;

	for (i = 0; i < NPARTICLES; i++) {
		float ovy, nvy;
		mixed *particle;

		particle = particles[i];

		ovy = particle[3];
		nvy = ovy + diff * 20.0;

		particle[0] += particle[2] * diff;
		particle[1] += (ovy + nvy) * 0.5 * diff;
		particle[3] = nvy;

		if (particle[1] >= 20.0) {
			particle[0] = 20.0;
			particle[1] = 20.0;

			particle[2] = (float)random(1000) / 1000.0 * 40.0 - 10.0;
			particle[3] = (float)random(1000) / 1000.0 * 20.0 - 30.0;
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

static void do_frame(float diff)
{
	object paint;

	paint = new_object(LWO_PAINTER);
	paint->start(80, 20);

	do_particles(paint, diff);

	send_out("\033[1;1H");
	send_out(implode(paint->render_color(), "\n"));
}
