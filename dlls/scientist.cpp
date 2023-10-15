void CDeadScientist :: Spawn( )
{
	//PRECACHE_MODEL("models/scientist.mdl");
	//SET_MODEL(ENT(pev), "models/scientist.mdl");
//я так понял тут прекеш, что закментил и спавн вместе. Потому я вставил тот код. Тоже вместе.
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC 
	else
		PRECACHE_MODEL("models/scientist.mdl");

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC 
	else
		SET_MODEL(ENT(pev), "models/scientist.mdl");

	pev->effects		= 0;
	pev->sequence		= 0;
	// Corpses have less health
	pev->health			= 8;//gSkillData.scientistHealth;
	
	m_bloodColor = BLOOD_COLOR_RED;

	if ( pev->body == -1 )
	{// -1 chooses a random head
		pev->body = RANDOM_LONG(0, NUM_SCIENTIST_HEADS-1);// pick a head, any head
	}
	// Luther is black, make his hands black
	if ( pev->body == HEAD_LUTHER )
		pev->skin = 1;
	else
		pev->skin = 0;

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );
	if (pev->sequence == -1)
	{
		ALERT ( at_console, "Dead scientist with bad pose\n" );
	}

	//	pev->skin += 2; // use bloody skin -- UNDONE: Turn this back on when we have a bloody skin again!
	MonsterInitDead();
}