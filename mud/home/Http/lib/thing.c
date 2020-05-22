inherit "support";
inherit "bulk";

static string thing_text(object obj)
{
	mapping lprops;
	object arch;
	object env;
	object *inv;
	int sz;
	string buffer;

	buffer = "<h2>Thing</h2>";

	arch = obj->query_archetype();

	if (arch) {
		buffer += "<p>Archetypes: " + object2string(arch) + "</li>\n";
	}

	env = obj->query_environment();

	if (env) {
		buffer += "<p>Environment: " + object2string(env) + "</p>\n";
	}

	inv = obj->query_inventory();
	sz = sizeof(inv);

	if (sz) {
		int i;

		buffer += "<p>Inventory:</p>\n";
		buffer +="<ul>\n";

		for (i = 0; i < sz; i++) {
			buffer += "<li>" + object2string(inv[i]) + "</li>\n";
		}

		buffer += "</ul>\n";
	}

	buffer += bulk_text(obj);

	return buffer;
}
