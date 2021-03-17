inherit "support";
inherit "bulk";
inherit "detail";
inherit "form/thing";
inherit "/lib/string/sprint";

static string thing_text(object obj)
{
	int sz;
	mapping props;
	mixed *values;
	object arch, env, *inv, po, pe;
	string buffer, *details, *names, prep, prox, subbuffer;

	subbuffer = "<p>\n";

	if (arch = obj->query_archetype()) {
		subbuffer += "Archetype: " + object2link(arch) + "<br />\n";
	}

	if (env = obj->query_environment()) {
		subbuffer += "Environment: " + object2link(env) + "<br />\n";
	}

	if (prox = obj->query_prox()) {
		prep = obj->query_prep();

		if (!prep) {
			prep = "???";
		}

		subbuffer += "Prox: " + prep + " " + prox + "<br />\n";
	}

	if (po = obj->query_possessor()) {
		subbuffer += "Possessed by: " + object2link(po) + "<br />\n";
	}

	if (pe = obj->query_possessee()) {
		subbuffer += "Possesing: " + object2link(pe) + "<br />\n";
	}

	subbuffer += "</p>\n"; 

	if (subbuffer == "<p>\n</p>\n") {
		buffer = "<h2>Thing</h2>\n";
	} else {
		buffer = oinfobox("Thing", 2, subbuffer);
	}

	inv = obj->query_inventory();
	sz = sizeof(inv);

	if (sz) {
		int i;
		string subbuffer;

		subbuffer = "<p>\n";

		for (i = 0; i < sz; i++) {
			subbuffer += object2link(inv[i]) + "<br />\n";
		}

		subbuffer += "</p>\n";

		buffer += oinfobox("Inventory", 3, subbuffer);
	}

	buffer += bulk_text(obj);

	if (sizeof(details = obj->query_details())) {
		buffer += detail_text(obj, details);
	}

	props = obj->query_local_properties();
	names = map_indices(props);
	values = map_values(props);

	sz = sizeof(names);

	if (sz) {
		int i;
		string subbuffer;

		subbuffer = "<dl>\n";

		for (i = 0; i < sz; i++) {
			subbuffer += "<dt>" + names[i] + "</dt><dd>" + hybrid_sprint(values[i]) + "</dd>\n";
		}

		subbuffer += "</dl>\n";
		buffer += oinfobox("Local properties", 2, subbuffer);
	}

	return buffer;
}
