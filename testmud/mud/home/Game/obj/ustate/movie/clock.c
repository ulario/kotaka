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
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>
#include <status.h>

inherit "~/lib/animate";

float pi;

#define CLK_X 9
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
	ACCESS_CHECK(previous_object() == query_user());

	::begin();

	send_out("\033[1;1H\033[2J");
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

static void do_frame(float diff)
{
	object paint;
	mixed *time;

	paint = new_object(LWO_PAINTER);
	paint->start(80, 20);
	paint->set_color(0xC);

	time = millitime();
	do_clock(paint, (float)time[0] + time[1]);

	send_out("\033[1;1H");
	send_out(implode(paint->render_color(), "\n"));
}
