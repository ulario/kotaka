inherit "verb";

int pre_bind(object actor, mapping roles)
{
	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return 1;
	}

	if (!actor->query_character_lwo()) {
		send_out("You are not a character.\n");
		return 1;
	}

	if (!actor->query_living_lwo()) {
		send_out("You're dead, you can't do that.\n");
		return 1;
	}
}
