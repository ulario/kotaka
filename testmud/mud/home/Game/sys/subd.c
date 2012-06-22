#include <kotaka/paths.h>
/* drawing the feedback screen: */

/*
we limit ourselves to 80 x 20

The 5 remaining lines are reserved for player I/O

first three rows are for header information

Then a 17 x 17 overhead view on the left

Then on the right we have a flowing paragraph describing the current "room"

Then another paragraph with an inventory listing.
*/

#define XM (80 - 17 - 1)
#define YM (2)

string draw_look(object living)
{
	int x, y, i;
	object painter;
	object environment;
	object *contents;

	painter = new_object(LWO_PAINTER);

	/* be really simple for now */
	painter->start(80, 20);

	if (!living) {
		painter->set_color(0x4C);
		for (i = 0; y < 17; y++) {
			painter->move_pen(XM, YM + y);
			painter->draw(STRINGD->chars('?', 17));
		}
	} else if (!(environment = living->query_environment())) {
		painter->set_color(0x7);

		for (i = 0; y < 17; y++) {
			painter->move_pen(XM, YM + y);
			painter->draw(STRINGD->chars(':', 17));
		}
	} else {
		painter->set_color(0x20);

		for (y = 0; y < 17; y++) {
			painter->move_pen(XM, YM + y);
			painter->draw(STRINGD->chars('`', 17));
		}
	}

	for (i = 0; i < 17; i += 1) {
		if (i % 4 == 0) {
			painter->set_color(0x8F);
		} else {
			painter->set_color(0x88);
		}

		painter->move_pen(XM + i, YM - 1);
		painter->draw("|");
		painter->move_pen(XM - 1, YM + i);
		painter->draw("-");
	}

	painter->set_color(0x8B);
	if (environment) {
		float ox, oy;
		int sz;

		ox = living->query_x_position();
		oy = living->query_y_position();

		contents = environment->query_inventory() - ({ living });
		sz = sizeof(contents);

		for (i = 0; i < sz; i++) {
			float dx, dy;
			object neighbor;

			neighbor = contents[i];

			dx = neighbor->query_x_position() - ox;
			dy = neighbor->query_y_position() - oy;

			if (dx < -10.0 || dx > 10.0 || dy < -10.0 || dy > 10.0) {
				continue;
			}

			x = (int)dx + 8;
			y = (int)dy + 8;

			if (x < 0 || x > 16 || y < 0 || y > 16) {
				continue;
			}

			painter->move_pen(XM + x, YM + y);
			painter->draw("X");
		}
	}

	if (living) {
		painter->move_pen(XM + 8, YM + 8);
		painter->set_color(0x8F);
		painter->draw("@");
	}

	painter->set_color(0x07);
	painter->move_pen(0, 0);
	painter->draw("Ularian Forest");

	return painter->render_color();
}
