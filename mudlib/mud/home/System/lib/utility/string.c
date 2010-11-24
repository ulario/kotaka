#include <kotaka/paths.h>

inherit SECOND_AUTO;

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
