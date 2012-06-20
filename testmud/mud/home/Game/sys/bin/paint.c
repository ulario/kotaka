#include <kotaka/paths.h>
#include <game/paths.h>
#include <status.h>

inherit LIB_BIN;

void main(string args)
{
	int x, y;
	object paint;
	string buffer;
	string *lines;

	paint = new_object(LWO_PAINTER);
	paint->start(80, 20);
	paint->set_color(0xC);
	paint->move_pen(0, 0);
	paint->draw("-----------------");
	paint->move_pen(0, 1);
	paint->draw("- Ularian Woods -");
	paint->move_pen(0, 2);
	paint->draw(STRINGD->chars(80, '-'));

	paint->set_color(0x23);
	for (y = 3; y < 20; y++) {
		paint->move_pen(0, y);
		paint->draw(";;;;;;;;;;;;;;;;;");
	}

	paint->set_color(0x8F);
	paint->move_pen(8, 11);
	paint->draw("@");

	lines = explode(STRINGD->wordwrap("Outdoors in a misty forest surrounded by greenery.  Mosses, grasses, and leaves litter the ground, and many kinds of trees are growing, some with bunches of cute brown mushrooms at their feet.", 50), "\n");
	paint->set_color(0x87);

	for (y = 0; y < sizeof(lines); y++) {
		paint->move_pen(25, y + 5);
		paint->draw(lines[y]);
	}

	send_out(paint->render_color());
}
