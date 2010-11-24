#include <kotaka/paths.h>
#include <kotaka/assert.h>

string grammar;

static void create()
{
	::create();
	
	grammar = read_file("~/open/data/parse/xml.dpd");
}

static void upgraded()
{
	::create();

	grammar = read_file("~/open/data/parse/xml.dpd");
}

mixed parse(string input)
{
	return parse_string(grammar, input);
}

/*
xml: ? make_xml
xml: xml section ? build_xml
*/
mixed *make_xml(mixed *input)
{
	ASSERT(sizeof(input) == 0);
	return ({ ({ }) });
}

mixed *build_xml(mixed *input)
{
	return ({ input[0] + input[1] });
}

/*
section: open_tag xml close_tag ? make_section
section: text ? make_anon_section
*/
/* section: ({ name, content }) */
/* open tag: ({ name }) */
mixed *make_section(mixed *input)
{
	if (input[0] != input[2]) {
		error("Open/close tag mismatch");
	}

	return ({ ({ input[0], input[1] }) }) ;
}
mixed *make_anon_section(mixed *input)
{
	return ({ ({ nil, input[0] }) });
}

/*
open_tag: '<' alnum_word '>' ? make_simple_open_tag
close_tag: '</' alnum_word '>' ? make_close_tag
*/
mixed *make_simple_open_tag(mixed *input)
{
	return ({ input[1] });
}
mixed *make_close_tag(mixed *input)
{
	return ({ input[1] });
}

/*
text: ? make_text
text: text escape ? build_text_unescape
text: text alnum_word ? build_text
text: text quoted_string ? build_text
text: text space ? build_text
*/
mixed *make_text(mixed *input)
{
	return ({ "" });
}
mixed *build_text_unescape(mixed *input)
{
	return ({ input[0] + "<escape: " + input[1] + ">" });
}
mixed *build_text(mixed *input)
{
	return ({ input[0] + input[1] });
}
