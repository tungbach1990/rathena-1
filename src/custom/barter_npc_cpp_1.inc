		/*
			//Hidden/Disabled npc.
	if (nd->class_ < 0 || nd->sc.option&(OPTION_INVISIBLE|OPTION_HIDE))
		return 1;

	if (sd->state.block_action & PCBLOCK_NPCCLICK) {
		clif_msg(sd, WORK_IN_PROGRESS);
		return 1;
	}

	switch(nd->subtype) {
	
	*/
		
		// Barter Shop ]BACHNT]
		case NPCTYPE_BARTERSHOP:
			clif_npc_barter_open(sd,nd);
			break;
		case NPCTYPE_EXBARTERSHOP:
			clif_npc_expanded_barter_open(sd,nd);
			break;