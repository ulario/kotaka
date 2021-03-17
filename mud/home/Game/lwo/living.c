int hp;
int attack_bonus;
int defense_bonus;

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

int query_attack_bonus()
{
	return attack_bonus;
}

void set_attack_bonus(int new_bonus)
{
	attack_bonus = new_bonus;
}

int query_defense_bonus()
{
	return defense_bonus;
}

void set_defense_bonus(int new_bonus)
{
	defense_bonus = new_bonus;
}
