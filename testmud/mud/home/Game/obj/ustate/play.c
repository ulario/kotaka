#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/constant.h>
#include <kotaka/assert.h>

#include <game/paths.h>

inherit LIB_USTATE;

int stopped;
int reading;
int introed;
int facing;

object body;

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

private void prompt()
{
	if (body) {
		switch(body->query_id_base()) {
		case "deer":
			send_out("(\033[1;37mwhite-tailed \033[0;33mdeer\033[0m) > ");
			break;
		case "wolf":
			send_out("(\033[1;30mgray wolf\033[0m) > ");
			break;
		default:
			send_out("(rock) > ");
		}
	} else {
		send_out("(no character) > ");
	}
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	prompt();
	reading = 1;
}

void stop()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 1;
}

void go()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 0;

	if (!reading) {
		prompt();
	}
}

static void self_destruct()
{
	destruct_object(this_object());
}

private void do_help()
{
	send_out(read_file("~/data/doc/guest_help"));
}

void receive_in(string input)
{
	string first, rest;

	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (!sscanf(input, "%s %s", first, rest)) {
		first = input;
		rest = "";
	}

	switch(first) {
	case "quit":
		pop_state();
		return;
	case "inhabit":
		if (body) {
			send_out("You already have a body.\n");
		} else {
			object *inv;
			int sz;

			inv = GAME_ROOT->query_inventory();
			sz = sizeof(inv);

			if (!sz) {
				send_out("There are no objects for you to inhabit.\n");
			} else {
				body = inv[random(sz)];

				switch(body->query_id_base()) {
				case "wolf":
					send_out("You are now a shaggy furred wolf with sharp teeth.\n");
					break;
				case "deer":
					send_out("You are now a sleek haired deer.\n");
					break;
				case "rock":
					send_out("You are now a granite rock.\n");
					break;
				}
			}
		}
		break;
	case "uninhabit":
		if (!body) {
			send_out("You do not presently have a body.\n");
		} else {
			body = nil;
			send_out("You step out of the object.\n");
		}
		break;
	case "face":
		if (sscanf(rest, "%d", facing)) {
			send_out("You are now facing " + facing + " degrees clockwise from due north.\n");
		} else {
			send_out("I need a number for that.\n");
		}
		break;
	case "forward":
		{
			float distance;

			if (sscanf(rest, "%f", distance)) {
				float sin, cos;

				sin = sin((float)facing * SUBD->pi() / 180.0);
				cos = cos((float)facing * SUBD->pi() / 180.0);

				body->set_x_position(body->query_x_position() + sin * distance);
				body->set_y_position(body->query_y_position() - cos * distance);

				send_out("You proceed " + distance + " meters forward.\n");
			}
		}
		break;
	case "look":
		if (!body) {
			send_out("Lacking a body, you have no eyes to see with.\n");
		} else {
			string buffer;

			buffer = GAME_SUBD->draw_look(body, facing);

			send_out("Currently facing " + facing + " degrees clockwise of due north.\n");
			send_out("You see:\n");
			send_out(buffer);
		}
		break;
	default:
		if (!VERBD->do_action(first, body, input)) {
			send_out("The art of " + first + "ing is unknown to you.\n");
		}
		break;
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}
