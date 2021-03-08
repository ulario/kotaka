inherit "support";
inherit "/lib/string/sprint";

static string bulk_text(object obj)
{
	string buffer;

	buffer = "<span style=\"white-space: no wrap;\">\n";

	buffer += "Mass: " + mixed_sprint(obj->query_mass()) + " kg";

	if (obj->query_mass_absolute()) {
		buffer += " (absolute)<br />\n";
	} else {
		buffer += " (derived, local mass " + mixed_sprint(obj->query_local_mass()) + " kg<br />\n";
	}


	buffer += "Density: " + mixed_sprint(obj->query_density()) + " kg/l";

	if (obj->query_density_absolute()) {
		buffer += " (absolute)<br />\n";
	} else {
		buffer += " (derived, local density " + mixed_sprint(obj->query_local_density()) + " kg/l)<br />\n";
	}

	buffer += "Capacity: " + mixed_sprint(obj->query_capacity()) + " m^3";

	if (obj->query_capacity_absolute()) {
		buffer += " (absolute)<br />\n";
	} else {
		buffer += " (derived, local capacity " + mixed_sprint(obj->query_local_capacity()) + " m^3)<br />\n";
	}

	buffer += "Max mass: " + mixed_sprint(obj->query_max_mass()) + " kg";

	if (obj->query_max_mass_absolute()) {
		buffer += " (absolute)<br />\n";
	} else {
		buffer += " (derived, local max mass " + mixed_sprint(obj->query_local_max_mass()) + " kg)<br />\n";
	}

	if (obj->query_virtual()) {
		buffer += "Virtual<br />\n";
	}

	if (obj->query_flexible()) {
		buffer += "Flexible<br />\n";
	}

	buffer += "</span>\n";

	return oinfobox("Bulk", 3, buffer);
}
