#include <kotaka/paths.h>
#include <game/paths.h>
#include <status.h>

inherit LIB_BIN;

void main(string args)
{
	int x, y;
	object paint;
	string buffer;

	send_out("Canvas test:\n");

	paint = new_object("~/lwo/paint/painter");
	paint->start(80, 20);
	paint->set_color(0xC);

	send_out(status(ST_TICKS) + " ticks starting.\n");

	for (x = 0; x < 200; x++) {
		paint->move_pen(random(80), random(20));
		paint->set_color(random(16));
		paint->draw("+");
	}

	paint->set_color(0x2);
	paint->move_pen(20, 0);
	paint->draw("+-----------+");
	paint->move_pen(20, 1);
	paint->draw("|     /     |");
	paint->move_pen(20, 2);
	paint->draw("+-----------+");

	buffer = "   " + random(250);
	buffer = buffer[strlen(buffer) - 3 ..];
	buffer += " / 250";

	paint->set_color(0xB);
	paint->move_pen(22, 1);
	paint->draw(buffer);

	paint->set_color(0x2A);
	paint->move_pen(40, 10);
	paint->draw("/");
	paint->move_pen(39, 11);
	paint->draw("//");
	paint->move_pen(38, 12);
	paint->draw("///");
	paint->move_pen(37, 13);
	paint->draw("////");
	paint->move_pen(36, 14);
	paint->draw("/////");

	paint->set_color(0x20);
	paint->move_pen(41, 11);
	paint->draw("\\");
	paint->move_pen(41, 12);
	paint->draw("\\\\");
	paint->move_pen(41, 13);
	paint->draw("\\\\\\");
	paint->move_pen(41, 14);
	paint->draw("\\\\\\\\");

	paint->set_color(0x30);
	paint->move_pen(39, 15);
	paint->draw("|||");
	paint->move_pen(39, 16);
	paint->draw("|||");
	paint->move_pen(39, 17);
	paint->draw("|||");

	send_out(status(ST_TICKS) + " ticks after painting remaining.\n");
	send_out(paint->render_color());
	send_out(status(ST_TICKS) + " ticks after rendering remaining.\n");
}
