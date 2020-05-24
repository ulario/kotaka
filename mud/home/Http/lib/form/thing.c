inherit "/lib/string/sprint";
inherit "../support";

static string thing_form(object obj)
{
	string buffer;

	buffer = "<p>Fun little boxes:</p>\n";

	buffer += "<form action=\"object.lpc?obj=" + object2string(obj) + "\" method=\"post\">\n";
	buffer += "Mass: <input type=\"text\" name=\"mass\" value=\"" + mixed_sprint(obj->query_mass()) + "\"/>\n";
	buffer += "<input type=\"submit\" value=\"change mass\" />\n";
	buffer += "</form>\n";

	buffer += "<form action=\"object.lpc?obj=" + object2string(obj) + "\" method=\"post\">\n";
	buffer += "Local mass: <input type=\"text\" name=\"localmass\" value=\"" + mixed_sprint(obj->query_local_mass()) + "\"/>\n";
	buffer += "<input type=\"submit\" value=\"change local mass\" />\n";
	buffer += "</form>\n";

	return oinfobox("Configuration", 2, buffer);
}
