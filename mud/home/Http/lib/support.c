#include <kotaka/paths/thing.h>
#include <kotaka/paths/system.h>

static string oinfobox(string header, int level, string content)
{
	string buf;

	buf = "<h" + level + ">" + header + "</h" + level + ">\n";
	buf += "<table>\n";
	buf += "<tr>\n";
	buf += "<td class=\"infobox\">\n";
	buf += content;
	buf += "</td>\n";
	buf += "</tr>\n";
	buf += "</table>\n";
}

static string simplename(object obj)
{
	string name;

	name = obj->query_object_name();

	if (name) {
		return name;
	} else {
		return object_name(obj);
	}
}

static string object2string(object obj)
{
	string name;

	name = "";

	if (obj <- LIB_THING) {
		object env;
		string id;

		name = obj->query_object_name();

		if (name) {
			return name;
		}

		env = obj->query_environment();

		if (env) {
			id = obj->query_id();

			if (id) {
				return object2string(env) + ";" + id;
			} else {
				return simplename(obj);
			}
		} else {
			return simplename(obj);
		}
	} else {
		return simplename(obj);
	}
}

static string object2link(object obj)
{
	string str;

	str = object2string(obj);

	return "<a href=\"./object-" + str + "\">" + str + "</a>";
}

static mixed string2object(string str)
{
	string semisuffix;
	object obj;

	if (sscanf(str, "%s;%s", str, semisuffix)) {
		semisuffix = ";" + semisuffix;
	}

	obj = find_object(str);

	if (!obj) {
		obj = IDD->find_object_by_name(str);
	}

	if (!obj) {
		return "Could not find " + str;
	}

	if (semisuffix) {
		string *parts;
		int i;
		int sz;

		parts = explode(semisuffix[1 ..], ";");
		sz = sizeof(parts);

		for (i = 0; i < sz; i++) {
			obj = obj->find_by_id(parts[i]);

			if (!obj) {
				return "Could not find " + parts[i] + " within " + str;
			}

			str += ";" + parts[i];
		}
	}

	return obj;
}
