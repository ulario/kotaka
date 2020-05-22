#include <kotaka/paths/thing.h>

inherit "thing";

static string object_text(object obj)
{
	string buffer;
	string name;

	buffer = "<h1>Object</h2>";
	buffer += "<p>Object owner: " + obj->query_owner() + "</p>\n";

	if (name = obj->query_object_name()) {
		buffer += "<p>Object name: " + name + "</p>\n";
	}

	if (obj <- LIB_THING) {
		buffer += thing_text(obj);
	}

	return buffer;
}
