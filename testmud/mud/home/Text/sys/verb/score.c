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
#include <account/paths.h>
#include <game/paths.h>
#include <text/paths.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	object user;
	object body;
	object painter;
	object gc;

	int i, j;

	user = query_user();
	body = user->query_body();

	if (!body) {
		send_out("You don't have a body to score.\n");
		return;
	}

	painter = new_object(LWO_PAINTER);
	painter->set_size(40, 12);

	gc = painter->create_gc();
	gc->set_color(0x03);
	gc->set_clip(0, 0, 39, 11);

	for (i = 0; i < 12; i++) {
		gc->move_pen(0, i);
		gc->draw(STRINGD->chars(':', 40));
	}

	gc->set_color(0x0F);
	gc->move_pen(2, 1);
	gc->draw(STRINGD->spaces(36));

	gc->move_pen(3, 1);
	gc->draw(implode(body->query_property("nouns"), ", "));

	for (i = 3; i < 6; i++) {
		gc->move_pen(2, i);
		gc->draw(STRINGD->spaces(36));
	}

	gc->move_pen(3, 3);
	gc->draw("HP:  " + random(10) + "/" + random(10));
	gc->move_pen(3, 4);
	gc->draw("MP:  " + random(10) + "/" + random(10));
	gc->move_pen(3, 5);
	gc->draw("END: " + random(10) + "/" + random(10));

	for (j = 5; j <= 27; j += 11) {
		gc->move_pen(j, 7);
		gc->draw(STRINGD->spaces(9));
		gc->move_pen(j, 7);
		switch (j) {
		case 5:
			gc->draw("Physical");
			break;
		case 16:
			gc->draw("Mental");
			break;
		case 27:
			gc->draw("Social");
			break;
		}
		for (i = 8; i < 11; i++) {
			gc->move_pen(j, i);
			gc->draw(STRINGD->spaces(9));
		}
	}

	send_out(implode(painter->render_color(), "\n") + "\n");
}
