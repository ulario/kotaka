private int hp;

int query_hp()
{
	return hp;
}

void set_hp(int new_hp)
{
	hp = new_hp;
}

int subtract_hp(int damage)
{
	return hp -= damage;
}

int add_hp(int healing)
{
	return hp += healing;
}
