inherit "support";
inherit "/lib/string/sprint";

static string bulk_text(object obj)
{
	string buffer;

	if (obj->query_virtual()) {
		buffer = "Virtual<br />\n";
	} else {
		buffer = "Mass: " + mixed_sprint(obj->query_mass()) + " kg<br />\n";
		buffer += "Density: " + mixed_sprint(obj->query_density()) + " kg/l<br />\n";
		buffer += "Capacity: " + mixed_sprint(obj->query_capacity()) + " m^3<br />\n";
		buffer += "Max mass: " + mixed_sprint(obj->query_max_mass()) + " kg<br />\n";
	}

	return oinfobox("Bulk", 3, buffer);
}
