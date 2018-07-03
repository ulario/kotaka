/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kotaka/paths/utility.h>
#include <kotaka/paths/ansi.h>
#include <kotaka/privilege.h>

inherit "~/lib/animate";

int nparticles;
float **particles;
float speed;

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

private void set_nparticles()
{
	nparticles = screen_width * screen_height / 100;
}

private void reset_particle(float *particle)
{
	particle[2] = MATHD->rnd() * 10.0 + 1.0;

	particle[0] = (MATHD->rnd() - 0.5) * (float)(screen_width) * particle[2];
	particle[1] = -((float)(screen_height) * 0.5 + MATHD->rnd()) * particle[2];
}

private void initialize_particle(float *particle)
{
	/* particles reset when they hit the bottom of the screen, not the bottom of the field */
	/* add a statistical bias so that particles start out evenly spread */

	do {
		particle[2] = (1.0 - pow(MATHD->rnd(), 2.0)) * 11.0;
	} while (particle[2] < 1.0);

	particle[0] = (MATHD->rnd() - 0.5) * (float)(screen_width) * particle[2];
	particle[1] = (MATHD->rnd() - 0.5) * (float)(screen_height) * particle[2];
}

void begin()
{
	int i;

	ACCESS_CHECK(previous_object() == query_user());

	::begin();

	speed = 5.0;
	send_out("\033[1;1H\033[2J");

	check_screen();
	set_nparticles();

	particles = allocate(nparticles);

	for (i = 0; i < nparticles; i++) {
		float *particle;
		particle = particles[i] = allocate_float(3);

		initialize_particle(particle);
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

	for (i = 0; i < nparticles; i++) {
		mixed *particle;

		particle = particles[i];
		particle[1] += diff * 50.0;

		if (particle[1] / particle[2] > (float)(screen_height) * 0.5) {
			reset_particle(particle);
		}
	}

	sortflag = 1;

	if (sortflag) {
		SORTD->qsort(particles, 0, nparticles, "zcomp");
	}

	for (i = nparticles - 1; i >= 0; i--) {
		mixed *particle;
		float depth;

		particle = particles[i];
		depth = particle[2];

		x = (int)floor(particle[0] / depth + (float)(screen_width) * 0.5);
		y = (int)floor(particle[1] / depth + (float)(screen_height) * 0.5);

		paint->move_pen(x, y);

		if (depth <= 2.5) {
			paint->set_color(0xF);
		} else if (depth <= 6.25) {
			paint->set_color(0xA);
		} else if (depth <= 10.0) {
			paint->set_color(0x2);
		} else {
			paint->set_color(0x8);
		}
		paint->draw("" + (i % 2));
	}
}

static void do_frame(float diff)
{
	object paint;
	object gc;

	paint = new_object(LWO_PAINTER);
	paint->set_size(screen_width, screen_height);
	paint->add_layer("default");
	paint->set_layer_size("default", screen_width, screen_height);

	gc = paint->create_gc();
	gc->set_layer("default");
	gc->set_clip(0, 0, screen_width - 1, screen_height - 1);

	do_particles(gc, diff);

	send_out("\033[1;1H");
	send_out(implode(paint->render_color(), "\n"));
}

void end()
{
	destruct_object(this_object());
}
