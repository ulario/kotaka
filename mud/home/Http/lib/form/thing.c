inherit "/lib/string/sprint";
inherit "../support";

static string thing_form(object obj)
{
	string buffer;

	buffer = "<h1>Object form</h1>\n";

	buffer += "<form action=\"object.lpc?obj=" + object2string(obj) + "\" method=\"post\">\n";
	buffer += "<p>Mass: <input type=\"text\" name=\"mass\" value=\"" + mixed_sprint(obj->query_mass()) + "\"/></p><br />\n";
	buffer += "<input type=\"submit\" value=\"change mass\" />\n";
	buffer += "</form>\n";

	return buffer;
}
