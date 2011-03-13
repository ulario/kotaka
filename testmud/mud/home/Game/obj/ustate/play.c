#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/constant.h>
#include <kotaka/assert.h>

#include <game/paths.h>

inherit LIB_USTATE;

int stopped;
int reading;
int introed;

int born;
int alive;

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
	if (alive) {
		send_out("(\033[32mbody\033[37m) > ");
	} else if (born) {
		send_out("(\033[31mspirit\033[37m) > ");
	} else {
		send_out("(\033[34mspirit\033[37m) > ");
	}
}

static void begin()
{
	float angle;
	float radius;
	object spirit;
	
	send_out("Linking...done\n");
	
	body = clone_object("~/obj/demo/monster/wolf");
	body->set_id_base("wolf");
	body->move(find_object(ROOT));

	spirit = clone_object("~/obj/demo/monster/spirit");
	spirit->set_id_base("spirit");
	spirit->move(body);

	ASSERT(spirit->query_environment() == body);

	body->set_property("angle", SUBD->rnd() * 360.0);
	angle = SUBD->rnd() * PI * 2.0;
	radius = sqrt(SUBD->rnd()) * 20.0;

	body->set_property("position:x", radius * sin(angle));
	body->set_property("position:y", radius * cos(angle));
}

static void stop()
{
	stopped = 1;
}

static void go()
{
	stopped = 0;

	if (!reading) {
		prompt();
	}
}

static void self_destruct()
{
	destruct_object(this_object());
}

static void end()
{
	object spirit;
	object *inv;
	int i, sz;
	send_out("Exiting biolink.\n");

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

	destruct_object(body->find_by_id("spirit"));
	destruct_object(body);

	call_out("self_destruct", 5);
}

private void scan_world()
{
	object root;
	object *kids;
	
	int i;
	int sz;
	
	root = find_object(ROOT);
	
	kids = root->query_inventory();
	kids -= ({ body });
	sz = sizeof(kids);
	
	for (i = 0; i < sz; i++) {
		object obj;
		float x, y, z;
		
		obj = kids[i];
		
		x = obj->query_property("position:x");
		y = obj->query_property("position:y");
		z = obj->query_property("position:z");
		send_out("Object " + object_name(obj) + " located at (" +
			x + ", " + y + ", " + z + ")\n");
	}
}

private void do_help()
{
	send_out(read_file("~/data/doc/guest_help"));
}

private void do_look()
{
	scan_world();
}

static void receive_in(string input)
{
	string first;

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

	case "help":
		{
			object help;
			
			help = clone_object("help");

			if (input != "") {
				help->set_args( ({ input }) );
			}

			push_state(help);
		}
		break;

	case "move":
		if (!alive) {
			if (!born) {
				send_out("You float around in your office at The Great Hall.\n");
			} else {
				send_out("You attempt to move away from your body,\nbut the Grim Reaper gives your spirit\na threatening stare that stills you...\nThe Grim Reaper speaks:\"\033[31mLeave ye not...lest ye find no shelter.\033[0m\"");
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
	case "end":
		pop_state();
		return;
	case "quit":
		query_user()->quit();
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

static void receive_out(string output)
{
	send_out("[outgoing] " + output);
}
