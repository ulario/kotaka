string *snouns;
string *pnouns;
string *adjectives;

mapping descriptions;

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

/* nouns */

void add_snoun(string snoun)
{
	snouns |= ({ snoun });
}

void remove_snoun(string snoun)
{
	snouns -= ({ snoun });
}

void set_snouns(string *new_snouns)
{
	snouns = ({ }) | new_snouns - ({ nil });
}

string *query_snouns()
{
	return snouns[..];
}

void add_pnoun(string pnoun)
{
	pnouns |= ({ pnoun });
}

void remove_pnoun(string pnoun)
{
	pnouns -= ({ pnoun });
}

void set_pnouns(string *new_pnouns)
{
	pnouns = ({ }) | new_pnouns - ({ nil });
}

string *query_pnouns()
{
	return pnouns[..];
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
	return adjectives[..];
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
		"snouns": sizeof(snouns) ? snouns[..] : nil,
		"pnouns": sizeof(pnouns) ? pnouns[..] : nil,
		"adjectives": sizeof(snouns) ? adjectives[..] : nil,
		"descriptions": map_sizeof(descriptions) ? descriptions : nil
	]);
}

void load(mapping data)
{
	mixed v;

	snouns = (v = data["snouns"]) ? ({ }) | v : ({ });
	pnouns = (v = data["pnouns"]) ? ({ }) | v : ({ });
	adjectives = (v = data["adjectives"]) ? ({ }) | v : ({ });
	descriptions = (v = data["descriptions"]) ? ([ ]) + v : ([ ]);
}
