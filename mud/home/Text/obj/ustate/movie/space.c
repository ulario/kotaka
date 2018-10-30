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

int sortflag;

static void create(int clone)
{
	::create();

	sortflag = 1;
}

static void destruct(int clone)
{
	::destruct();
}

private void set_nparticles()
{
	nparticles = screen_width * screen_height / 50;
}

private void reset_particle_xy(float *particle)
{
	particle[0] = (MATHD->rnd() - 0.5) * (float)(screen_width) * 10.0;
	particle[1] = (MATHD->rnd() - 0.5) * (float)(screen_height) * 10.0;
}

private void reset_particle(float *particle)
{
	reset_particle_xy(particle);
	particle[2] = MATHD->rnd() * 1.0 + 10.0;
}

private void initialize_particle(float *particle)
{
	reset_particle_xy(particle);
	particle[2] = MATHD->rnd() * 10.0 + 1.0;
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
		particles[i] = allocate_float(3);
		initialize_particle(particles[i]);
	}

	sortflag = 1;
}

int zcomp(mixed *a, mixed *b)
{
	if (a[2] > b[2]) {
		return 1;
	}
}

private void do_particles(object paint, float diff)
{
	int x, y, i;

	for (i = 0; i < nparticles; i++) {
		mixed *particle;
		mixed *p2;

		particle = particles[i];

		particle[2] -= diff * 5.0;

		if (particle[2] < 1.0) {
			reset_particle(particle);
			sortflag = 1;
		}
	}

	if (sortflag) {
		sortflag = 0;
		SORTD->qsort(particles, 0, nparticles, "zcomp");
	}

	for (i = nparticles - 1; i >= 0; i--) {
		mixed *particle;
		float depth;

		particle = particles[i];
		depth = particle[2];

		x = (int)floor(particle[0] / depth + (float)(screen_width) / 2.0);
		y = (int)floor(particle[1] / depth + (float)(screen_height) / 2.0);

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
