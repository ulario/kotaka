#include <kotaka/paths.h>

string grammar;

static void create()
{
	::create();
	
	grammar = read_file("~/data/parse/english.dpd");
}

static void upgraded()
{
	::create();

	grammar = read_file("~/data/parse/english.dpd");
}

mixed parse(string input)
{
	mixed *ret;

	ret = parse_string(grammar, input);

	if (!ret) {
		error("Parse failure");
	}

	return ret;
}

mixed *vp_verb(mixed *input)
{
	return ({ "verb", ({ input[0] }) });
}

mixed *vp_verb_np(mixed *input)
{
	return ({ "verb", ({ input[0] }), "noun", ({ input[1 ..] }) });
}

mixed *pp_prep_np(mixed *input)
{
	return ({ "prep", ({ input[0] }), "noun", ({ input[1 ..] }) });
}

mixed *np_oart_oadjc_noun(mixed *input)
{
	return ({ input[0], input[1], input[2] });
}

mixed *oart(mixed *input)
{
	return ({ nil });
}

mixed *oart_art(mixed *input)
{
	return ({ input[0] });
}

mixed *oadjc(mixed *input)
{
	return ({ ({ }) });
}

mixed *oadjc_adjl(mixed *input)
{
	return ({ input });
}
