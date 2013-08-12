/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <kotaka/paths/algorithm.h>
#include <kotaka/paths/kotaka.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>

inherit "~/lib/animate";

float **particles;
float speed;

#define NPARTICLES 150

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

	ACCESS_CHECK(previous_object() == query_user());

	::begin();

	speed = 5.0;
	send_out("\033[1;1H\033[2J");

	particles = allocate(NPARTICLES);

	for (i = 0; i < NPARTICLES; i++) {
		particles[i] = allocate_float(3);
		particles[i][0] = MATHD->rnd() * 800.0 - 400.0;
		particles[i][1] = MATHD->rnd() * 250.0 - 125.0;
		particles[i][2] = MATHD->rnd() * 10.0 + 1.0;
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
		particle[2] -= diff * 5.0;

		if (particle[2] < 1.0) {
			particle[0] = MATHD->rnd() * 800.0 - 400.0;
			particle[1] = MATHD->rnd() * 250.0 - 125.0;
			particle[2] = MATHD->rnd() * 1.0 + 10.0;
			sortflag = 1;
		}
	}

	if (sortflag) {
		SORTD->qsort(particles, 0, NPARTICLES, "zcomp");
	}

	for (i = NPARTICLES - 1; i >= 0; i--) {
		mixed *particle;
		float depth;

		particle = particles[i];
		depth = particle[2];

		x = (int)floor(particle[0] / depth + 40.0);
		y = (int)floor(particle[1] / depth + 12.5);

		paint->move_pen(x, y);

		if (depth < 5.0) {
			paint->set_color(0x8F);
			paint->draw("@");
		} else if (depth < 7.5) {
			paint->set_color(0x87);
			paint->draw("*");
		} else {
			paint->set_color(0x88);
			paint->draw("+");
		}

	}
}

static void do_frame(float diff)
{
	object paint;
	object gc;

	paint = new_object(LWO_PAINTER);
	paint->set_size(80, 25);
	paint->add_layer("default");
	paint->set_layer_size("default", 80, 25);

	gc = paint->create_gc();
	gc->set_layer("default");
	gc->set_clip(0, 0, 79, 24);
	gc->set_color(0xF);

	do_particles(gc, diff);

	send_out("\033[1;1H");
	send_out(implode(paint->render_color(), "\n"));
}
