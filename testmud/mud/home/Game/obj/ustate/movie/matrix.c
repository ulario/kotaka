#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>
#include <status.h>

inherit "~/lib/animate";

float **particles;
float speed;

#define NPARTICLES 200

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

	speed = 5.0;
	send_out("\033[1;1H\033[2J");

	particles = allocate(NPARTICLES);

	for (i = 0; i < NPARTICLES; i++) {
		float *particle;
		particle = particles[i] = allocate_float(3);

		particle[2] = SUBD->rnd() * 10.0 + 1.0;
		particle[0] = (SUBD->rnd() * 80.0 - 40.0) * particle[2];
		particle[1] = (SUBD->rnd() * 400.0 - 80.0) * particle[2];
	}
}

int zcomp(mixed *a, mixed *b)
{
	if (a[2] > b[2]) {
		return 1;
	}
}

private void do_particles(object paint, float diff)
{
	int x, y, i, sortflag;

	for (i = 0; i < NPARTICLES; i++) {
		mixed *particle;

		particle = particles[i];
		particle[1] += diff * 50.0;

		if (particle[1] > 100.0) {
			particle[2] = SUBD->rnd() * 10.0 + 1.0;
			particle[0] = (SUBD->rnd() * 80.0 - 40.0) * particle[2];
			particle[1] = (SUBD->rnd() * 20.0 - 40.0) * particle[2];
		}
	}

	sortflag = 1;

	if (sortflag) {
		SUBD->qsort(particles, 0, NPARTICLES, "zcomp");
	}

	for (i = NPARTICLES - 1; i >= 0; i--) {
		mixed *particle;
		float depth;

		particle = particles[i];
		depth = particle[2];

		x = (int)floor(particle[0] / depth + 40.0);
		y = (int)floor(particle[1] / depth + 12.5);

		paint->move_pen(x, y);

		if (SUBD->rnd() * 10.0 - depth > 0.0) {
			paint->set_color(0xA);
			paint->draw("" + (i % 2));
		} else {
			paint->set_color(0x2);
			paint->draw("" + (i % 2));
		}
	}
}

static void do_frame(float diff)
{
	object paint;

	paint = new_object(LWO_PAINTER);
	paint->start(80, 25);
	paint->set_color(0xF);

	do_particles(paint, diff);

	send_out("\033[1;1H");
	send_out(implode(paint->render_color(), "\n"));
}
