string process_aliases(string str, mapping aliases, string *disabled, varargs string *active)
{
	string firstword;
	string expansion;
	string whole;
	
	if (!active) {
		active = ({ });
	}
	
	if (str[0] == '\'' && !sizeof(disabled & ({ "'" }) )) {
		str = "say " + str[1 ..];
	} else if (str[0] == ':' && !sizeof(disabled & ({ ":" }) )) {
		str = "emote " + str[1 ..];
	}
	
	sscanf(str, "%s %s", firstword, str);
	
	if (!firstword) {
		firstword = str;
		str = "";
	} else {
		str = " " + str;
	}
	
	/* avoid endless loop */
	if (sizeof(active & ({ firstword }) )) {
		return firstword + str;
	}
	
	expansion = aliases[firstword];
	
	if (expansion) {
		active |= ({ firstword });
		return process_aliases(expansion + str, aliases, disabled, active);
	}
	
	if (sizeof(disabled & ({ firstword }) )) {
		return firstword + str;
	}

	switch(firstword) {
	case "n":
		expansion = "go north";
		break;
	case "e":
		expansion = "go east";
		break;
	case "s":
		expansion = "go south";
		break;
	case "w":
		expansion = "go west";
		break;
	case "l":
		expansion = "look";
		break;
	case "chat":
		expansion = "chpost chat";
		break;
	case "wiz":
		expansion = "chpost wizard";
		break;
	case "adm":
		expansion = "chpost admin";
		break;
	}

	if (expansion) {
		active |= ({ firstword });
		return process_aliases(expansion + str, aliases, disabled, active);
	}

	return firstword + str;
}
