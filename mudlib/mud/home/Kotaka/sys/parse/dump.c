#include <kotaka/paths.h>
#include <type.h>

#define CONFIGD "foo"

mapping congs;
/*
we set things up to return things as follows:

nil: ({ 0 })
int: ({ 1, <value> })
flt: ({ 2, <value> })
str: ({ 3, <value> })
obj: ({ 4, <value> })
arr: ({ 5, ({ <array elements> }) })
map: ({ 6, ({ <mapping indices> }), ({ <mapping values> }) })
lwo: ({ 7, path, save value })
dup: ({ 8, dup_number })

Arrays, mappings are kept track of separately so that we can keep
duplicates and self references consistent.

arr and map serve as declarations, and in addition to being processed as
values, will also be cached for later dupes.

subelements of an array or mapping declaration are listed as if they
were top level

So ({ 0, 1, 2 })

Would be listed as:

({ 5, ({ ({ 1, 0 }), ({ 1, 1 }), ({ 1, 2 }) }) })

The 5 introduces an array, and the elements follow.

The three subarrays represent the three elements

*/

string grammar;

private void load_grammar()
{
	grammar = read_file("~/data/parse/dump.dpd");
}

static void create()
{
	load_grammar();
}

static void upgraded()
{
	load_grammar();
}

private mixed build(mixed *input)
{
	mixed out;
	
	switch(input[0]) {
	case T_NIL:
		return nil;
	case T_INT:
	case T_FLOAT:
	case T_STRING:
	case T_OBJECT:
		return input[1];
	case T_ARRAY:
		{
			mixed *elements;
			int i;
			
			elements = input[1];
			out = congs[map_sizeof(congs)] = allocate(sizeof(elements));
			
			for (i = 0; i < sizeof(out); i++) {
				out[i] = build(elements[i]);
			} 
		}
		return out;
	case T_MAPPING:
		{
			mixed *indices;
			mixed *values;
			int i;
			
			indices = input[1];
			values = input[2];
			
			out = congs[map_sizeof(congs)] = ([ ]);
			
			for (i = 0; i < sizeof(indices); i++) {
				out[build(indices[i])] = build(values[i]);
			}
		}
		return out;
	case T_MAPPING + 1:
		{
			mixed value;
			string path;
			
			
			path = input[1];
			
			out = congs[map_sizeof(congs)] =
				CONFIGD->query_game_driver()->make_lwo(path);
			
			value = build(input[2]);
			
			out->load(value);
		}
		return out;
	case 8:
		return congs[input[1]];
	default:
		error("Unimplemented type for this:\n" + STRINGD->tree_sprint(input));
	}
}

mixed parse(string input)
{
	mixed *elements;
	
	congs = ([ ]);

	elements = parse_string(grammar, input);
	
	return build(elements[0]);
}

static mixed *parse_nil(string *input)
{
	return ({ ({ T_NIL }) });
}

static mixed *parse_int(string *input)
{
	return ({ ({ T_INT, (int)input[0] }) });
}

static mixed *parse_flt(string *input)
{
	return ({ ({ T_FLOAT, (float)input[0] }) });
}

static mixed *parse_str(string *input)
{
	return ({
		({ T_STRING, STRINGD->
			quote_unescape(
				input[0][1 .. strlen(input[0]) - 2]
			)
		})
	});
}

static mixed *parse_obj(string *input)
{
	string oname, osubname;
	string *parts;
	object root, obj;
	
	oname = input[0];
	oname = oname[1 .. strlen(oname) - 2];	/* strip off angle brackets */

	if (function_object("parse_object", previous_object(1))) {
		object ret;
		
		ret = previous_object(1)->parse_object(oname);
		
		if (!ret) {
			CHANNELD->post_message("warning", oname,
				"Previous object returned nil");
		}

		return ({ ({ T_OBJECT, ret }) });
	}
	
	parts = explode(oname, ":") - ({ "" });
	
	if (sizeof(parts) == 0) {
		error("<" + oname + "> Badly formatted object designation");
	}
	
	if (parts[0][0] == '/') {
		string osubname;
		
		root = find_object(parts[0]);
		
		if (!root) {
			error(parts[0] + ": no such object");
		}
	} else {
		error("Unspecified base for object");
	}
	
	osubname = implode(parts[1 ..], ":");
	obj = root->locate_object(osubname);

	if (!obj) {
		CHANNELD->post_message("warning", nil,
			"Cannot find " + osubname + " in " +
			object_name(root));
	}
	
	return ({ ({ T_OBJECT, obj }) });
}

static mixed *make_arr(mixed *input)
{
	mixed *elements;
	mixed *nocomma;
	int i;

	elements = input[1 .. sizeof(input) - 2];
	nocomma = allocate(sizeof(elements) / 2 + 1);
	
	for (i = 0; i < sizeof(nocomma); i++) {
		nocomma[i] = elements[i * 2];
	}
	
	return ({ ({ T_ARRAY, nocomma }) });
}

static mixed *make_empty_arr(mixed *input)
{
	return ({ ({ T_ARRAY, ({ }) }) });
}

static mixed *make_map(mixed *input)
{
	mixed *elements;
	mixed *indices;
	mixed *values;
	int i;

	elements = input[1 .. sizeof(input) - 2];
	indices = allocate(sizeof(elements) / 4 + 1);
	values = allocate(sizeof(elements) / 4 + 1);
	
	for (i = 0; i < sizeof(indices); i++) {
		indices[i] = elements[i * 4];
		values[i] = elements[i * 4 + 2];
	}
	
	return ({ ({ T_MAPPING, indices, values }) });
}

static mixed *make_empty_map(mixed *input)
{
	return ({ ({ T_MAPPING, ({ }), ({ }) }) });
}
static mixed *make_lwo(mixed *input)
{
	string path;
	mixed *value;
	int i;

	path = input[1];
	value = input[3];

	return ({ ({ T_MAPPING + 1, path, value }) });
}

static mixed *parse_dup(mixed *input)
{
	return ({ ({ 8, (int)input[0][1 ..] }) });
}
