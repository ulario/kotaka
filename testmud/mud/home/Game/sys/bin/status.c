#include <kotaka/paths.h>
#include <game/paths.h>
#include <status.h>

inherit LIB_BIN;

int percent(int n, int d)
{
	return (int)floor( (float)(n * 100) / (float)d);
}

int color(int percent)
{
	switch(percent) {
	case 0 .. 49:
		return 0xA;
	case 50 .. 79:
		return 0xB;
	case 81 .. 100:
		return 0x9;
	}
}

void main(string args)
{
	object paint;
	string buffer;

	int max;
	int current;

	paint = new_object(LWO_PAINTER);

	paint->start(40, 5);
	paint->set_color(0x0C);
	paint->move_pen(0, 0);
	paint->draw("+--------------------------------------+");
	paint->move_pen(0, 1);
	paint->draw("| Swap:    0123456789 / 0123456789 --- |");
	paint->move_pen(0, 2);
	paint->draw("| Object:  0123456789 / 0123456789 --- |");
	paint->move_pen(0, 3);
	paint->draw("| Callout: 0123456789 / 0123456789 --- |");
	paint->move_pen(0, 4);
	paint->draw("+--------------------------------------+");
	paint->set_color(0xF);
	paint->move_pen(2, 1);
	paint->draw("Swap:");
	paint->move_pen(2, 2);
	paint->draw("Object:");
	paint->move_pen(2, 3);
	paint->draw("Callout:");

	max = status(ST_SWAPSIZE);
	buffer = "          " + max;
	buffer = "/ " + buffer[strlen(buffer) - 10 ..];
	paint->set_color(0xF);
	paint->move_pen(22, 1);
	paint->draw(buffer);

	current = status(ST_SWAPUSED);
	buffer = "          " + current;
	buffer = buffer[strlen(buffer) - 10 ..];
	paint->set_color(color(percent(current, max)));
	paint->move_pen(11, 1);
	paint->draw(buffer);

	max = status(ST_OTABSIZE);
	buffer = "          " + max;
	buffer = "/ " + buffer[strlen(buffer) - 10 ..];
	paint->set_color(0xF);
	paint->move_pen(22, 2);
	paint->draw(buffer);

	current = status(ST_NOBJECTS);
	buffer = "          " + current;
	buffer = buffer[strlen(buffer) - 10 ..];
	paint->set_color(color(percent(current, max)));
	paint->move_pen(11, 2);
	paint->draw(buffer);

	max = status(ST_COTABSIZE);
	buffer = "          " + max;
	buffer = "/ " + buffer[strlen(buffer) - 10 ..];
	paint->set_color(0xF);
	paint->move_pen(22, 3);
	paint->draw(buffer);

	current = status(ST_NCOSHORT) + status(ST_NCOLONG);
	paint->set_color(color(percent(current, max)));
	paint->move_pen(11, 3);
	buffer = "          " + current;
	buffer = buffer[strlen(buffer) - 10 ..];
	paint->draw(buffer);

	send_out(paint->render_color());
}
