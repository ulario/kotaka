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

	if (!ret)
		error("Parse failure");

	return ret[0];
}
/*
whitespace = / +/
prep = /at|in|on|from|by|with|of|for|to/
art = /a|an|the/
conj = /and|or/
word = /[^ ]+/

iclause: vp
iclause: vp ppl

vp: verb ? vp_verb
vp: verb np ? vp_verb_np

pp: prep np ? pp_prep_np

np: oart oadjc noun ? np_oart_oadjc_noun

oart: ? oart
oart: art ? oart_art

oadjc: ? oadjc
oadjc: adjl ? oadjc_adjl

ppl: pp
ppl: ppl pp

adjl: adj
adjl: adjl adj

verb: word
noun: word
noun: prep
adj: word
adj: conj
*/
