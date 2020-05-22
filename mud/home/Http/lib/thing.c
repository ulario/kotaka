inherit "support";
inherit "bulk";
inherit "form/thing";
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
	string subbuffer;

	arch = obj->query_archetype();

	subbuffer = "";

	if (arch) {
		subbuffer += "<p>Archetype: " + object2link(arch) + "</p>\n";
	}

	env = obj->query_environment();

	if (env) {
		subbuffer += "<p>Environment: " + object2link(env) + "</p>\n";
	}

	buffer = oinfobox("Thing", 2, subbuffer);

	inv = obj->query_inventory();
	sz = sizeof(inv);

	if (sz) {
		int i;
		string subbuffer;

		subbuffer = "";

		for (i = 0; i < sz; i++) {
			subbuffer += "<p>" + object2link(inv[i]) + "</p>\n";
		}

		buffer += oinfobox("Inventory", 3, subbuffer);
	}

	buffer += bulk_text(obj);

	props = obj->query_local_properties();
	names = map_indices(props);
	values = map_values(props);
	sz = sizeof(names);

	buffer += "</td><td>\n";

	if (sz) {
		int i;
		string subbuffer;

		subbuffer = "<dl>\n";

		for (i = 0; i < sz; i++) {
			subbuffer += "<dt>" + names[i] + "</dt><dd>" + hybrid_sprint(values[i]) + "</dd>\n";
		}

		subbuffer += "</dl>\n";
		buffer += oinfobox("Properties", 3, subbuffer);
	}

	return buffer;
}
