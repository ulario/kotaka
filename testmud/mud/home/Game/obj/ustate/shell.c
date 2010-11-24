#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>

inherit LIB_USTATE;

int stopped;
int reading;
int introed;

int superuser;

void exec(object new_shell)
{
	ACCESS_CHECK(GAME());
	
	swap_state(new_shell);
}

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
	if (!introed) {
		send_out(read_file("~/data/doc/guest_welcome"));
		introed = 1;
	}
	send_out("[\033[1;31mU\033[33ml\033[32ma\033[36mr\033[34mi\033[35mo\033[0m] ");
}

static void begin()
{
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

static void end()
{
	send_out("Goodbye.\n");
	
	destruct_object(this_object());
}

private void do_help()
{
	send_out(read_file("~/data/doc/guest_help"));
}

private void scan_world()
{
	object root;
	object *kids;
	
	int i;
	int sz;
	
	root = find_object(ROOT);
	
	kids = root->query_inventory();
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

static void receive_in(string input)
{
	string first;

	reading = 1;

	if (!sscanf(input, "%s %s", first, input)) {
		first = input;
		input = "";
	}

	switch(first) {
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
	case "play":
		push_state(clone_object("play"));
		break;
	case "statwatch":
		push_state(clone_object("status"));
		break;
	case "reloadhelp":
		GAME_HELPD->load();
		break;
	case "worldscan":
		scan_world();
		break;
	case "quit":
		query_user()->quit();
		return;
	case "login":
		push_state(clone_object("login"));
		break;
	case "krecompile":
		OBJECTD->klib_recompile();
		break;
	case "recompile":
		OBJECTD->global_recompile();
		break;
	case "register":
		push_state(clone_object("register"));
		break;
	case "stop":
		push_state(clone_object("stopper"));
		break;
	case "flash":
		{
			object stopper;
		
			stopper = clone_object("stopper");
			push_state(stopper);
			stopper->abort();
		}
		break;
	case "":
		break;
	default:
		send_out("Huh? \n");
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
