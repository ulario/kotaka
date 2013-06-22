/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
/** Property typing and inheritance */
#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/property.h>

#include <type.h>

private mapping pinfo;
/* ({ type, code, extra }) */

/*
For combo/mixdown properties, extra is ({ local, remove })
For magic properties, extra is ({ reader, writer })
For other inheritance codes, extra has no meaning
*/

static void create()
{
	pinfo = ([ ]);
}

void add_property(string name, int type, int code, varargs mixed extra)
{
	ACCESS_CHECK(PRIVILEGED());

	if (sscanf(name, "base:")) {
		error("Reserved property");
	}

	if (pinfo[name]) {
		error(name + ": duplicate property");
	}

	if ((code == PROP_COMBO || code == PROP_MIXDOWN)
		&& type < T_ARRAY) {
		error("Combo properties must be of conglomerate type");
	}

	pinfo[name] = ({ type, code, extra });
}

void del_property(string name)
{
	ACCESS_CHECK(PRIVILEGED());

	if (pinfo[name]) {
		pinfo[name] = nil;
	} else {
		error(name + ": no such property");
	}
}

void clear_properties()
{
	ACCESS_CHECK(PRIVILEGED());

	pinfo = ([ ]);
}

mixed *query_property(string name)
{
	if (!pinfo[name]) {
		return nil;
	}

	return pinfo[name][..];
}

string *list_properties()
{
	return map_indices(pinfo);
}

atomic void load_property_type_file(string text)
{
	int lineno;
	object linebuf;
	string line;

	ACCESS_CHECK(PRIVILEGED());

	linebuf = new_object("~/lwo/linebuf");

	linebuf->load(text);

	while (!linebuf->empty()) {
		string *words;

		string pname;

		string lname;
		string rname;

		mixed extra;

		int typecode;
		int propcode;

		line = linebuf->getline();

		lineno++;

		line = STRINGD->replace(line, "\t", " ");

		sscanf(line, "%s#", line);

		words = explode(line, " ") - ({ "" });

		if (sizeof(words) == 0)
			continue;

		if (sizeof(words) < 3)
			error("Line " + (lineno + 1) +
				": Bad read of " + implode(words,
					" ") + "(bad word count)");

		pname = words[0];

		switch (words[1]) {
		case "any":
			typecode = -1;
			break;
		case "int":
		case "integer":
			typecode = T_INT;
			break;
		case "flt":
		case "float":
			typecode = T_FLOAT;
			break;
		case "str":
		case "string":
			typecode = T_STRING;
			break;
		case "obj":
		case "object":
			typecode = T_OBJECT;
			break;
		case "arr":
		case "array":
			typecode = T_ARRAY;
			break;
		case "map":
		case "mapping":
			typecode = T_MAPPING;
			break;
		default:
			error("Line " + (lineno + 1) +
				": Bad read of " + implode(words,
					" ") + " (invalid type code)");
		}

		switch (words[2]) {
		case "simple":
			if (sizeof(words) != 3)
				error("Line " + (lineno + 1) +
					": Bad read of " +
					implode(words,
						" ") +
					" (bad word count)");
			propcode = PROP_SIMPLE;
			break;

		case "inherit":
			if (sizeof(words) != 3)
				error("Line " + (lineno + 1) +
					": Bad read of " +
					implode(words,
						" ") +
					" (bad word count)");
			propcode = PROP_INHERIT;
			break;

		case "dropdown":
			if (sizeof(words) != 3)
				error("Line " + (lineno + 1) +
					": Bad read of " +
					implode(words,
						" ") +
					" (bad word count)");
			propcode = PROP_DROPDOWN;
			break;

		case "combo":
			if (sizeof(words) != 5)
				error("Line " + (lineno + 1) +
					": Bad read of " +
					implode(words,
						" ") +
					" (bad word count)");
			lname = words[3];
			rname = words[4];

			extra = ({ lname, rname });
			propcode = PROP_COMBO;
			break;

		case "mixdown":
			if (sizeof(words) != 5)
				error("Line " + (lineno + 1) +
					": Bad read of " +
					implode(words,
						" ") +
					" (bad word count)");
			lname = words[3];
			rname = words[4];

			extra = ({ lname, rname });
			propcode = PROP_MIXDOWN;
			break;

		case "magic":
			if (sizeof(words) != 5)
				error("Line " + (lineno + 1) +
					": Bad read of " +
					implode(words,
						" ") +
					" (bad word count)");
			lname = words[3];
			rname = words[4];

			extra = ({ lname, rname });
			propcode = PROP_MAGIC;
			break;

		default:
			error("Line " + (lineno + 1) +
				": Bad read of " + implode(words,
					" ") +
				" (invalid inheritance code)");
		}

		add_property(pname, typecode, propcode, extra);
	}
}
