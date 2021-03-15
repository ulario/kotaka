#include <kotaka/paths/system.h>
#include <kotaka/paths/thing.h>

static string hex(int d)
{
	string h;

	h = " ";

	if (d < 10) {
		h[0] = '0' + d;
	} else {
		h[0] = 'A' + (d - 10);
	}

	return h;
}

static int hexval(string hd)
{
	int v;

	v = hd[0];

	switch(v) {
	case '0' .. '9':
		return v - '0';
	case 'A' .. 'F':
		return 10 + v - 'A';
	case 'a' .. 'f':
		return 10 + v - 'a';
	}
}

static string urlencode(string str)
{
	string output;
	int sz, i;

	sz = strlen(str);
	output = "";

	for (i = 0; i < sz; i++) {
		string ch;
		int cv;

		cv = str[i];

		switch(cv) {
		case 'A' .. 'Z':
		case 'a' .. 'z':
		case '0' .. '9':
			ch = " ";
			ch[0] = cv;
			break;

		default:
			ch = "%";
			ch += hex(cv >> 4);
			ch += hex(cv & 15);
		}

		output += ch;
	}

	return output;
}

static string urldecode(string str)
{
	string output;
	string head, hex, tail;

	output = "";

	while (sscanf(str, "%s%%%s", head, tail)) {
		string ch;

		output += head;

		ch = " ";
		ch[0] = (hexval(tail[0 .. 0]) << 4) + hexval(tail[1 .. 1]);

		output += ch;

		str = tail[2 ..];
	}

	output += str;

	return output;
}

static string oinfobox(string header, int level, string content)
{
	string buf;

	buf = "<div class=\"infobox\">\n";
	buf += "<h" + level + ">" + header + "</h" + level + ">\n";

	buf += "<div class=\"infopane\">\n";
	buf += content;
	buf += "</div>\n";
	buf += "</div>\n";

	return buf;
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

	return "<a href=\"object.lpc?obj=" + urlencode(str) + "\">" + str + "</a>";
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
