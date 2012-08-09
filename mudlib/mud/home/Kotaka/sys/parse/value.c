#include <kotaka/paths.h>

#define CONFIGD "foo"

string grammar;

static void create()
{
	::create();
	
	grammar = read_file("~/data/parse/value.dpd");
}

static void upgraded()
{
	::create();

	grammar = read_file("~/data/parse/value.dpd");
}

mixed parse(string input)
{
	mixed *ret;
	
	ret = parse_string(grammar, input);
	
	if (!ret)
		error("Parse failure");
	
	return ret[0];
}

static string *parse_str(string *input)
{
	string str;
	
	str = input[0];
	str = str[1 .. strlen(str) - 2];
	
	str = STRINGD->replace(str, "\\t", "\t");
	str = STRINGD->replace(str, "\\n", "\n");
	str = STRINGD->replace(str, "\\\"", "\"");
	str = STRINGD->replace(str, "\\\\", "\\");
	
	return ({ str });
}

static int *parse_int(string *input)
{
	return ({ (int)input[0] });
}

static float *parse_flt(string *input)
{
	return ({ (float)input[0] });
}

static object *parse_obj(string *input)
{
	string oname, osubname;
	string *parts;
	object root, obj;
	
	oname = input[0];
	oname = oname[1 .. strlen(oname) - 2];	/* strip off angle brackets */

	if (function_object("parse_object", previous_object(1))) {
		return ({ previous_object(1)->parse_object(oname) });
	}

	parts = explode(oname, ":") - ({ "" });

	if (sizeof(parts) == 0) {
		error("Badly formatted object designation: <" + oname + ">");
	}

	if (parts[0][0] == '/') {
		string osubname;
		
		root = find_object(parts[0]);
		
		if (!root)
			error(parts[0] + ": no such object");
	} else if (parts[0] != "ROOT") {
		error("Unspecified base for object");
	} else {
		root = CONFIGD->query_object_root();
	}

	if (sizeof(parts) > 1) {
		osubname = implode(parts[1 ..], ":");
		obj = root->locate_object(osubname);
		return ({ obj });
	} else {
		return ({ root });
	}
}

static mixed *parse_nil(string *input)
{
	return ({ nil });
}

static mixed *pick_middle(mixed *input)
{
	if (sizeof(input) % 2 != 1)
		error("Illegal input");
	return input[sizeof(input) / 2 .. sizeof(input) / 2];
}

static mixed **make_arr(mixed *input)
{
	return ({ input });
}

static mixed **make_empty_arr(string *input)
{
	return ({ ({ }) });
}

static mixed *build_arr(mixed *input)
{
	return ({ input[0] + ({ input[2] }) });
}

static mapping *make_map(mixed *input)
{
	return ({ ([ input[0] : input[1] ]) });
}

static mapping *make_empty_map(string *input)
{
	return ({ ([ ]) });
}

static mapping *build_map(mixed *input)
{
	return ({ input[0] + ([ input[2] : input[3] ]) });
}

static mixed *strike_middle(mixed *input)
{
	return ({ input[0], input[sizeof(input) - 1] });
}
