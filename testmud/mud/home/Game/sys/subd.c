#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <game/paths.h>

/* drawing the feedback screen: */

/*
we limit ourselves to 80 x 20

The 5 remaining lines are reserved for player I/O

first three rows are for header information

Then a 17 x 17 overhead view on the left

Then on the right we have a flowing paragraph describing the current "room"

Then another paragraph with an inventory listing.
*/

#define XM (61)
#define YM (1)

private void draw_tickmarks(object painter)
{
	int i;

	for (i = 0; i < 17; i += 1) {
		if (i % 4 == 0) {
			painter->set_color(0x8F);
		} else {
			painter->set_color(0x88);
		}

		painter->move_pen(XM + i, YM - 1);
		painter->draw("|");
		painter->move_pen(XM + i, YM + 17);
		painter->draw("|");
		painter->move_pen(XM - 1, YM + i);
		painter->draw("-");
		painter->move_pen(XM + 17, YM + i);
		painter->draw("-");
	}
}

string draw_look(object living, varargs int facing)
{
	int x, y, i;
	object painter;
	object environment;
	object *contents;

	ACCESS_CHECK(GAME());

	painter = new_object(LWO_PAINTER);

	/* be really simple for now */
	painter->start(80, 20);

	if (!living) {
		painter->set_color(0x47);
		for (i = 0; y < 17; y++) {
			painter->move_pen(XM, YM + y);
			painter->draw(STRINGD->chars(' ', 17));
		}
		painter->set_color(0x74);
		painter->move_pen(XM + 6, YM + 8);
		painter->draw("Error");
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

	draw_tickmarks(painter);
	painter->set_color(0x03);

	if (environment) {
		float ox, oy;
		int sz;

		float pi, sin, cos;

		ox = living->query_x_position();
		oy = living->query_y_position();

		pi = SUBD->pi();

		sin = sin((float)facing * pi / 180.0);
		cos = cos((float)facing * pi / 180.0);

		contents = environment->query_inventory() - ({ living });
		sz = sizeof(contents);

		for (i = 0; i < sz; i++) {
			float dx, dy;
			float vx, vy;
			object neighbor;

			neighbor = contents[i];

			dx = neighbor->query_x_position() - ox;
			dy = neighbor->query_y_position() - oy;

			vy = cos * dy - sin * dx;
			vx = sin * dy + cos * dx;

			if (vx < -10.0 || vx > 10.0 || vy < -10.0 || vy > 10.0) {
				continue;
			}

			x = (int)vx + 8;
			y = (int)vy + 8;

			if (x < 0 || x > 16 || y < 0 || y > 16) {
				continue;
			}

			painter->move_pen(XM + x, YM + y);

			switch(neighbor->query_id_base()) {
			case "wolf":
				painter->set_color(0x08);
				painter->draw("w");
				break;
			case "deer":
				painter->set_color(0x03);
				painter->draw("d");
				break;
			case "rock":
				painter->set_color(0x07);
				painter->draw("@");
				break;
			case "soil":
				painter->set_color(0x83);
				painter->draw(":");
				break;
			default:
				painter->set_color(0x0D);
				painter->draw("?");
				break;
			}
		}
	}

	if (living) {
		painter->move_pen(XM + 8, YM + 8);
		painter->set_color(0x0F);
		painter->draw("@");
	}

	painter->set_color(0x07);
	painter->move_pen(0, 0);
	painter->draw("Ularian Forest");

	return implode(painter->render_color(), "\n") + "\n";
}

string titled_name(string name, int class)
{
	if (name) {
		STRINGD->to_title(name);
	} else {
		name = "guest";
	}

	switch(class) {
	case 0:
		name = "\033[1;34m" + name + "\033[0m";
		break;
	case 1:
		name = "\033[1;32mMr " + name + "\033[0m";
		break;
	case 2:
		name = "\033[1;33mSir " + name + "\033[0m";
		break;
	case 3:
		name = "\033[1;31mLord " + name + "\033[0m";
		break;
	}

	return name;
}

void send_to_all(string phrase)
{
	int sz;
	object *users;

	ACCESS_CHECK(GAME());

	users = GAME_USERD->query_users();
	users += GAME_USERD->query_guests();

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(phrase);
	}
}

void send_to_all_except(string phrase, object *exceptions)
{
	int sz;
	object *users;

	ACCESS_CHECK(GAME());

	users = GAME_USERD->query_users();
	users += GAME_USERD->query_guests();
	users -= exceptions;

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(phrase);
	}
}
