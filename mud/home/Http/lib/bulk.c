inherit "support";
inherit "/lib/string/sprint";

static string bulk_text(object obj)
{
	string buffer;

	buffer = "<h3>Bulk</h3>";

	if (obj->query_virtual()) {
		buffer += "<p>Virtual</p>\n";
	} else {
		buffer += "<p>Mass: " + mixed_sprint(obj->query_mass()) + " kg</p>\n";
		buffer += "<p>Density: " + mixed_sprint(obj->query_density()) + " kg/l</p>\n";
		buffer += "<p>Capacity: " + mixed_sprint(obj->query_capacity()) + " m^3</p>\n";
		buffer += "<p>Max mass: " + mixed_sprint(obj->query_max_mass()) + " kg</p>\n";
	}

	return buffer;
}
