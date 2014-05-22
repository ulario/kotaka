/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/ansi.h>
#include <kotaka/privilege.h>

inherit "~/lib/animate";

float **particles;

#define NPARTICLES 100

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

	send_out("\033[1;1H\033[2J");

	particles = allocate(NPARTICLES);

	for (i = 0; i < NPARTICLES; i++) {
		particles[i] = allocate_float(4);
		particles[i][1] = 25.0;
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
		nvy = ovy + diff * 40.0;

		particle[0] += particle[2] * diff;
		particle[1] += (ovy + nvy) * 0.5 * diff;
		particle[3] = nvy;

		if (particle[1] >= 25.0) {
			particle[0] = 20.0;
			particle[1] = 25.0;

			particle[2] = MATHD->bell_rnd(2) * 80.0 - 30.0;
			particle[3] = MATHD->bell_rnd(2) * 40.0 - 60.0;
		}

		x = (int)floor(particle[0]);
		y = (int)floor(particle[1]);
		paint->move_pen(x, y);

		switch(i % 7) {
		case 0: paint->set_color(0x8f); break;
		case 1: paint->set_color(0x87); break;
		case 2: paint->set_color(0x88); break;
		case 3: paint->set_color(0x81); break;
		case 4: paint->set_color(0x89); break;
		case 5: paint->set_color(0x83); break;
		case 6: paint->set_color(0x8b); break;
		}

		paint->draw("+");
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

	do_particles(gc, diff);

	send_out("\033[1;1H");
	send_out(implode(paint->render_color(), "\n"));
}
