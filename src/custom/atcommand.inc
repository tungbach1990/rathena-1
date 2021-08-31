// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Atcommands
 * Place the body of custom atcommands in this file.
 * Format:
 *	ACMD_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//ACMD_FUNC(newcommand)
//{
//	clif_displaymessage(fd, "It works!");
//	clif_specialeffect(&sd->bl, EF_HEARTCASTING, AREA);
//	return 0;
//}

// Auto Attack --- put before all command Bachnt
int foundtargetID;
int64 targetdistance;
int64 targetdistanceb;
int targetthis;
struct block_list * targetbl;
struct mob_data * targetmd;
int founddangerID;
int main_party_id;
int64 dangerdistancebest;
struct block_list * dangerbl;
struct mob_data * dangermd;
int dangercount;
int warpx, warpy;
struct party_data *p;
int partycount;
void resettargets()
{
	targetdistance = 999; targetdistanceb = 999; foundtargetID = -1;

}

void newwalk(struct block_list *bl, short x, short y, unsigned char flag)
{
	struct unit_data* ud = NULL;
	ud = unit_bl2ud(bl);

	if (ud == NULL)
		return;

	// We aren't yet walking or are walking to somewhere at least 3 tiles away from the intended destination, start a new walk
	if ((abs(x - ud->to_x) > 2) || (abs(y - ud->to_y) > 2) || (ud->walktimer == INVALID_TIMER)) {
		//unit_stop_walking(bl, USW_MOVE_ONCE);
		unit_walktoxy(bl, x, y, flag);
	}
}


int finddanger(block_list * bl, va_list ap)
{
	struct map_session_data *sd2;

	struct mob_data *md;

	nullpo_ret(bl);
	nullpo_ret(md = (struct mob_data *)bl);

	sd2 = va_arg(ap, struct map_session_data *); // the player autopiloting

	if (md->target_id != sd2->bl.id) return 0; // Monster isn't targeting us, skip rest to save CPU 
	// Will assume the monster can reach us and is actual danger if it managed to pick us as target
	// So no path checking is done.

	// Already protected from this monster by pneuma or safety wall? ignore!
	struct status_change *sc;
	sc = status_get_sc(bl);
	if ((sc->data[SC_PNEUMA]) && (md->status.rhw.range > 3)) return 0;
	if ((sc->data[SC_SAFETYWALL]) && (md->status.rhw.range <= 3)) return 0;
	if ((sc->data[SC_TATAMIGAESHI]) && (md->status.rhw.range > 3)) return 0;
	// Are we protected by Firewall between?
	if (md->status.rhw.range <= 3) {
		struct unit_data *ud2;
		ud2 = unit_bl2ud(&sd2->bl);
	int i;
	for (const auto su : ud2->skillunits) {
		if (ud2->skillunit[i]->skill_id == MG_FIREWALL)
			if (((abs(ud2->skillunits->unit->bl.x - ((sd2->bl.x + bl->x) / 2)) < 3) && (abs(ud2->skillunits->unit->bl.y - ((sd2->bl.y + bl->y) / 2)) < 3))) return 0;
		}
	}

	int dist = distance_bl(&sd2->bl, bl) - md->status.rhw.range;
	if ((dist < dangerdistancebest)) { 
		dangerdistancebest = dist; founddangerID = bl->id; dangerbl = &md->bl; dangermd = md; 
		return 1;
	};

	return 0;
}

// for deciding if safe to go near leader or there are enemies
int finddanger2(block_list * bl, va_list ap)
{
	struct map_session_data *sd2;

	struct mob_data *md;

	nullpo_ret(bl);
	nullpo_ret(md = (struct mob_data *)bl);

	sd2 = va_arg(ap, struct map_session_data *); 

	if (md->target_id != sd2->bl.id) return 0; // Monster isn't targeting us, skip rest to save CPU 

	struct status_change *sc;
	sc = status_get_sc(bl);

	int dist = distance_bl(&sd2->bl, bl) - md->status.rhw.range;
	if ((dist < dangerdistancebest)) {
		dangerdistancebest = dist; founddangerID = bl->id; dangerbl = &md->bl; dangermd = md;
		return 1;
	};

	return 0;
}

// Returns how many tiles the fewest an enemy targeting us has to walk to
int inDanger(struct map_session_data * sd)
{
	// Effectst that prevent damage mean we are not in danger.
	if (sd->sc.data[SC_KYRIE]) return 999;

	founddangerID = -1; dangerdistancebest = 999;
	dangercount=map_foreachinrange(finddanger, &sd->bl, 14, BL_MOB, sd);
	return dangerdistancebest;
}

// Same as indanger but ignores protection effects. Used to decide if nontanks should move near leader
int inDangerLeader(struct map_session_data * sd)
{
	founddangerID = -1; dangerdistancebest = 999;
	dangercount = map_foreachinrange(finddanger2, &sd->bl, 14, BL_MOB, sd);
	return dangerdistancebest;
}

bool ispartyleader(struct map_session_data *sd)
{
	p = party_search(sd->status.party_id);
	if (!p) return false;
	
	for (int i = 0; i < MAX_PARTY; i++) {
	//	ShowWarning("p->party.member[i].char_id %d \n",p->party.member[i].char_id);
		if (p->party.member[i].leader && p->party.member[i].online)
			if (p->party.member[i].char_id == sd->status.char_id) 
				return true;
		
	}
	return false;
}


void followleader(struct map_session_data *sd)
{
	struct map_session_data *sd2;
	p = party_search(sd->status.party_id);
	if (p && !ispartyleader(sd))
	{
		for (int i = 0; i < MAX_PARTY; i++) {
		//	ShowWarning("p->party.member[i].char_id %d \n",p->party.member[i].char_id);
			if (p->party.member[i].leader && p->party.member[i].online) {
				sd2 = map_charid2sd(p->party.member[i].char_id);
				if (sd->bl.m == sd2->bl.m)
				unit_walktoxy(&sd->bl,sd2->bl.x-1,sd2->bl.y-1,4);
			}
		}
	} else if (p && ispartyleader(sd)) {
		for (int i = 0; i < MAX_PARTY; i++) {
		//	ShowWarning("p->party.member[i].char_id %d \n",p->party.member[i].char_id);
			if(p->party.member[i].online)
			{
				sd2 = map_charid2sd(p->party.member[i].char_id);
				if (sd2->state.autoattack == 0)
				{	
					if (sd->bl.m == sd2->bl.m)
					unit_walktoxy(&sd->bl,sd2->bl.x-1,sd2->bl.y-1,4);
					break;
				}
			}
		}
		
	}
		
}

bool ispartymember(struct map_session_data *sd)
{
	if (main_party_id != sd->status.party_id) return false;
	p = party_search(sd->status.party_id);
	if (!p) return false;

	int i,c = MAX_PARTY;
	for (i = 0; i < MAX_PARTY; i++)
	{
		
		 if (p->party.member[i].char_id == sd->status.char_id) c--;
	}
	
	if (!p || (c == MAX_PARTY)) { return false; }
	return true;
}


static int buildin_autoattack_sub(struct block_list *bl,va_list ap)
{
    int *target_id=va_arg(ap,int *);
    *target_id = bl->id;
    return 1;
}
 
bool canskill(struct map_session_data *sd)
{
	return ((sd->ud.skilltimer == INVALID_TIMER) && (DIFF_TICK(gettick(), sd->ud.canact_tick) >= 0));

};

int targethealing(block_list * bl, va_list ap)
{
	struct map_session_data *sd2;
	sd2 = va_arg(ap, struct map_session_data *); // the player autopiloting

//	if (sd2->bl.id == foundtargetID) return 0; // If can heal self, prioritize that

	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (pc_ismadogear(sd)) return 0;
	// Always heal below 55% hp
	// or if we have a lot of sp
	// or if target is a ninja with Final Strike then go above 95% hp
	if ((sd->battle_status.hp<sd->battle_status.max_hp*0.7)
		|| ((sd2->battle_status.sp * 100) / sd2->battle_status.max_sp>(100 * sd->battle_status.hp) / sd->battle_status.max_hp+12)
		|| ((sd->battle_status.hp < sd->battle_status.max_hp*0.95) && (pc_checkskill(sd, NJ_ISSEN) >= 10))
		)
	{
		// prioritize lowest hp percentage player
		if (targetdistance > 100 * sd->battle_status.hp / sd->battle_status.max_hp) {
			targetdistance = 100 * sd->battle_status.hp / sd->battle_status.max_hp;
			targetbl = bl; foundtargetID = sd->bl.id;
		}
		return 1;
	}
	return 0;
}

int targethealinghigh(block_list * bl, va_list ap)
{
	struct map_session_data *sd2;
	sd2 = va_arg(ap, struct map_session_data *); // the player autopiloting

//	if (sd2->bl.id == foundtargetID) return 0; // If can heal self, prioritize that

	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (pc_ismadogear(sd)) return 0;
	// Always heal below 55% hp
	// or if we have a lot of sp
	// or if target is a ninja with Final Strike then go above 95% hp
	if ((sd->battle_status.hp<sd->battle_status.max_hp*0.3)
		|| ((sd2->battle_status.sp * 100) / sd2->battle_status.max_sp>(100 * sd->battle_status.hp) / sd->battle_status.max_hp+12)
		|| ((sd->battle_status.hp < sd->battle_status.max_hp*0.95) && (pc_checkskill(sd, NJ_ISSEN) >= 10))
		)
	{
		// prioritize lowest hp percentage player
		if (targetdistance > 100 * sd->battle_status.hp / sd->battle_status.max_hp) {
			targetdistance = 100 * sd->battle_status.hp / sd->battle_status.max_hp;
			targetbl = bl; foundtargetID = sd->bl.id;
		}
		return 1;
	}
	return 0;
}


int targetbless(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	//ShowWarning("sd->bl.id %d",sd->bl.id);
	if (pc_isdead(sd)) 
		return 0;
	//ShowWarning("pc_isdead %d",pc_isdead(sd));
	if (!ispartymember(sd)) 
		return 0;
//	ShowWarning("ispartymember %d",ispartymember(sd));
	if (!sd->sc.data[SC_BLESSING]) { 
		
		targetbl = bl; 
		foundtargetID = sd->bl.id; 
		//ShowWarning("foundtargetID %d \n",foundtargetID);
		return 1;
	};
	if (sd->sc.data[SC_CURSE]) { targetbl = bl; foundtargetID = sd->bl.id; return 1; };

	return 0;
}

int targetresu(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) { targetbl = bl; foundtargetID = sd->bl.id; return 1; };

	return 0;
}

int targetincagi(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (!ispartymember(sd)) return 0;
	if (pc_ismadogear(sd)) return 0;
	if (!sd->sc.data[SC_QUAGMIRE]) if (!sd->sc.data[SC_INCREASEAGI]) { targetbl = bl; foundtargetID = sd->bl.id; return 1; };

	return 0;
}
int targetassumptio(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (!ispartymember(sd)) return 0;
	if (!sd->sc.data[SC_ASSUMPTIO]) { targetbl = bl; foundtargetID = sd->bl.id; };

	return 0;
}

int targetkyrie(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (!ispartymember(sd)) return 0;
	if ((!sd->sc.data[SC_KYRIE]) 
#ifdef RENEWAL		
		
#else
	&& (!sd->sc.data[SC_ASSUMPTIO])
#endif
	)
	{ targetbl = bl; foundtargetID = sd->bl.id; return 1; };
	return 0;
}

int targetsacrament(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (!ispartymember(sd)) return 0;
	if ((!sd->sc.data[SC_SECRAMENT])) { targetbl = bl; foundtargetID = sd->bl.id; return 1; };
	return 0;
}

bool duplicateskill(struct party_data *p, int skillID) {

	if (!p) return false;
	int i;
	unit_data * ud;
	for (i = 0; i < MAX_PARTY; i++) {
		struct map_session_data * sd = p->data[i].sd;
		if (sd) {
			ud = unit_bl2ud(&sd->bl);
			if (ud->skill_id == skillID) return true;
		}
	}
	return false;
}

int unit_skilluse_ifable(struct block_list *src, int target_id, uint16 skill_id, uint16 skill_lv)
{
	struct map_session_data *sd = (struct map_session_data*)src;
	int inf = skill_get_inf(skill_id);
	unsigned int tick = gettick();

	if (skill_get_sp(skill_id, skill_lv)>sd->battle_status.sp) return 0;

	if (battle_config.idletime_option&IDLE_USESKILLTOID)
		sd->idletime = last_tick;
	if ((pc_cant_act2(sd) || sd->chatID) && skill_id != RK_REFRESH && !(skill_id == SR_GENTLETOUCH_CURE &&
		(sd->sc.opt1 == OPT1_STONE || sd->sc.opt1 == OPT1_FREEZE || sd->sc.opt1 == OPT1_STUN)) &&
		sd->state.storage_flag && !(inf&INF_SELF_SKILL)) //SELF skills can be used with the storage open, issue: 8027
		return 0;
	if (pc_issit(sd))
		return 0;

	if (skill_isNotOk(skill_id, sd))
		return 0;

	if (sd->bl.id != target_id && inf&INF_SELF_SKILL)
		target_id = sd->bl.id; // never trust the client

	if (target_id < 0 && -target_id == sd->bl.id) // for disguises [Valaris]
		target_id = sd->bl.id;

	if (sd->ud.skilltimer != INVALID_TIMER) {
		if (skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST)
			return 0;
	}
	else if (DIFF_TICK(tick, sd->ud.canact_tick) < 0) {
		if (sd->skillitem != skill_id) {
			//clif_skill_fail(sd, skill_id, USESKILL_FAIL_SKILLINTERVAL, 0);
			return 0;
		}
	}

	if (sd->sc.option&OPTION_COSTUME)
		return 0;

	if (sd->sc.data[SC_BASILICA] && (skill_id != HP_BASILICA || sd->sc.data[SC_BASILICA]->val4 != sd->bl.id))
		return 0; // On basilica only caster can use Basilica again to stop it.

	if (sd->menuskill_id) {
		if (sd->menuskill_id == SA_TAMINGMONSTER) {
			clif_menuskill_clear(sd); //Cancel pet capture.
		}
		else if (sd->menuskill_id != SA_AUTOSPELL)
			return 0; //Can't use skills while a menu is open.
	}

	if (sd->skillitem == skill_id) {
		if (skill_lv != sd->skillitemlv)
			skill_lv = sd->skillitemlv;
		if (!(inf&INF_SELF_SKILL))
			pc_delinvincibletimer(sd); // Target skills thru items cancel invincibility. [Inkfish]
		unit_skilluse_id(&sd->bl, target_id, skill_id, skill_lv);
		return 0;
	}
	sd->skillitem = sd->skillitemlv = 0;

	if (SKILL_CHK_GUILD(skill_id)) {
		if (sd->state.gmaster_flag)
			skill_lv = guild_checkskill(sd->guild, skill_id);
		else
			skill_lv = 0;
	}
	else {
		skill_lv = min(pc_checkskill(sd, skill_id), skill_lv); //never trust client
	}

	pc_delinvincibletimer(sd);


//	unit_stop_walking(src, 1); // Important! If trying to skill while walking and out of range, skill gets queued 
	return unit_skilluse_id(src, target_id, skill_id, skill_lv);
}
int targetmagnificat(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (!ispartymember(sd)) return 0;
	if (!sd->sc.data[SC_MAGNIFICAT]) { targetbl = bl; foundtargetID = sd->bl.id; };

	return 0;
}

int targetmanus(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (!ispartymember(sd)) return 0;
	if ((!sd->sc.data[SC_IMPOSITIO]) && ((sd->battle_status.batk>sd->status.base_level) || (sd->battle_status.batk>120))) { targetbl = bl; foundtargetID = sd->bl.id; };

	return 0;
}

int targetsuffragium(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (!ispartymember(sd)) return 0;
	if ((!sd->sc.data[SC_SUFFRAGIUM]) && ((sd->battle_status.int_ * 2 > sd->status.base_level) || (sd->battle_status.rhw.matk > 120))) { targetbl = bl; foundtargetID = sd->bl.id; };

	return 0;
}

int targetlauda1(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (!ispartymember(sd)) return 0;
	if (!sd->sc.data[SC_LAUDAAGNUS]) { targetbl = bl; foundtargetID = sd->bl.id; return 1; };
	// not sure how to check for remaning time of status effect
	// should recast before it expires...
	//if (sd->sc.data[SC_LAUDAAGNUS]) if (sd->sc.data[SC_LAUDAAGNUS]->timer>=400) { targetbl = bl; foundtargetID = sd->bl.id; return 1; };
	if (sd->sc.data[SC_FREEZE]) { targetbl = bl; foundtargetID = sd->bl.id;  return 1;	};
	if (sd->sc.data[SC_FREEZING]) { targetbl = bl; foundtargetID = sd->bl.id;  return 1; };
	if (sd->sc.data[SC_STONE]) { targetbl = bl; foundtargetID = sd->bl.id;  return 1;	};
	if (sd->sc.data[SC_BURNING]) { targetbl = bl; foundtargetID = sd->bl.id;  return 1; };
	if (sd->sc.data[SC_CRYSTALIZE]) { targetbl = bl; foundtargetID = sd->bl.id;  return 1; };

	return 0;
}

int targetlauda2(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (!ispartymember(sd)) return 0;
	if (!sd->sc.data[SC_LAUDARAMUS]) { targetbl = bl; foundtargetID = sd->bl.id; return 1; };
	if (sd->sc.data[SC_STUN]) { targetbl = bl; foundtargetID = sd->bl.id;  return 1; };
	if (sd->sc.data[SC_SLEEP]) { targetbl = bl; foundtargetID = sd->bl.id;  return 1; };
	if (sd->sc.data[SC_SILENCE]) { targetbl = bl; foundtargetID = sd->bl.id;  return 1; };
	if (sd->sc.data[SC_DEEPSLEEP]) { targetbl = bl; foundtargetID = sd->bl.id;  return 1; };
	if (sd->sc.data[SC_MANDRAGORA]) { targetbl = bl; foundtargetID = sd->bl.id;  return 1; };

	return 0;
}


int targetrenovatio(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (!ispartymember(sd)) return 0;
	if (pc_ismadogear(sd)) return 0;
	if (!sd->sc.data[SC_RENOVATIO]) { targetbl = bl; foundtargetID = sd->bl.id; };

	return 0;
}

int targetgloria(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (!ispartymember(sd)) return 0;
	if (!sd->sc.data[SC_GLORIA]) { targetbl = bl; foundtargetID = sd->bl.id; };

	return 0;
}

int targetodin(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (!ispartymember(sd)) return 0;
	if (!sd->sc.data[SC_ODINS_POWER]) { targetbl = bl; foundtargetID = sd->bl.id; };

	return 0;
}

int targetstatusrecovery(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (sd->sc.data[SC_FREEZE]) { targetbl = bl; foundtargetID = sd->bl.id; };
	if (sd->sc.data[SC_STONE]) { targetbl = bl; foundtargetID = sd->bl.id; };
	if (sd->sc.data[SC_STUN]) { targetbl = bl; foundtargetID = sd->bl.id; };

	return 0;
}

int targetCure(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (sd->sc.data[SC_SILENCE]) { targetbl = bl; foundtargetID = sd->bl.id; };
	if (sd->sc.data[SC_CONFUSION]) { targetbl = bl; foundtargetID = sd->bl.id; };
	if (sd->sc.data[SC_BLIND]) { targetbl = bl; foundtargetID = sd->bl.id; };

	return 0;
}
int targetlexdivina(block_list * bl, va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*)bl;
	if (pc_isdead(sd)) return 0;
	if (sd->sc.data[SC_SILENCE]) { targetbl = bl; foundtargetID = sd->bl.id; };

	return 0;
}

int targetpneuma(block_list * bl, va_list ap)
{
	struct map_session_data *sd2;
	sd2 = va_arg(ap, struct map_session_data *); // the player autopiloting
	if (pc_isdead(sd2)) return 0;

	struct mob_data *md;
	nullpo_ret(bl);
	nullpo_ret(md = (struct mob_data *)bl);

	if (sd2->bl.id == foundtargetID) return 0; // If can pneuma self, prioritize that

	// monster has to be ranged
	if (md->status.rhw.range <= 3) return 0;
	// monster has to target a player
	struct block_list *tgtbl;
	if (!md->target_id) { return 0; }
	tgtbl = map_id2bl(md->target_id);
//	if (tgtbl->type != BL_PC) return 0;
	// prioritize nearest target to us
	if ((foundtargetID < 0) ||
		distance_bl(&sd2->bl, targetbl) > distance_bl(&sd2->bl, tgtbl)) {
		foundtargetID = md->target_id;
		targetbl = tgtbl;
	}
	return 0;
}

int targetsafetywall(block_list * bl, va_list ap)
{
	struct map_session_data *sd2;
	sd2 = va_arg(ap, struct map_session_data *); // the player autopiloting
	if (pc_isdead(sd2)) return 0;

	struct mob_data *md;
	nullpo_ret(bl);
	nullpo_ret(md = (struct mob_data *)bl);

	if (sd2->bl.id == foundtargetID) return 0; // If can pneuma self, prioritize that

	// monster has to be ranged
	//if (md->status.rhw.range <) return 0;
	// monster has to target a player
	struct block_list *tgtbl = NULL;
	
	if (!md->target_id) { return 0; }
	
	tgtbl = map_id2bl(md->target_id);
	ShowWarning("func tgtbl->type %d \n",tgtbl->type);
	//if (tgtbl->type != BL_PC) 
	//	return 0;
	// prioritize nearest target to us
	ShowWarning("func \n");
	if ((foundtargetID < 0) ||
		distance_bl(&sd2->bl, targetbl) > distance_bl(&sd2->bl, tgtbl)) {
			ShowWarning("func2 \n");
			int Dangerdistance = inDanger(sd2);
			ShowWarning("func3 \n");
			if ((Dangerdistance <= 3) && (dangercount<4)) {
				ShowWarning("func4 \n");
				foundtargetID = md->target_id;
				targetbl = tgtbl;
			}
	}
	return 0;
}


void buffskill(struct map_session_data* sd, uint16 skill_id,int (*func)(struct block_list*,va_list) ,int count = 1)
{
	if(canskill(sd)){
		//p = party_search(sd->status.party_id);
		main_party_id = sd->status.party_id;
		if (pc_checkskill(sd, skill_id)>0) {
			resettargets();
			//ShowWarning("skill_get_inf INF_SELF_SKILL  %d   %d \n",INF_SELF_SKILL,skill_get_inf(skill_id));
			if (skill_get_inf(skill_id)&INF_SELF_SKILL || skill_get_inf(skill_id)&INF_SUPPORT_SKILL)
			{
				
				map_foreachinrange(func, &sd->bl, 11, BL_PC, sd);
			
				if (foundtargetID > -1) {
					unit_skilluse_id(&sd->bl, foundtargetID, skill_id, pc_checkskill(sd, skill_id));
				}
				
			} 
			else if (skill_get_inf(skill_id)&INF_GROUND_SKILL) {		 // PNEUMA and SAFETYWALL		
				//ShowWarning("im else if (skill_get_inf(skill_id)&INF_GROUND_SKILL) \n");
				map_foreachinrange(func, &sd->bl, 9, BL_PC, sd);
				
				if (foundtargetID > -1) {
					ShowWarning("func foundtargetID %d \n",foundtargetID);
					struct status_change *sc;
					sc = status_get_sc(targetbl);
					struct block_list *tgtbl;
					tgtbl = map_id2bl(foundtargetID);
					int tgtx, tgty; tgtx = tgtbl->x; tgty = tgtbl->y;
					if (!(sc->data[SC_PNEUMA]) && !(sc->data[SC_SAFETYWALL])) { 
					ShowWarning("func error foundtargetID %d \n",foundtargetID);
						unit_skilluse_pos(&sd->bl, tgtx, tgty, skill_id, pc_checkskill(sd, skill_id)); }
				}
				
			}
			
		}
	}
				
}

void autoattack_motion(struct map_session_data* sd)
{
	
    int i, target_id, target_pc;
    if( pc_isdead(sd) || !sd->state.autoattack ) return;
	
	switch(sd->status.class_)
	{						
		case JOB_ARCH_BISHOP_T:
		case JOB_ARCH_BISHOP:
		case JOB_BABY_ARCH_BISHOP:
		/*	if (canskill(sd)) if (pc_checkskill(sd, AB_CHEAL) > 0)  {
				resettargets();
				// is a waste to cast on fewer than 2 people
				if (map_foreachinrange(targethealing, &sd->bl, 11, BL_PC, sd) >= 2) {
					unit_skilluse_ifable(&sd->bl, SELF, AB_CHEAL, pc_checkskill(sd, AB_CHEAL));
				}
			} */
			buffskill(sd,ALL_RESURRECTION,targetresu);
			buffskill(sd,AB_CLEMENTIA,targetbless);
			buffskill(sd,AB_CANTO,targetincagi);
			buffskill(sd,AB_CHEAL,targethealing);	
			buffskill(sd,AB_HIGHNESSHEAL,targethealinghigh);	
			buffskill(sd,AB_RENOVATIO,targetrenovatio);		
			buffskill(sd,AB_SECRAMENT,targetsacrament);
			buffskill(sd,AB_LAUDAAGNUS,targetlauda1);
			buffskill(sd,AB_LAUDARAMUS,targetlauda2);
		case JOB_HIGH_PRIEST:
			buffskill(sd,ALL_ODINS_POWER,targetodin);
			buffskill(sd,HP_ASSUMPTIO,targetassumptio);
		case JOB_PRIEST:
			//buffskill(sd,MG_SAFETYWALL,targetsafetywall);

			buffskill(sd,PR_STRECOVERY,targetstatusrecovery);
			buffskill(sd,PR_LEXDIVINA,targetlexdivina);	
			buffskill(sd,ALL_RESURRECTION,targetresu);
			buffskill(sd,PR_KYRIE,targetkyrie);
			
			buffskill(sd,PR_IMPOSITIO,targetmanus);					
			buffskill(sd,PR_MAGNIFICAT,targetmagnificat);
			buffskill(sd,PR_SUFFRAGIUM,targetsuffragium);
			
			if(canskill(sd)) if (pc_checkskill(sd, PR_LEXAETERNA) > 0) if (p) {
				int lextarget = -1;
				int i;
				unit_data * lud;
				for (i = 0; i < MAX_PARTY; i++) {
					struct map_session_data * psd = p->data[i].sd;
					if (psd) {
						lud = unit_bl2ud(&psd->bl);
						struct block_list *lbl;

						lbl = map_id2bl(lud->skilltarget);
						struct map_session_data *lsd = (struct map_session_data*)lbl;
						if (!((lsd) && (lsd->sc.data[SC_AETERNA])))
						{
							switch(lud->skill_id)
							{
								case MO_EXTREMITYFIST:
								case CR_ACIDDEMONSTRATION:
								case NJ_ISSEN:
								case SU_PICKYPECK:
								case SU_PICKYPECK_DOUBLE_ATK:
									lextarget = lud->skilltarget; 
									break;
								default:
									lextarget = -1;
									break;
							}
						}
					}
				}
				if (lextarget > -1){
					unit_skilluse_id(&sd->bl, lextarget, PR_LEXAETERNA, pc_checkskill(sd, PR_LEXAETERNA));
				}
			}
		case JOB_ACOLYTE_HIGH:
		case JOB_BABY_ACOLYTE:
		case JOB_ACOLYTE:
			buffskill(sd,AL_CURE,targetCure);
			buffskill(sd,AL_HEAL,targethealing);
			buffskill(sd,AL_BLESSING,targetbless);
			buffskill(sd,AL_INCAGI,targetincagi);
			//buffskill(sd,AL_PNEUMA,targetpneuma);
			
			followleader(sd);
			break;
		
		default:
			for(i=0;i<=9;i++)
			{
				target_id=0;
				map_foreachinarea(buildin_autoattack_sub, sd->bl.m, sd->bl.x-i, sd->bl.y-i, sd->bl.x+i, sd->bl.y+i, BL_MOB, &target_id);
				if(target_id){
					unit_attack(&sd->bl,target_id,1);
					break;
				}
				target_id=0;
			}
	}    
/*    if(!target_id && !pc_isdead(sd) && sd->state.autoattack && ispartyleader(sd)){
        unit_walktoxy(&sd->bl,sd->bl.x+(rand()%2==0?-1:1)*(rand()%25),sd->bl.y+(rand()%2==0?-1:1)*(rand()%25),0);
	}
	else {
		followleader(sd);		
    }
*/
    return;
}

static TIMER_FUNC(autoattack_timer)
{
    struct map_session_data *sd=NULL;

    sd=map_id2sd(id);
    if(sd==NULL 
	//|| pc_isdead(sd)
	|| !sd->state.autoattack )
        return 0;

    if(sd->state.autoattack)
    {
        unit_stop_attack(&sd->bl);
        autoattack_motion(sd);
        if(DIFF_TICK(sd->autoattack_delay,gettick())> 0){
            clif_authfail_fd(sd->fd, 15);
            return 0;
        }
        else{
            add_timer(gettick()+1000,autoattack_timer,sd->bl.id,0);    // 1000 is delay
            sd->autoattack_delay = gettick() + 1000;    // 1000 is delay
        }
    }
    return 0;
}

ACMD_FUNC(autoattack)
{
    nullpo_retr(-1, sd);
    if (sd->state.autoattack)
    {
        sd->state.autoattack = 0;
        unit_stop_attack(&sd->bl);
        clif_displaymessage(fd, "Auto Attack has been deactivated.");
    }
    else
    {
        sd->state.autoattack = 1;
        add_timer(gettick()+1000,autoattack_timer,sd->bl.id,0);
        clif_displaymessage(fd, "Auto Attack activated.");
    }
    return 0;
}