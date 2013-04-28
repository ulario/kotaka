/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012-2013  Raymond Jennings
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
#include <kotaka/assert.h>
#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <catalog/paths.h>

#include <type.h>
#include <limits.h>

string *chars;

string hex(int i);
string bin(int i);

static void create()
{
	chars = allocate(256);
}

static void upgraded()
{
	chars = allocate(256);
}

void reboot()
{
	chars = allocate(256);
}

string chars(int code, int count)
{
	string log;

	if (code < 0 || code > 0xFF) {
		error("Invalid character code");
	}

	if (count > MAX_STRING_SIZE) {
		error("String too long (" + count + ")");
	}

	if (!chars[code]) {
		string single;
		single = " ";
		single[0] = code;
		chars[code] = single;
	}

	log = chars[code];

	while (strlen(log) < count) {
		if (strlen(log) * 2 > count) {
			log += log[0 .. count - strlen(log) - 1];
		} else {
			log += log;
		}
	}

	chars[code] = log;
	return log[0 .. count - 1];
}

string spaces(int count)
{
	return chars(' ', count);
}

string nulls(int count)
{
	return chars('\0', count);
}

string replace(string input, string from, string to);

/**************/
/* Characters */
/**************/

int char_is_whitespace(int char)
{
	if ((char == '\n') || (char == '\t') || (char == ' ')) {
		return 1;
	}

	return 0;
}

int char_to_lower(int char)
{
	if ((char <= 'Z') && (char >= 'A')) {
		char |= 0x20;
	}

	return char;
}

int char_to_upper(int char)
{
	if ((char <= 'z') && (char >= 'a'))
		char &= ~0x20;

	return char;
}

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
	return regex_match(str, "[-+A-Za-z0-9_]+");
}

int is_valid_id(string str)
{
	return regex_match(str, "[-+A-Za-z0-9_]+(#[1-9][0-9]*)?");
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
	return regex_match(str, "[_a-zA-Z0-9]+");
}

int is_valid_username(string str)
{
	return str && strlen(str) >= 1 && strlen(str) <= 32 &&
		regex_match(str, "[A-Za-z][_A-Za-z0-9]*");
}

string trim_whitespace(string str)
{
	int start, end;

	if (!str || str == "")
		return str;

	start = 0;
	end = strlen(str) - 1;

	while ((start <= end) && char_is_whitespace(str[start])) {
		start++;
	}

	while ((start <= end) && char_is_whitespace(str[end])) {
		end--;
	}

	return str[start .. end];
}

string rtrim_null(string str)
{
	int end;

	if (!str || str == "")
		return str;

	end = strlen(str) - 1;

	while (end >= 0 && str[end] == 0)
		end--;

	return str[.. end];
}

string rtrim_whitespace(string str)
{
	int end;

	if (!str || str == "")
		return str;

	end = strlen(str) - 1;

	while ((end >= 0) && char_is_whitespace(str[end])) {
		end--;
	}

	return str[.. end];
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

string yn_check(string str)
{
	if (str == nil || str == "") {
		return "E";
	}

	switch(to_upper(str)[0]) {
	case 'Y':
		return "Y";
	case 'N':
		return "N";
	default:
		return "E";
	}
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

/* If s1 is later in alphabetical order, return 1.  If s2 is later,
   return -1.  If neither, return 0. */
int stricmp(string s1, string s2)
{
	int tmp1, tmp2, len1, len2;
	int len, iter;

	len1 = strlen(s1);
	len2 = strlen(s2);
	len = len1 > len2 ? len2 : len1;

	for (iter = 0; iter < len; iter++) {
		tmp1 = s1[iter];
		tmp2 = s2[iter];

		if (tmp1 <= 'Z' && tmp1 >= 'A') {
			tmp1 += 'a' - 'A';
		}

		if (tmp2 <= 'Z' && tmp2 >= 'A') {
			tmp2 += 'a' - 'A';
		}

		if (tmp1 > tmp2) {
			return 1;
		}
		if (tmp2 > tmp1) {
			return -1;
		}
	}

	if (len1 == len2) {
		return 0;
	}
	if (len1 > len2) {
		return 1;
	}
	return -1;
}


/* If s1 is later in alphabetical order, return 1.  If s2 is later,
   return -1.  If neither, return 0. */
int strcmp(string s1, string s2)
{
	int len1, len2;
	int len, iter;

	len1 = strlen(s1);
	len2 = strlen(s2);
	len = len1 > len2 ? len2 : len1;

	for (iter = 0; iter < len; iter++) {
		if (s1[iter] > s2[iter]) {
			return 1;
		}
		if (s2[iter] > s1[iter]) {
			return -1;
		}
	}

	if (len1 == len2) {
		return 0;
	}
	if (len1 > len2) {
		return 1;
	}
	return -1;
}

string replace(string input, string from, string to)
{
	string *arr;

	arr = explode(from + input + from, from);

	return implode(arr, to);
}

string quote_escape(string input)
{
	input = replace(input, "\\", "\\\\");
	input = replace(input, "\"", "\\\"");
	input = replace(input, "\n", "\\n");
	input = replace(input, "\r", "\\r");
	input = replace(input, "\t", "\\t");

	return input;
}

string quote_unescape(string input)
{
	input = replace(input, "\\t", "\t");
	input = replace(input, "\\r", "\r");
	input = replace(input, "\\n", "\n");
	input = replace(input, "\\\"", "\"");
	input = replace(input, "\\\\", "\\");

	return input;
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

string mixed_sprint(mixed data, varargs mapping seen)
{
	int iter;
	string tmp;
	mixed *arr;

	if (!seen)
		seen = ([ ]);

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
			return "#" + seen[data];
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
		{
			string name;

			if (data <- LIB_CATALOG && (name = data->query_object_name())) {
				return "<" + name + ">";
			} else {
				return "<" + object_name(data) + ">";
			}
		}
		break;
	default:
		error("Unrecognized DGD type in mixed_sprint");
	}
}

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
		return mixed_sprint(data);
	case T_ARRAY:
		if (seen[data] != nil) {
			return "#" + seen[data];
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
					":\n" + ind + "    " +
					tree_sprint(data[indices[index]],
					indent + 4, seen);
			}

			return "([\n" + ind + implode(parts,
				",\n" + ind) + "\n" + ind + "])";
		}
	}
}

int prefix_string(string prefix, string str)
{
	if (strlen(str) < strlen(prefix)) {
		return 0;
	}

	if (str[0 .. strlen(prefix) - 1] == prefix) {
		return 1;
	}

	return 0;
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
		return mixed_sprint(data);
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

string normalize_whitespace(string input, int maxwidth)
{
	string *words;
	string *lines;

	int index;
	string line;

	int linewidth;

	input = replace(input, "\t", " ");
	input = replace(input, "\n", " ");

	words = explode(input, " ") - ({ "" });

	line = "";
	lines = ({ });

	for(index = 0; index < sizeof(words); index++) {
		string word;
		int wordwidth;

		word = words[index];
		wordwidth = strlen(word) + 1;

		switch(word[strlen(word) - 1]) {
		case '.':
		case '!':
		case ';':
		case '?':
			wordwidth++;
			word += "  ";
			break;
		default:
			word += " ";
		}

		if (wordwidth + linewidth > maxwidth) {
			lines += ({ trim_whitespace(line) });
			linewidth = 0;
			line = "";
		}

		line += word;
		linewidth += wordwidth;
	}

	if (line != "") {
		lines += ({ trim_whitespace(line) });
	}

	return implode(lines, "\n") + "\n";
}

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

string stime(int time)
{
	int ancient;
	string timestr;

	ancient = time() - 6 * 30 * 24 * 60 * 60;

	timestr = ctime(time);

	if (time >= ancient) {
		timestr = timestr[4 .. 15];
	} else {
		timestr = timestr[4 .. 10] + timestr[19 .. 23];
	}
	return timestr;
}

int ip_to_number(string ip)
{
	int a, b, c, d;

	sscanf(ip, "%d.%d.%d.%d", a, b, c, d);

	return a << 24 | b << 16 | c << 8 | d;
}

string ip_to_string(int ip)
{
	int a, b, c, d;

	a = (ip >> 24) & 0xFF;
	b = (ip >> 16) & 0xFF;
	c = (ip >> 8) & 0xFF;
	d = (ip) & 0xFF;

	return a + "." + b + "." + c + "." + d;
}

string string_truncate(string line, int bits)
{
	int bytes;
	int mask;

	bytes = (bits + 7) >> 3;
	mask = 0xFF >> ((8 - bits) & 0x7);

	if (strlen(line) > bytes) {
		line = line[0 .. bytes - 1];
	} else while (strlen(line) < bytes) {
		line += "\000";
	}

	if (strlen(line) == bytes) {
		line[bytes - 1] &= mask;
	}

	return rtrim_null(line);
}

string string_rshift(string a, int bits)
{
	int buf;
	int index;

	a = a[bits / 8 ..];
	bits %= 8;

	buf = a[0];

	for (index = 0; index < strlen(a); index++) {
		buf |= ((index + 1) < strlen(a) ? a[index + 1] : 0) << 8;
		a[index] = (buf >> bits) & 255;
		buf >>= 8;
	}

	return rtrim_null(a);
}

string string_lshift(string a, int bits)
{
	for (; bits >= 8; bits -= 8) {
		a = "\000" + a;
	}
	return rtrim_null(string_rshift("\000" + a, 8 - bits));
}

string string_and(string a, string b)
{
	int i;

	if (strlen(a) < strlen(b)) {
		string c;
		c = a;
		a = b;
		b = c;
	}

	for (i = 0; i <= strlen(b); i++) {
		b[i] &= a[i];
	}

	return b;
}

string string_or(string a, string b)
{
	int i;

	if (strlen(a) < strlen(b)) {
		string c;
		c = a;
		a = b;
		b = c;
	}

	for (i = 0; i <= strlen(b); i++) {
		a[i] |= b[i];
	}

	return a;
}

string string_xor(string a, string b)
{
	int i;

	if (strlen(a) < strlen(b)) {
		string c;
		c = a;
		a = b;
		b = c;
	}

	for (i = 0; i <= strlen(b); i++) {
		a[i] ^= b[i];
	}

	return a;
}

string string_nand(string a, string b)
{
	int lim;
	int i;

	lim = strlen(a);

	if (lim > strlen(b)) {
		lim = strlen(b);
	}

	for (i = 0; i < lim; i++) {
		a[i] &= ~b[i];
	}

	return a;
}

string string_patch(string canvas, string brush, int coff, int blen)
{
	int smask;
	int emask;

	int sbyte;
	int ebyte;

	int sbit;
	int ebit;

	int byte;

	brush = string_lshift(brush, coff);

	sbit = coff;
	ebit = coff + blen - 1;

	sbyte = sbit >> 3;
	ebyte = ebit >> 3;

	sbit &= 7;
	ebit &= 7;

	smask = (0xFF << sbit) & 0xFF;
	emask = 0xFF >> (7 - ebit);

	while(strlen(brush) <= ebyte) {
		brush += "\000";
	}

	while(strlen(canvas) <= ebyte) {
		canvas += "\000";
	}

	if (sbyte == ebyte) {
		smask &= emask;
	} else {
		for (byte = sbyte + 1; byte < ebyte - 1; byte++) {
			canvas[byte] = brush[byte];
		}

		canvas[ebyte] =
			(canvas[ebyte] & ~emask) | (brush[ebyte] & emask);
	}

	canvas[sbyte] =
		(canvas[sbyte] & ~smask)
		| (brush[sbyte] & smask);

	return canvas;
}

string string_from_seconds(int seconds, varargs int details)
{
	int count;
	string unit;
	string out;
	int ago;

	if (seconds == 0) {
		return "now";
	}

	if (seconds < 0) {
		ago = 1;
		seconds = -seconds;
	}

	details++;

	while (details-- && seconds) {
		if (seconds >= 86400) {
			count = seconds / 86400;
			unit = "day";
			seconds %= 86400;
		} else if (seconds >= 3600) {
			count = seconds / 3600;
			unit = "hour";
			seconds %= 3600;
		} else if (seconds >= 60) {
			count = seconds / 60;
			unit = "minute";
			seconds %= 60;
		} else if (seconds >= 1) {
			count = seconds;
			unit = "second";
			seconds = 0;
		}

		if (count > 1) {
			unit += "s";
		}

		if (out) {
			out += ", " + count + " " + unit;
		} else {
			out = count + " " + unit;
		}
	}

	if (ago) {
		out += " ago";
	}

	return out;
}

string pack_int(int value, varargs int be)
{
	string out;

	out = "    ";

	if (be) {
		out[0] = value & 0xFF;
		out[1] = (value >> 8) & 0xFF;
		out[2] = (value >> 16) & 0xFF;
		out[3] = (value >> 24) & 0xFF;
	} else {
		out[3] = value & 0xFF;
		out[2] = (value >> 8) & 0xFF;
		out[1] = (value >> 16) & 0xFF;
		out[0] = (value >> 24) & 0xFF;
	}
	return out;
}

int unpack_int(string str, varargs int be)
{
	int value;

	if (be) {
		value |= str[0];
		value |= str[1] << 8;
		value |= str[2] << 16;
		value |= str[3] << 24;
	} else {
		value |= str[3];
		value |= str[2] << 8;
		value |= str[1] << 16;
		value |= str[0] << 24;
	}

	return value;
}

string vpack_int(int value, varargs int be)
{
	string out;
	string char;

	char = " ";
	out = "";

	do {
		char[0] = (value & 0x7F) | 0x80;

		if (be) {
			out = char + out;
		} else {
			out = out + char;
		}

		value >>= 7;
	} while (value);

	out[strlen(out) - 1] &= 0x7F;

	return out;
}

int vunpack_int(string str, varargs int be)
{
	string char;
	int value;
	int index;

	char = " ";

	if (be) {
		for (index = 0; index < strlen(str); index++) {
			value <<= 7;
			char[0] = str[index];
			value = char[0] & 0x7F;
		}
	} else {
		for (index = strlen(str) - 1; index >= 0; index--) {
			value <<= 7;
			char[0] = str[index];
			value = char[0] & 0x7F;
		}
	}

	return value;
}

string **align_table(string **table, varargs int ralign)
{
	int irow;
	int icol;
	int cols;

	int tsz;

	string **out;

	int *widths;
	widths = ({ });
	tsz = sizeof(table);
	out = allocate(tsz);

	for (irow = 0; irow < tsz; irow++) {
		string *row;
		int rsz;

		row = table[irow];

		if (!row) {
			continue;
		}

		rsz = sizeof(row);

		if (cols < rsz) {
			cols = rsz;
		}

		while (sizeof(widths) < cols) {
			widths += ({ 0 });
		}

		for (icol = 0; icol < rsz; icol++) {
			int sz;

			sz = strlen(row[icol]);

			if (widths[icol] < sz) {
				widths[icol] = sz;
			}
		}
	}

	for (irow = 0; irow < tsz; irow++) {
		string *row;
		int rsz;

		row = table[irow];

		if (!row) {
			row = ({ });
		}

		rsz = sizeof(row);

		out[irow] = allocate(cols);

		for (icol = 0; icol < rsz; icol++) {
			string cell;
			string piece;
			string spaces;

			piece = row[icol];
			spaces = spaces(widths[icol] - strlen(piece));

			if (ralign) {
				cell = spaces + piece;
			} else {
				cell = piece + spaces;
			}

			out[irow][icol] = cell;
		}
	}

	return out;
}

string bin(int val)
{
	int index;
	string out;
	out = "";

	if (!val) {
		return "0";
	}

	while (val) {
		int bit;

		bit = val & 0x1;
		val >>= 1;

		out = " " + out;

		out[0] = '0' + bit;
	}

	return out;
}
string hex(int val)
{
	int index;
	string out;
	out = "";

	if (!val) {
		return "0";
	}

	while (val) {
		int nibble;

		nibble = val & 0xF;
		val >>= 4;

		out = " " + out;

		if (nibble > 9) {
			out[0] = 'a' + (nibble - 10);
		} else {
			out[0] = '0' + nibble;
		}
	}

	return out;
}

/*
000000000011111111112222
012345678901234567890123
WWW MMM DD HH:MM:SS YYYY

MMM DD HH:MM
MMM DD  YYYY
*/

/* ansi-coded ascii graphic compiler */

private int target_bits(int chr)
{
	switch (chr) {
	case 'k': return 0;
	case 'r': return 1;
	case 'g': return 2;
	case 'y': return 3;
	case 'b': return 4;
	case 'm': return 5;
	case 'c': return 6;
	case 'w': return 7;
	case 'K': return 8;
	case 'R': return 9;
	case 'G': return 10;
	case 'Y': return 11;
	case 'B': return 12;
	case 'M': return 13;
	case 'C': return 14;
	case 'W': return 15;
	default: return -1;
	}
}

/* Simple compiler for ansi-escaped ascii graphics */
/* Colors: */
/* K = black, R = red, G = green, Y = yellow */
/* B = blue, M = magenta, C = cyan, W = white */
/* case determines intensity and blink */
string simple_ansify(string chars, varargs string fore, string back)
{
	string buf;
	int i;
	int sz;
	int forecolor;
	int backcolor;
	buf = "";

	/* 1 = blue bit */
	/* 2 = green bit */
	/* 4 = red bit */
	/* 8 = intense/blink bit */
	forecolor = 7;
	backcolor = 0;

	sz = strlen(chars);

	for (i = 0; i < sz; i++) {
		int target_bits;
		int forecolor_change;
		int backcolor_change;
		int bold_change;
		int blink_change;
		string *codes;
		string escape;

		codes = ({ });

		target_bits = -1;

		if (fore) {
			target_bits = target_bits(fore[i]);
		}

		if (target_bits == -1) {
			target_bits = 7;
		}

		if ((target_bits & 7) != (forecolor & 7)) {
			forecolor_change = 1;
		}
		if ((target_bits & 8) != (forecolor & 8)) {
			bold_change = 1;
		}

		forecolor = target_bits;

		target_bits = -1;

		if (back) {
			target_bits = target_bits(back[i]);
		}

		if (target_bits == -1) {
			target_bits = 0;
		}

		if ((target_bits & 7) != (backcolor & 7)) {
			backcolor_change = 1;
		}
		if ((target_bits & 8) != (backcolor & 8)) {
			blink_change = 1;
		}

		backcolor = target_bits;

		if (forecolor == 7 && backcolor == 0 &&
			(bold_change
				|| blink_change
				|| forecolor_change
				|| backcolor_change
			)
		) {
			codes = ({ "0" });
			bold_change = 0;
			blink_change = 0;
			forecolor_change = 0;
			backcolor_change = 0;
		}

		if (bold_change) {
			if (forecolor & 8) {
				codes += ({ "1" });
			} else {
				codes += ({ "22" });
			}

			bold_change = 0;
		}

		if (forecolor_change) {
			codes += ({ "3" + (forecolor & 7) });

			forecolor_change = 0;
		}

		if (blink_change) {
			if (backcolor & 8) {
				codes += ({ "5" });
			} else {
				codes += ({ "25" });
			}

			blink_change = 0;
		}

		if (backcolor_change) {
			codes += ({ "4" + (backcolor & 7) });

			backcolor_change = 0;
		}

		if (sizeof(codes)) {
			escape = "\033[" + implode(codes, ";") + "m";
			buf += escape;
		}

		buf += chars[i .. i];
	}

	return buf;
}

string wordwrap(string text, int width)
{
	string *words;
	string buffer;
	int tail;
	int length;
	int i;
	int sz;

	buffer = "";
	words = explode(text, " ") - ({ nil, "" });
	sz = sizeof(words);

	for (i = 0; i < sz; i++) {
		string word;
		int wlen;

		word = words[i];
		wlen = strlen(word);

		if (length + tail + wlen > width) {
			length = 0;
			buffer += "\n";
		} else {
			buffer += spaces(tail);
			length += tail;
		}

		buffer += word;
		length += wlen;

		switch(word[wlen - 1]) {
		case '.':
		case '?':
		case '!':
			tail = 2;
			break;
		default:
			tail = 1;
		}
	}

	return buffer;
}

static void destruct()
{
	/* we're too important to just be deleted willy nilly */
	error("Permission denied");
}
