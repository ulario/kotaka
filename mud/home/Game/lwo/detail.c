string *snouns;
string *pnouns;
string *adjectives;

mapping descriptions;

string combine;

/* craetor */

static void create(int clone)
{
	if (clone) {
		snouns = ({ });
		pnouns = ({ });
		adjectives = ({ });
		descriptions = ([ ]);
	}
}

/** public **/

/*

combine styles:

replace:
	detail from archetype is ignored
overlap:
	add nouns and adjectives
	local override, default to archetype
nil:
	(defaults to overlap)

*/

void set_combine(string new_combine)
{
	combine = new_combine;
}

string query_combine()
{
	return combine;
}

/* nouns */

void add_snoun(string snoun)
{
	snouns |= ({ snoun });
}

void remove_snoun(string snoun)
{
	snouns -= ({ snoun });
}

string *query_snouns()
{
	return snouns;
}

void add_pnoun(string pnoun)
{
	pnouns |= ({ pnoun });
}

void remove_pnoun(string pnoun)
{
	pnouns -= ({ pnoun });
}

string *query_pnouns()
{
	return pnouns;
}

/* adjectives */

void add_adjective(string adjective)
{
	adjectives |= ({ adjective });
}

void remove_adjective(string adjective)
{
	adjectives -= ({ adjective });
}

string *query_adjectives()
{
	return adjectives;
}

/* descriptions */

string *query_descriptions()
{
	return map_indices(descriptions);
}

void set_description(string description, string text)
{
	descriptions[description] = text;
}

string query_description(string description)
{
	return descriptions[description];
}

/* saveload */

mapping save()
{
	return ([
		"combine": combine,
		"snouns": sizeof(snouns) ? snouns : nil,
		"pnouns": sizeof(pnouns) ? snouns : nil,
		"adjectives": sizeof(snouns) ? snouns : nil,
		"descriptions": map_sizeof(descriptions) ? descriptions : nil
	]);
}

void load(mapping data)
{
	mixed v;

	combine = data["combine"];
	snouns = (v = data["snouns"]) ? v : ({ });
	pnouns = (v = data["pnouns"]) ? v : ({ });
	adjectives = (v = data["adjectives"]) ? v : ({ });
	descriptions = (v = data["descriptions"]) ? v : ([ ]);
}
