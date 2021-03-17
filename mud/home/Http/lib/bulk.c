inherit "support";
inherit "/lib/string/sprint";

static string bulk_text(object obj)
{
	string buffer;

	if (obj->query_virtual()) {
		buffer = "Virtual<br />\n";
	} else {
		if (obj->query_flexible()) {
			buffer += "Flexible<br />\n";
		}

		buffer = "<span style=\"white-space: no wrap;\">\n";

		buffer += "Mass: " + mixed_sprint(obj->query_mass()) + " kg<br />\n";
		buffer += "Density: " + mixed_sprint(obj->query_density()) + " kg/l<br />\n";
		buffer += "Capacity: " + mixed_sprint(obj->query_capacity()) + " m^3<br />\n";
		buffer += "Max mass: " + mixed_sprint(obj->query_max_mass()) + " kg<br />\n</br />\n";

		if (obj->query_archetype()) {
			buffer += "Local mass: " + mixed_sprint(obj->query_local_mass()) + " kg<br />\n";
			buffer += "Local density: " + mixed_sprint(obj->query_local_density()) + " kg/l<br />\n";
			buffer += "Local capacity: " + mixed_sprint(obj->query_local_capacity()) + " m^3<br />\n";
			buffer += "Local max mass: " + mixed_sprint(obj->query_local_max_mass()) + " kg\n";
		} else {
			buffer += "No archetype\n";
		}

		buffer += "</span>\n";
	}

	return oinfobox("Bulk", 3, buffer);
}
