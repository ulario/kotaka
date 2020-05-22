inherit "support";
inherit "bulk";
inherit "/lib/string/sprint";

static string thing_text(object obj)
{
	object arch;
	object env;
	object *inv;
	string *names;
	mixed *values;
	int sz;

	mapping props;

	string buffer;

	buffer = "<h2>Thing</h2>";

	arch = obj->query_archetype();

	if (arch) {
		buffer += "<p>Archetypes: " + object2link(arch) + "</li>\n";
	}

	env = obj->query_environment();

	if (env) {
		buffer += "<p>Environment: " + object2link(env) + "</p>\n";
	}

	inv = obj->query_inventory();
	sz = sizeof(inv);

	if (sz) {
		int i;

		buffer += "<p>Inventory:</p>\n";
		buffer +="<ul>\n";

		for (i = 0; i < sz; i++) {
			buffer += "<li>" + object2link(inv[i]) + "</li>\n";
		}

		buffer += "</ul>\n";
	}

	inv = obj->query_inventory();
	sz = sizeof(inv);

	if (sz) {
		int i;

		buffer += "<p>Inventory:</p>\n";
		buffer +="<ul>\n";

		for (i = 0; i < sz; i++) {
			buffer += "<li>" + object2link(inv[i]) + "</li>\n";
		}

		buffer += "</ul>\n";
	}

	buffer += bulk_text(obj);

	props = obj->query_local_properties();
	names = map_indices(props);
	values = map_values(props);
	sz = sizeof(names);

	if (sz) {
		int i;

		buffer += "<h3>Local properties</h3>\n";
		buffer += "<dl>\n";

		for (i = 0; i < sz; i++) {
			buffer += "<dt>" + names[i] + "</dt><dd>" + hybrid_sprint(values[i]) + "</dd>\n";
		}

		buffer += "</dl>\n";
	}

	return buffer;
}
