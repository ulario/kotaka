/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <kotaka/paths/text.h>
#include <kotaka/paths/system.h>
#include <status.h>
#include <type.h>

inherit "~System/lib/string/escape";
inherit "/lib/string/char";
inherit "/lib/string/bitops";
inherit "/lib/string/trim";
inherit "/lib/string/format";
inherit "/lib/string/base";
inherit "/lib/string/int";

/***********/
/* strings */
/***********/

int is_whitespace(string str)
{
	int len;
	int iter;

	len = strlen(str);

	for (iter = 0; iter < len; iter++) {
		if (!char_is_whitespace(str[iter]))
			return 0;
	}

	return 1;
}

int is_capitalized(string test)
{
	int char;

	char = test[0];

	return (char >= 'A' && char <= 'Z');
}

int string_has_char(int char, string str)
{
	int len, iter;

	len = strlen(str);
	for (iter = 0; iter < len; iter++) {
		if (str[iter] == char)
			return 1;
	}
	return 0;
}

int regex_match(string in, string regex)
{
	regex = replace(regex, "/", "\\/");

	return !!parse_string("regstring = /" + regex + "/\n"
		+ "notreg = nomatch\n" + "full : regstring", in);
}

int is_valid_base_id(string str)
{
	return regex_match(str, "[A-Za-z][-+A-Za-z0-9_]*");
}

int is_valid_id(string str)
{
	return regex_match(str, "[A-Za-z][-+A-Za-z0-9_]*(#[1-9][0-9]*)?");
}

int is_alpha(string str)
{
	return regex_match(str, "[a-zA-Z]+");
}

int is_alphanum(string str)
{
	return regex_match(str, "[a-zA-Z0-9]+");
}

int is_valid_ident(string str)
{
	return regex_match(str, "[A-Za-z][_a-zA-Z0-9]*");
}

int is_valid_username(string str)
{
	return str && strlen(str) >= 1 && strlen(str) <= 32 &&
		regex_match(str, "[A-Za-z][_A-Za-z0-9]*");
}

string to_lower(string text)
{
	int ctr;
	int len;
	string newword;

	newword = text;
	len = strlen(newword);

	for (ctr = 0; ctr < len; ctr++) {
		newword[ctr] = char_to_lower(newword[ctr]);
	}

	return newword;
}

string to_upper(string text)
{
	int ctr;
	int len;
	string newword;

	newword = text;
	len = strlen(newword);

	for (ctr = 0; ctr < len; ctr++) {
		newword[ctr] = char_to_upper(newword[ctr]);
	}

	return newword;
}

string to_title(string text)
{
	string *words;
	int index;

	words = explode(text, " ") - ({ "" });

	for (index = 0; index < sizeof(words); index++) {
		string word;

		word = to_lower(words[index]);

		if (index == 0 || index == (sizeof(words) - 1)) {
			word[0] = char_to_upper(word[0]);
		} else {
			switch(word) {
			case "and":
			case "auf":
			case "de":
			case "of":
			case "or":
			case "the":
				break;
			default:
				word[0] = char_to_upper(word[0]);
			}
		}

		words[index] = word;
	}

	return implode(words, " ");
}

string mixed_sprint(mixed data, varargs mapping seen);

string sprint_object(object obj, varargs mapping seen)
{
	string path;
	string oname;

	if (sscanf(object_name(obj), "%s#-1", path)) {
		if (seen[obj] != nil) {
			return "@" + seen[obj];
		}

		seen[obj] = map_sizeof(seen);

		if (function_object("sprint_save", obj)) {
			return "(< <" + path + ">: " + mixed_sprint(obj->sprint_save(), seen) + ">)";
		} else {
			return "<" + object_name(obj) + ">";
		}
	} else if (function_object("query_object_name", obj) && (oname = obj->query_object_name())) {
		return "<" + oname + ">";
	} else {
		return "<" + object_name(obj) + ">";
	}
}

string mixed_sprint(mixed data, varargs mapping seen)
{
	int iter;
	string tmp;
	mixed *arr;

	if (!seen) {
		seen = ([ ]);
	}

	switch (typeof(data)) {
	case T_NIL:
		return "nil";

	case T_STRING:
		return "\"" + quote_escape(data) + "\"";

	case T_INT:
		return (string)data;

	case T_FLOAT:
		/* decimal point is required */
		{
			string mantissa;
			string exponent;
			string str;

			str = (string)data;

			if (!sscanf(str, "%se%s", mantissa, exponent)) {
				mantissa = str;
				exponent = "";
			} else {
				exponent = "e" + exponent;
			}

			if (!sscanf(mantissa, "%*s.")) {
				mantissa += ".0";
			}

			return mantissa + exponent;
		}

	case T_ARRAY:
		if (seen[data] != nil) {
			return "@" + seen[data];
		}

		seen[data] = map_sizeof(seen);

		if (sizeof(data) == 0)
			return "({ })";

		tmp = "({ ";
		for (iter = 0; iter < sizeof(data); iter++) {
			tmp += mixed_sprint(data[iter], seen);
			if (iter < sizeof(data) - 1) {
				tmp += ", ";
			}
		}
		return tmp + " })";

	case T_MAPPING:
		if (seen[data] != nil) {
			return "@" + seen[data];
		}

		seen[data] = map_sizeof(seen);

		if (map_sizeof(data) == 0)
			return "([ ])";

		arr = map_indices(data);
		tmp = "([ ";
		for (iter = 0; iter < sizeof(arr); iter++) {
			tmp += mixed_sprint(arr[iter], seen) + " : " +
				mixed_sprint(data[arr[iter]], seen);
			if (iter != sizeof(arr) - 1)
				tmp += ", ";
		}
		return tmp + " ])";

	case T_OBJECT:
		if (function_object("sprint_object", previous_object())) {
			return previous_object()->sprint_object(data, seen);
		} else {
			return sprint_object(data, seen);
		}
		break;
	}
}

string object_escape(string input)
{
	input = replace(input, "\\", "\\\\");
	input = replace(input, ">", "\\>");

	return input;
}

string object_unescape(string input)
{
	input = replace(input, "\\>", ">");
	input = replace(input, "\\\\", "\\");

	return input;
}

/**************/
/* LPC values */
/**************/

string tree_sprint(mixed data, varargs int indent, mapping seen)
{
	string ind;

	ind = "";

	if (!seen)
		seen = ([ ]);

	while (strlen(ind) < indent)
		ind += " ";

	switch (typeof(data)) {
	case T_INT:
	case T_NIL:
	case T_STRING:
	case T_FLOAT:
	case T_OBJECT:
		return mixed_sprint(data, seen);

	case T_ARRAY:
		if (seen[data] != nil) {
			return "@" + seen[data];
		}

		seen[data] = map_sizeof(seen);

		if (sizeof(data) == 0)
			return "({ })";

		{
			string *parts;
			int index;

			parts = allocate(sizeof(data));

			for (index = 0; index < sizeof(data); index++) {
				parts[index] =
					"  " + tree_sprint(data[index],
					indent + 2, seen);
			}

			return "({\n" + ind + implode(parts,
				",\n" + ind) + "\n" + ind + "})";
		}

	case T_MAPPING:
		if (seen[data] != nil) {
			return "@" + seen[data];
		}

		seen[data] = map_sizeof(seen);

		if (map_sizeof(data) == 0)
			return "([ ])";
		{
			mixed *indices;
			string *parts;
			int index;

			parts = allocate(map_sizeof(data));
			indices = map_indices(data);

			for (index = 0; index < map_sizeof(data); index++) {
				parts[index] =
					"  " + mixed_sprint(indices[index], seen) +
					" :\n" + ind + "    " +
					tree_sprint(data[indices[index]],
					indent + 4, seen);
			}

			return "([\n" + ind + implode(parts,
				",\n" + ind) + "\n" + ind + "])";
		}
	}
}

int is_simple(mixed data)
{
	int sz;

	switch (typeof(data)) {
	case T_INT:
	case T_NIL:
	case T_STRING:
	case T_FLOAT:
	case T_OBJECT:
		return 1;
	case T_ARRAY:
		sz = sizeof(data);
		return sz == 0;
	case T_MAPPING:
		sz = map_sizeof(data);
		return sz == 0;
	}
}

int is_flat(mixed data)
{
	int sz;
	int i;

	if (is_simple(data)) {
		return 1;
	}

	switch (typeof(data)) {
	case T_ARRAY:
		sz = sizeof(data);

		for (i = 0; i < sz; i++) {
			if (!is_simple(data[i])) {
				return 0;
			}
		}

		return 1;
	case T_MAPPING:
		sz = map_sizeof(data);

		{
			mixed *ind;
			mixed *val;


			ind = map_indices(data);
			val = map_values(data);

			for (i = 0; i < sz; i++) {
				if (!is_simple(ind[i])) {
					return 0;
				}
				if (!is_simple(val[i])) {
					return 0;
				}
			}
		}
		return 0;
	}
}

string hybrid_sprint(mixed data, varargs int indent, mapping seen)
{
	string ind;

	ind = spaces(indent);

	if (!seen)
		seen = ([ ]);

	switch (typeof(data)) {
	case T_INT:
	case T_NIL:
	case T_STRING:
	case T_FLOAT:
	case T_OBJECT:
		return mixed_sprint(data, seen);
	case T_ARRAY:
		if (seen[data] != nil) {
			return "#" + seen[data];
		}

		if (sizeof(data) == 0)
			return "({ })";

		if (is_flat(data)) {
			return mixed_sprint(data, seen);
		}

		seen[data] = map_sizeof(seen);

		{
			string *parts;
			int index;

			parts = allocate(sizeof(data));

			for (index = 0; index < sizeof(data); index++) {
				parts[index] =
					"  " + hybrid_sprint(data[index],
					indent + 2, seen);
			}

			return "({\n" + ind + implode(parts,
				",\n" + ind) + "\n" + ind + "})";
		}
	case T_MAPPING:
		if (seen[data] != nil) {
			return "@" + seen[data];
		}

		if (map_sizeof(data) == 0) {
			return "([ ])";
		}

		if (is_flat(data)) {
			return mixed_sprint(data, seen);
		}

		seen[data] = map_sizeof(seen);

		{
			mixed *indices;
			string *parts;
			int index;

			parts = allocate(map_sizeof(data));
			indices = map_indices(data);

			for (index = 0; index < map_sizeof(data); index++) {
				parts[index] =
					"  " + mixed_sprint(indices[index], seen) +
					" :" + (is_simple(data[indices[index]]) ? " " : "\n" + ind + "    ") +
						hybrid_sprint(data[indices[index]],
					indent + 4, seen);
			}

			return "([\n" + ind + implode(parts,
				",\n" + ind) + "\n" + ind + "])";
		}
	}
}

/************************/
/* formatting functions */
/************************/

string ralign(mixed item, int width)
{
	if (strlen("" + item) > width) {
		return "" + item;
	}

	return spaces(width - strlen("" + item)) + item;
}

string lalign(mixed item, int width)
{
	if (strlen("" + item) > width) {
		return "" + item;
	}

	return item + spaces(width - strlen("" + item));
}
