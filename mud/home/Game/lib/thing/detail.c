mapping details;

/* creator */

static void create()
{
	details = ([ ]);
}

/* setup */

void detail_patch()
{
	if (!details) {
		details = ([ ]);
	}
}

/* details */

string *query_details()
{
	detail_patch();

	return map_indices(snouns);
}

void add_detail(string detail)
{
	detail_patch();

	details[detail] = new_object("~/lwo/detail");
}

void remove_detail(string detail)
{
	detail_patch();

	details[detail] = nil;
}

/* nouns */

void set_snouns(string detail, string *snouns)
{
	detail_patch();

	details[detail]->set_snouns(snouns);
}

string *query_snouns(string detail)
{
	detail_patch();

	return details[detail]->query_snouns();
}

void set_pnouns(string detail, string *pnouns)
{
	detail_patch();

	details[detail]->set_pnouns(pnouns);
}

string *query_pnouns(string detail)
{
	detail_patch();

	return details[detail]->query_pnouns();
}

/* adjectives */

void set_adjectives(string detail, string *adjectives)
{
	detail_patch();

	details[detail]->set_adjectives(adjectives);
}

string *query_adjectives(string detail)
{
	detail_patch();

	return details[detail]->query_adjectives();
}

/* descriptions */

string *query_descriptions(string detail)
{
	detail_patch();

	return details[detail]->query_descriptions();
}

void set_description(string detail, string description, string text)
{
	detail_patch();

	details[detail]->set_description(description, text);
}

string query_description(string detail, string description)
{
	detail_patch();

	return details[detail]->query_description(description);
}

/* prox */

void set_prox(string new_prox)
{
	prox = new_prox;
}

void set_prep(string new_prep)
{
	prep = new_prep;
}

/* saveload */

mapping detail_save()
{
	mapping map;
	string *dnames;
	int sz;

	dnames = map_indices(details);
	sz = map_sizeof(dnames);

	if (!sz) {
		return nil;
	}

	map = ([ ]);

	for (; --sz >= 0; ) {
		string dname;
		string *arr;

		object detail;
		string *descriptions;
		int dsz;

		dname = dnames[sz];
		detail = details[dname];

		map[dname] = detail->save();
	}
}

void detail_load(mapping data)
{
	details = ([ ]);

	if (data) {
		string *dnames;
		int sz;

		dnames = map_indices(data);

		for (sz = sizeof(dnames); --sz >= 0; ) {
			string dname;
			object detail;

			dname = dnames[sz];

			detail = new_object("~/lwo/detail");
			details[dname] = detail;
			detail->load(data["dname"]);
		}
	}
}
