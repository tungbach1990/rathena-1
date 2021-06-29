
BUILDIN_FUNC(callshop)
{
	TBL_PC *sd = NULL;
	struct npc_data *nd;
	const char *shopname;
	int flag = 0;
	if (!script_rid2sd(sd)) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}
	shopname = script_getstr(st, 2);
	if (script_hasdata(st,3))
		flag = script_getnum(st,3);
	nd = npc_name2id(shopname);
	if( !nd || nd->bl.type != BL_NPC || (nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP && nd->subtype != NPCTYPE_POINTSHOP && nd->subtype != NPCTYPE_MARKETSHOP && nd->subtype != NPCTYPE_BARTERSHOP && nd->subtype != NPCTYPE_EXBARTERSHOP) ) { // Bartershop Bachnt
		ShowError("buildin_callshop: Shop [%s] not found (or NPC is not shop type)\n", shopname);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if (nd->subtype == NPCTYPE_SHOP) {
		// flag the user as using a valid script call for opening the shop (for floating NPCs)
		sd->state.callshop = 1;

		switch (flag) {
			case 1: npc_buysellsel(sd,nd->bl.id,0); break; //Buy window
			case 2: npc_buysellsel(sd,nd->bl.id,1); break; //Sell window
			default: clif_npcbuysell(sd,nd->bl.id); break; //Show menu
		}
	}
#if PACKETVER >= 20131223
	else if (nd->subtype == NPCTYPE_MARKETSHOP) {
		unsigned short i;

		for (i = 0; i < nd->u.shop.count; i++) {
			if (nd->u.shop.shop_item[i].qty)
				break;
		}

		if (i == nd->u.shop.count) {
			clif_messagecolor(&sd->bl, color_table[COLOR_RED], msg_txt(sd, 534), false, SELF);
			return SCRIPT_CMD_FAILURE;
		}

		sd->npc_shopid = nd->bl.id;
		clif_npc_market_open(sd, nd);
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}
#endif
// Bartershop Bachnt
	else if (nd->subtype == NPCTYPE_BARTERSHOP) {
		sd->npc_shopid = nd->bl.id;
		clif_npc_barter_open(sd, nd);
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}
	else if (nd->subtype == NPCTYPE_EXBARTERSHOP) {
		sd->npc_shopid = nd->bl.id;
		clif_npc_expanded_barter_open(sd, nd);
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}
	else
		clif_cashshop_show(sd, nd);

	sd->npc_shopid = nd->bl.id;
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(npcshopitem)
{
	const char* npcname = script_getstr(st, 2);
	struct npc_data* nd = npc_name2id(npcname);
	int n, i;
	int amount;
	int currencycount;	// Bartershop Bachnt
	uint16 offs = 2;
// Bartershop Bachnt
	if( !nd || ( nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP && nd->subtype != NPCTYPE_POINTSHOP && nd->subtype != NPCTYPE_MARKETSHOP && 
				nd->subtype != NPCTYPE_BARTERSHOP && 
				nd->subtype != NPCTYPE_EXBARTERSHOP) ) { // Not found.
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

#if PACKETVER >= 20131223
	if (nd->subtype == NPCTYPE_MARKETSHOP) {
		offs = 3;
		npc_market_delfromsql_(nd->exname, 0, true);
	}
#endif

	// get the count of new entries
	amount = (script_lastdata(st)-2)/offs;
	// Bartershop Bachnt
	if (nd->subtype == NPCTYPE_EXBARTERSHOP) {
		amount = 1;
		offs = 1;
	}
	// generate new shop item list
	RECREATE(nd->u.shop.shop_item, struct npc_item_list, amount);
	for (n = 0, i = 3; n < amount; n++, i+=offs) {
		// Bartershop Bachnt
#if PACKETVER >= 20131223
		switch(nd->subtype) {
			case NPCTYPE_MARKETSHOP:		
		nd->u.shop.shop_item[n].nameid = script_getnum(st,i);
		nd->u.shop.shop_item[n].value = script_getnum(st,i+1);
			nd->u.shop.shop_item[n].qty = script_getnum(st,i+2);
			nd->u.shop.shop_item[n].flag = 1;
			npc_market_tosql(nd->exname, &nd->u.shop.shop_item[n]);
			// Bartershop Bachnt
				break;
			case NPCTYPE_BARTERSHOP:
				if (sscanf(script_getstr(st,i),"%d:%d",&nd->u.shop.shop_item[n].nameid,&nd->u.shop.shop_item[n].qty) != 2){
					ShowError("[SCRIPT] Error syntax script [npcshopitem] NPCTYPE_BARTERSHOP\n*\t\t%s\n",script_getstr(st,i));
					script_pushint(st,0);
					return SCRIPT_CMD_FAILURE;
				} 
				if (sscanf(script_getstr(st,i+1),"%d:%d",&nd->u.shop.shop_item[n].nameidtrade,&nd->u.shop.shop_item[n].value2) != 2){
					ShowError("[SCRIPT] Error syntax script [npcshopitem] NPCTYPE_BARTERSHOP\n*\t\t%s\n",script_getstr(st,i+1));
					script_pushint(st,0);
					return SCRIPT_CMD_FAILURE;
				} 
				break;
			case NPCTYPE_EXBARTERSHOP:
				if(sscanf(script_getstr(st,i), "%d:%d:%d", &nd->u.shop.shop_item[n].nameid,&nd->u.shop.shop_item[n].qty,&nd->u.shop.shop_item[n].value) != 3) {
					ShowError("[SCRIPT] Error syntax script [npcshopitem] NPCTYPE_EXBARTERSHOP\n*\t\t%s\n",script_getstr(st,i));
					script_pushint(st,0);
					return SCRIPT_CMD_FAILURE;
				}				
				
				currencycount = (script_lastdata(st)-3)/offs;
				nd->u.shop.shop_item[n].value2 = currencycount;
				nd->u.shop.shop_item[n].currency = new struct npc_barter_currency[currencycount];
				for(int c = 0; c < currencycount; c++){	
					
					if(sscanf(script_getstr(st,i+1+c), "%d:%d:%d", &nd->u.shop.shop_item[n].currency[c].nameid,&nd->u.shop.shop_item[n].currency[c].refine,&nd->u.shop.shop_item[n].currency[c].amount) != 3) {
						ShowError("[SCRIPT] Error syntax script [npcshopitem] NPCTYPE_EXBARTERSHOP\n*\t\t%s\n",script_getstr(st,i));
						script_pushint(st,0);
						return SCRIPT_CMD_FAILURE;
					}
				}
				//offs = currencycount + 1;
				nd->u.shop.totalcurrency = currencycount;
				break;
/*				// get the currency count of new entries
				int currencycount = (script_lastdata(st)-5)/offs;
				nd->u.shop.shop_item[n].value2 = currencycount;
				
				//ShowInfo("========================================\n");
				//ShowInfo("Barter Add Currency count: %u\n",currencycount);
				nd->u.shop.shop_item[n].nameid = script_getnum(st,3);
				nd->u.shop.shop_item[n].qty = script_getnum(st,4);
				nd->u.shop.shop_item[n].value = script_getnum(st,5);
				nd->u.shop.shop_item[n].value2 = currencycount;
				nd->u.shop.shop_item[n].currency = new struct npc_barter_currency[currencycount];
				for(int c = 0; c < currencycount; c++){	
					//ShowInfo("Barter Add Currency Info: %u\t%u\t%u\n",script_getnum(st,6+n*offs+0),script_getnum(st,6+n*offs+1),script_getnum(st,6+n*offs+2));
					nd->u.shop.shop_item[n].currency[c].nameid = script_getnum(st,6+n*offs+0);
					nd->u.shop.shop_item[n].currency[c].refine = script_getnum(st,6+n*offs+1);
					nd->u.shop.shop_item[n].currency[c].amount = script_getnum(st,6+n*offs+2);
				}
				nd->u.shop.totalcurrency = currencycount;
				break;
*/				
			default :
				nd->u.shop.shop_item[n].nameid = script_getnum(st,i);
				nd->u.shop.shop_item[n].value = script_getnum(st,i+1);
				break;			
		}
#endif
	}
	nd->u.shop.count = n;

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(npcshopadditem)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc_name2id(npcname);
	int n, i;
	uint16 offs = 2, amount;

	if (!nd || ( nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP && nd->subtype != NPCTYPE_POINTSHOP && nd->subtype != NPCTYPE_MARKETSHOP)) { // Not found.
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if (nd->subtype == NPCTYPE_MARKETSHOP)
		offs = 3;

	// get the count of new entries
	amount = (script_lastdata(st)-2)/offs;

#if PACKETVER >= 20131223
	if (nd->subtype == NPCTYPE_MARKETSHOP) {
		for (n = 0, i = 3; n < amount; n++, i += offs) {
			t_itemid nameid = script_getnum(st,i);
			uint16 j;

			// Check existing entries
			ARR_FIND(0, nd->u.shop.count, j, nd->u.shop.shop_item[j].nameid == nameid);
			if (j == nd->u.shop.count) {
				RECREATE(nd->u.shop.shop_item, struct npc_item_list, nd->u.shop.count+1);
				j = nd->u.shop.count;
				nd->u.shop.shop_item[j].flag = 1;
				nd->u.shop.count++;
			}

			nd->u.shop.shop_item[j].nameid = nameid;
			nd->u.shop.shop_item[j].value = script_getnum(st,i+1);
			nd->u.shop.shop_item[j].qty = script_getnum(st,i+2);

			npc_market_tosql(nd->exname, &nd->u.shop.shop_item[j]);
		}
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}
#endif

	// append new items to existing shop item list
	RECREATE(nd->u.shop.shop_item, struct npc_item_list, nd->u.shop.count+amount);
	for (n = nd->u.shop.count, i = 3; n < nd->u.shop.count+amount; n++, i+=offs)
	{
		nd->u.shop.shop_item[n].nameid = script_getnum(st,i);
		nd->u.shop.shop_item[n].value = script_getnum(st,i+1);
	}
	nd->u.shop.count = n;

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

////////////////////////////
/// Barter Shop Add Item BACHNT
/// Add 1 item to Barter and ExBarter shop
/// bartershopadditem "<name>",<item id>,<qty>,<item cost id>,<item cost amount>
///
/// bartershopadditem "<name>",<item id>,<qty>,<zeny>,<item cost[1] id>,<item cost[1] refine>,<item cost[1] amount>,{<item cost[2] id>,<item cost[2] refine>,<item cost[2] amount>...}
////////////////////////////
BUILDIN_FUNC(bartershopadditem)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc_name2id(npcname);
	int c , i = 3;
	uint16 offs = 4, currencycount;

	if (!nd || ( nd->subtype != NPCTYPE_BARTERSHOP && nd->subtype != NPCTYPE_EXBARTERSHOP )) { // Not found.
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}
	
	// append new items to existing shop item list
	RECREATE(nd->u.shop.shop_item, struct npc_item_list, nd->u.shop.count+1);
	
	switch(nd->subtype){
		case NPCTYPE_BARTERSHOP:
			if (sscanf(script_getstr(st,i),"%d:%d",&nd->u.shop.shop_item[c].nameid,&nd->u.shop.shop_item[c].qty) != 2){
				ShowError("[SCRIPT] Error syntax script [npcshopitem] NPCTYPE_BARTERSHOP\n*\t\t%s\n",script_getstr(st,i));
				script_pushint(st,0);
				return SCRIPT_CMD_FAILURE;
			} 
			if (sscanf(script_getstr(st,i+1),"%d:%d",&nd->u.shop.shop_item[c].nameidtrade,&nd->u.shop.shop_item[c].value2) != 2){
				ShowError("[SCRIPT] Error syntax script [npcshopitem] NPCTYPE_BARTERSHOP\n*\t\t%s\n",script_getstr(st,i+1));
				script_pushint(st,0);
				return SCRIPT_CMD_FAILURE;
			} 
			break;
		case NPCTYPE_EXBARTERSHOP:
			offs = 1;
			c = nd->u.shop.count;
			t_itemid nameid;
			int qty;
			int value;
			
			t_itemid nameid2;
			int refine;
			int amount;
			if(sscanf(script_getstr(st,i), "%d:%d:%d", &nameid,&qty,&value) != 3) {
				ShowError("[SCRIPT] Error syntax script [npcshopitem] NPCTYPE_EXBARTERSHOP\n*\t\t%s\n",script_getstr(st,i));
				script_pushint(st,0);
				return SCRIPT_CMD_FAILURE;
			}				
			nd->u.shop.shop_item[c].nameid = nameid;
			nd->u.shop.shop_item[c].qty = qty;
			nd->u.shop.shop_item[c].value = value;
			currencycount = (script_lastdata(st)-3)/offs;
			nd->u.shop.shop_item[c].value2 = currencycount;
			nd->u.shop.shop_item[c].currency = new struct npc_barter_currency[currencycount];
			for(int d = 0; d < currencycount; d++){	
				
				if(sscanf(script_getstr(st,i+1+d), "%d:%d:%d", &nameid2,&refine,&amount) != 3) {
					ShowError("[SCRIPT] Error syntax script [npcshopitem] NPCTYPE_EXBARTERSHOP\n*\t\t%s\n",script_getstr(st,i));
					script_pushint(st,0);
					return SCRIPT_CMD_FAILURE;
				}
				nd->u.shop.shop_item[c].currency[d].nameid = nameid2;
				nd->u.shop.shop_item[c].currency[d].refine = refine;
				nd->u.shop.shop_item[c].currency[d].amount = amount;
			}
			//offs = currencycount + 1;
			nd->u.shop.totalcurrency += currencycount;
			break;
	}
	
	//nd->u.shop.shop_item[n].nameid = script_getnum(st,i);
	//nd->u.shop.shop_item[n].value = script_getnum(st,i+1);
	
	nd->u.shop.count++;

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

//// *npcshopdelitem "<name>",<item id>{,<item id>{,<item id>{,...}}};
BUILDIN_FUNC(npcshopdelitem)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc_name2id(npcname);
	int n, i, size;
	unsigned short amount;
	// Bartershop Bachnt
	int currencyremove = 0; 

	if (!nd || ( nd->subtype != NPCTYPE_SHOP && 
				nd->subtype != NPCTYPE_CASHSHOP && 
				nd->subtype != NPCTYPE_ITEMSHOP && 
				nd->subtype != NPCTYPE_POINTSHOP && 
				nd->subtype != NPCTYPE_MARKETSHOP && 
				nd->subtype != NPCTYPE_BARTERSHOP && 
				nd->subtype != NPCTYPE_EXBARTERSHOP )) { // Not found.
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	amount = script_lastdata(st)-2;
	size = nd->u.shop.count;

	// remove specified items from the shop item list
	for( i = 3; i < 3 + amount; i++ ) {
		t_itemid nameid = script_getnum(st,i);

		ARR_FIND( 0, size, n, nd->u.shop.shop_item[n].nameid == nameid );
		if( n < size ) {
			// Bartershop Bachnt
			if (nd->subtype == NPCTYPE_EXBARTERSHOP)
				currencyremove += nd->u.shop.shop_item[n].value2;
						
			if (n+1 != size)
				memmove(&nd->u.shop.shop_item[n], &nd->u.shop.shop_item[n+1], sizeof(nd->u.shop.shop_item[0])*(size-n));
#if PACKETVER >= 20131223
			if (nd->subtype == NPCTYPE_MARKETSHOP)
				npc_market_delfromsql_(nd->exname, nameid, false);
#endif
			size--;
		}
	}

	RECREATE(nd->u.shop.shop_item, struct npc_item_list, size);
	nd->u.shop.count = size;
	// Bartershop Bachnt
	if(nd->subtype == NPCTYPE_EXBARTERSHOP) {
		nd->u.shop.totalcurrency -= currencyremove;
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Sets a script to attach to a shop npc.
 * npcshopattach "<npc_name>";
 **/
BUILDIN_FUNC(npcshopattach)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc_name2id(npcname);
	int flag = 1;

	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);
// Bartershop Bachnt
	if (!nd || ( nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_CASHSHOP && 
				nd->subtype != NPCTYPE_ITEMSHOP && nd->subtype != NPCTYPE_POINTSHOP && 
				nd->subtype != NPCTYPE_MARKETSHOP && 
				nd->subtype != NPCTYPE_BARTERSHOP && nd->subtype != NPCTYPE_EXBARTERSHOP)) { // Not found.
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if (flag)
		nd->master_nd = ((struct npc_data *)map_id2bl(st->oid));
	else
		nd->master_nd = NULL;

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}




