/*
static int npc_buylist_sub(struct map_session_data* sd, uint16 n, struct s_npc_buy_list *item_list, struct npc_data* nd) {
	char npc_ev[EVENT_NAME_LENGTH];
	int i, key_nameid = 0, key_amount = 0;

	// discard old contents
	script_cleararray_pc( sd, "@bought_nameid" );
	script_cleararray_pc( sd, "@bought_quantity" );

	// save list of bought items
	for (i = 0; i < n; i++) {
		script_setarray_pc( sd, "@bought_nameid", i, item_list[i].nameid, &key_nameid );
		script_setarray_pc( sd, "@bought_quantity", i, item_list[i].qty, &key_amount );
	}

	// invoke event
	snprintf(npc_ev, ARRAYLENGTH(npc_ev), "%s::%s", nd->exname, script_config.onbuy_event_name);
	npc_event(sd, npc_ev, 0);

	return 0;
}
*/
/**
 * Barter Shop Buylist sub BACHNT
 */
static int npc_barter_buylist_sub(struct map_session_data* sd, uint16 n, struct s_barter_buy_list *item_list, struct npc_data* ndmaster, struct npc_data* nd) {
	char npc_ev[EVENT_NAME_LENGTH];
	int i, key_nameid = 0, key_amount = 0, key_cost_nameid = 0, key_cost_amount = 0, key_cost_zeny = 0;
	
	int j;

	// discard old contents
	script_cleararray_pc( sd, "@bought_nameid" );
	script_cleararray_pc( sd, "@bought_quantity" );

	script_cleararray_pc( sd, "@bought_cost_nameid" );
	script_cleararray_pc( sd, "@bought_cost_quantity" );
	script_cleararray_pc( sd, "@bought_cost_zeny" );

	//ShowInfo("Item Bought Count: %u\n",n);
	ShowInfo("NPC Shop Info: %s\tMasterNPC: %s\n",nd->name,ndmaster->name);
	ShowInfo("NPC Shop Count: %u\n",nd->u.shop.count);
	// save list of bought items
	for (i = 0; i < n; i++) {
		//ShowInfo("ItemID: %u\tQty: %u\n",item_list[i].itemId, item_list[i].amount);
		script_setarray_pc( sd, "@bought_nameid", i, item_list[i].itemId, &key_nameid );
		script_setarray_pc( sd, "@bought_quantity", i, item_list[i].amount, &key_amount );
		
		ARR_FIND(0,nd->u.shop.count,j,nd->u.shop.shop_item[j].nameid == item_list[i].itemId || itemdb_viewid(nd->u.shop.shop_item[j].nameid) == item_list[i].itemId);
		ShowInfo("NPC Shop index: %u\n",j);
		switch(nd->subtype){
			case NPCTYPE_BARTERSHOP:
				script_setarray_pc( sd, "@bought_cost_nameid", i, nd->u.shop.shop_item[j].nameidtrade, &key_cost_nameid );
				script_setarray_pc( sd, "@bought_cost_quantity", i, nd->u.shop.shop_item[j].value2, &key_cost_amount );
				break;
			case NPCTYPE_EXBARTERSHOP:
				// Todo: Incomplete
				//script_setarray_pc( sd, "@bought_cost_zeny", i, nd->u.shop.shop_item[j].value, &key_cost_zeny );
				break;
		}
	}

	// invoke event
	snprintf(npc_ev, ARRAYLENGTH(npc_ev), "%s::%s", ndmaster->exname, script_config.onbuy_event_name);
	npc_event(sd, npc_ev, 0);

	return 0;
}
