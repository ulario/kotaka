#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/constant.h>
#include <kotaka/assert.h>

#include <game/paths.h>

inherit LIB_USTATE;

int stopped;
int reading;
int introed;

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
	send_out("(\033[32mprimitive human\033[37m) > ");
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	body = GAME_TESTD->conjure_human();
	body->set_user(query_user());

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

void pre_end()
{
	object spirit;
	object *inv;
	int i, sz;

	ACCESS_CHECK(previous_object() == query_user());

	inv = body->query_inventory();

	if (sz = sizeof(inv)) {
		send_out("Inventory listing:\n\n");

		for (sz = sizeof(inv), i = 0; i < sz; i++) {
			send_out(inv[i]->query_id() + "\n");
		}

		send_out("\nEnd of list.\n");
	} else {
		send_out("No inventory.\n");
	}

	destruct_object(body);

	call_out("self_destruct", 5);
}

private void do_help()
{
	send_out(read_file("~/data/doc/guest_help"));
}

private void do_look()
{
	scan_world();
}

void receive_in(string input)
{
	string first;

	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (!sscanf(input, "%s %s", first, input)) {
		first = input;
		input = "";
	}

	switch(first) {
	case "look":
		if (!alive) {
			if (!born) {
				send_out("You gaze at the serene majesty of alabaster.\n");
			} else {
				send_out("You look around, but everything seems so hazy...\n");
			}
		} else {
			do_look();
		}
		break;

	case "move":
		if (!alive) {
			if (!born) {
				send_out("You float around in your office at The Great Hall.\n");
			} else {
				send_out("You attempt to move away from your body,\nbut the Grim Reaper gives your spirit\na threatening stare that stills you...\nThe Grim Reaper speaks: \"\033[31mLeave ye not...lest ye find no shelter.\033[0m\"\n");
			}
		} else {
			send_out("Not yet implemented.\n");
		}
		break;

	case "die":
		if (alive) {
			alive = 0;
			
			send_out("You die.\n");
			send_out("The Grim Reaper arrives in a wisp of black smoke, scythe in hand\nas he stands over your lifeless body.\n");
		} else if (!born) {
			send_out("You don't even exist yet.\n");
		} else {
			send_out("You are already dead.\n");
		}
		break;

	case "live":
		if (alive) {
			send_out("You are already alive.\n");
		} else {
			alive = 1;
			
			if (born) {
				send_out("A dazzling beam of light from the heavens illuminates your body.\nThe Grim Reaper grumbles, yawns,\ntips his hood to you, and vanishes into the darkness.\n");
			} else {
				born = 1;
				send_out("With a great hazy whoosh over your spirit, your eyes seem to open again, and you find yourself on solid earth for the first time.\n");
			}
		}

		break;

	case "quit":
		pop_state();
		return;

	case "krecompile":
		OBJECTD->klib_recompile();
		break;

	case "recompile":
		OBJECTD->global_recompile();
		break;

	case "":
		break;

	default:
		send_out(first + ": command not recognized.\n");
	}
	
	reading = 0;

	if (!stopped) {
		prompt();
	}
}
