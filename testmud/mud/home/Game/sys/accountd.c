/* account manager */

/* -1 = no such user */
/* 0 = root */
/* else = plain user */
int username_to_uid(string username)
{
	switch(username)
	{
	case "root":
		return 0;
	case "shentino":
		return 1000;
	default:
		return -1;
	}
}
