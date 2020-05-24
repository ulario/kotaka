inherit "verb";

int pre_bind(object actor, mapping roles)
{
	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return 1;
	}
}

object *pre_filter(object actor, string role, object *cand)
{
	return cand - ({ nil });
}
