#include <kotaka/paths/thing.h>

inherit "thing";

static string query_ip();

static string object_text(object obj)
{
	string subbuffer;
	string buffer;
	string name;

	subbuffer = "<p>Object owner: " + obj->query_owner() + "</p>\n";

	if (name = obj->query_object_name()) {
		subbuffer += "<p>Object name: " + name + "</p>\n";
	}

	if (query_ip() == "127.0.0.1") {
		subbuffer += "<p><a href=\"destruct.lpc?obj=" + object2string(obj) + "\">Destruct</p>\n";
	}

	buffer = "<table class=\"page\"><tr><td>\n";
	buffer += oinfobox("Object", 1, subbuffer);

	if (obj <- LIB_THING) {
		buffer += thing_text(obj);

		if (query_ip() == "127.0.0.1") {
			buffer += thing_form(obj);
		}
	}

	buffer += "</td></tr></table>\n";

	return buffer;
}
