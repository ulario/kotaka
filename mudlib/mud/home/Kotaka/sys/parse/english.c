#include <kotaka/paths.h>
#include <kotaka/log.h>

string grammar;

void decomment()
{
	string *lines;

	int i;
	int sz;

	lines = explode(grammar, "\n");
	sz = sizeof(lines);

	for (i = 0; i < sz; i++) {
		if (lines[i] == "") {
			lines[i] = nil;
		} else if (lines[i][0] == '#') {
			lines[i] = nil;
		}
	}

	lines -= ({ nil });
	grammar = implode(lines, "\n");
	LOGD->post_message("grammar", LOG_INFO, "Grammar:\n" + grammar);
}

static void create()
{
	::create();

	grammar = read_file("~/data/parse/english.dpd");

	decomment();
}

static void upgraded()
{
	::create();

	grammar = read_file("~/data/parse/english.dpd");

	decomment();
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
	return ({ ({ "V", input[0], nil }) });
}

mixed *vp_verb_np(mixed *input)
{
	return ({ ({ "V", input[0], input[1] }) });
}

mixed *pp_prep_np(mixed *input)
{
	return ({ ({ "P", input[0], input[1] }) });
}

mixed *np_oart_oadjc_noun(mixed *input)
{
	return ({ ({ "N", input[0], input[1], input[2] }) });
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
