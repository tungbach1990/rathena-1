/*
		case NPCTYPE_POINTSHOP:
			{
				char output[CHAT_SIZE_MAX];

				memset(output, '\0', sizeof(output));

				if (cost[1] < points || cost[0] < (price - points)) {
					sprintf(output, msg_txt(sd, 713), nd->u.shop.pointshop_str); // You do not have enough '%s'.
					clif_messagecolor(&sd->bl, color_table[COLOR_RED], output, false, SELF);
					return ERROR_TYPE_PURCHASE_FAIL;
				}
				pc_setreg2(sd, nd->u.shop.pointshop_str, cost[0] - (price - points));
				sprintf(output, msg_txt(sd, 716), nd->u.shop.pointshop_str, cost[0] - (price - points)); // Your '%s' is now: %d
				clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
			}
			break;
	}
	return ERROR_TYPE_NONE;
}
*/

/** Payment Process for NPCTYPE_BARTERSHOP and NPCTYPE_EXBARTERSHOP
 * @return e_CASHSHOP_ACK BACHNT
 **/
static enum e_CASHSHOP_ACK npc_barter_process_payment(struct npc_data *nd, int count, struct s_barter_buy_list *item_list, struct map_session_data *sd) {

	int i, n, j, amount, amountdel, new_, w, vt,amountneed, refineneed, delete_amount, z;
	t_itemid nameid,nameidtrade;
	struct item_data *id = NULL;
	struct item *it;
	//ShowWarning("Outside for: count:%u\n",count);
	for( n = 0; n < count; n++ ) {
		nameid = item_list[n].itemId;
		amount = item_list[n].amount;
		
		id = itemdb_exists(nameid);
		if (!id) { // Item Data is checked at script parsing but in case of item_db reload, check again.
			ShowWarning("Failed to find sellitem %u for itemshop NPC '%s' (%s, %d, %d)!\n", nd->u.shop.itemshop_nameid, nd->exname, map_mapid2mapname(nd->bl.m), nd->bl.x, nd->bl.y);
			return ERROR_TYPE_PURCHASE_FAIL;
		}
		ARR_FIND(0,nd->u.shop.count,j,nd->u.shop.shop_item[j].nameid == nameid || itemdb_viewid(nd->u.shop.shop_item[j].nameid) == nameid);
		//ShowWarning("Outside switch: \n");
		switch(nd->subtype) {
			//////////////////////////////
			/// Barter Shop
			//////////////////////////////
			case NPCTYPE_EXBARTERSHOP:
				z = (double)nd->u.shop.shop_item[j].value*amount;
				if( pc_payzeny(sd, (int)z, LOG_TYPE_NPC, NULL) )
					return ERROR_TYPE_MONEY;
				nd->u.shop.shop_item[j].qty -= amount;
				//ShowWarning("item trong shop %u \n",nd->u.shop.shop_item[j].value2);
				//ShowWarning("item trong shop %u \n",nd->u.shop.shop_item[j].qty);
				//ShowWarning("Currency Count: %u\n",amount);
				for(int c = 0; c < nd->u.shop.shop_item[j].value2; c++){
					nameidtrade = nd->u.shop.shop_item[j].currency[c].nameid;
					amountneed = nd->u.shop.shop_item[j].currency[c].amount;
					refineneed = nd->u.shop.shop_item[j].currency[c].refine;					
					delete_amount = amountneed * amount;
					for (i = 0; i < MAX_INVENTORY && delete_amount > 0; i++) {
						//ShowWarning("\nStart loop: i=%u\n",i);
						amountdel = 0;

						if (sd->inventory.u.items_inventory[i].nameid == 0 
										|| sd->inventory_data[i] == NULL 
										|| !(it = &sd->inventory.u.items_inventory[i]) 
										|| it->nameid != nameidtrade 
										|| it->refine != refineneed
										)
							continue;
						//ShowWarning("\nPass phase 1 loop: i=%u\n",i);
						if (!pc_can_sell_item(sd, it, nd->subtype))
							continue;
						//ShowWarning("\nPass phase 2 loop: i=%u\n",i);
						amountdel = it->amount;
						if (amountdel > delete_amount)
							amountdel = delete_amount;
						
						if (pc_delitem(sd, i, amountdel, 0, 0, LOG_TYPE_NPC)) {
							ShowWarning("Failed to delete item %u from '%s' at itemshop NPC '%s' (%s, %d, %d)!\n", nd->u.shop.itemshop_nameid, sd->status.name, nd->exname, map_mapid2mapname(nd->bl.m), nd->bl.x, nd->bl.y);
							return ERROR_TYPE_PURCHASE_FAIL;
						}
						delete_amount -= amountdel;
					}
					if (delete_amount > 0) {
						ShowError("Item %u is not enough as payment at itemshop NPC '%s' (%s, %d, %d, AID=%d, CID=%d)!\n", nd->u.shop.itemshop_nameid, nd->exname, map_mapid2mapname(nd->bl.m), nd->bl.x, nd->bl.y, sd->status.account_id, sd->status.char_id);
						return ERROR_TYPE_PURCHASE_FAIL;
					}
				}
				break;
			case NPCTYPE_BARTERSHOP:
				nameidtrade = nd->u.shop.shop_item[j].nameidtrade;
				amountneed = nd->u.shop.shop_item[j].value2;
				//ShowWarning("\nnpc_barter_process_payment j: %u %u %u %u %u\n",j,nameid,amount,nameidtrade,amountneed);
				delete_amount = amountneed * amount;
				//ShowWarning("\ndelete_amount: %u\n",delete_amount);
				for (i = 0; i < MAX_INVENTORY && delete_amount > 0; i++) {
					//ShowWarning("\nStart loop: i=%u\n",i);
					amountdel = 0;

					if (sd->inventory.u.items_inventory[i].nameid == 0 
									|| sd->inventory_data[i] == NULL 
									|| !(it = &sd->inventory.u.items_inventory[i]) 
									|| it->nameid != nameidtrade
									)
						continue;
					//ShowWarning("\nPass phase 1 loop: i=%u\n",i);
					if (!pc_can_sell_item(sd, it, nd->subtype))
						continue;
					//ShowWarning("\nPass phase 2 loop: i=%u\n",i);
					amountdel = it->amount;
					if (amountdel > delete_amount)
						amountdel = delete_amount;
					
					if (pc_delitem(sd, i, amountdel, 0, 0, LOG_TYPE_NPC)) {
						ShowWarning("Failed to delete item %u from '%s' at itemshop NPC '%s' (%s, %d, %d)!\n", nd->u.shop.itemshop_nameid, sd->status.name, nd->exname, map_mapid2mapname(nd->bl.m), nd->bl.x, nd->bl.y);
						return ERROR_TYPE_PURCHASE_FAIL;
					}
					delete_amount -= amountdel;
				}
				if (delete_amount > 0) {
					ShowError("Item %u is not enough as payment at itemshop NPC '%s' (%s, %d, %d, AID=%d, CID=%d)!\n", nd->u.shop.itemshop_nameid, nd->exname, map_mapid2mapname(nd->bl.m), nd->bl.x, nd->bl.y, sd->status.account_id, sd->status.char_id);
					return ERROR_TYPE_PURCHASE_FAIL;
				}				
				break;
			//////////////////////////////
		}
	}
	return ERROR_TYPE_NONE;
}


////////////////////
/// Barter Shop
/// Processes incoming npc barter purchase list
////////////////////
int npc_barter_buylist(struct map_session_data *sd, int count, struct s_barter_buy_list *item_list)
{
	struct npc_data *nd = (struct npc_data *)map_id2bl(sd->npc_shopid);
	enum e_CASHSHOP_ACK res;
	struct npc_item_list *shop = NULL;
	int i, j, amount, new_, w, vt;
	t_itemid nameid;
	unsigned short shop_size = 0;
	item_data *id;
	int points = 0;
	
	nullpo_retr(1, sd);
	nullpo_retr(1, item_list);
		
	//ShowWarning("\nsd->npc_shopid: %u - type: %u\n",sd->npc_shopid, nd->subtype);
	//if (!nd->u.scr.shop) ShowWarning("\nu.scr.shop: fail\n");
	if (nd == NULL || nd->subtype != NPCTYPE_BARTERSHOP ||	count == 0 || !nd->u.shop.shop_item)
		return 11;

	shop = nd->u.shop.shop_item;
	shop_size = nd->u.shop.count;

	w = 0;
	new_ = 0;

	int items[MAX_INVENTORY] = { 0 };
	
	// Validating Process
	//ShowWarning("\nStart Validating\n");
	//ShowWarning("\ncount in npc_barter_buylist: %u",count);
	for( i = 0; i < count; i++ )
	{
		nameid = item_list[i].itemId;
		amount = item_list[i].amount;
		id = itemdb_exists(nameid);
		//ShowWarning("\nInfo: %u %u\n",nameid,amount);

		if( !id || amount <= 0 )
			return ERROR_TYPE_ITEM_ID;

		ARR_FIND(0,nd->u.shop.count,j,nd->u.shop.shop_item[j].nameid == nameid || itemdb_viewid(nd->u.shop.shop_item[j].nameid) == nameid);
		/// Barter Shop
		if( j == nd->u.shop.count || nd->u.shop.shop_item[j].value2 <= 0 )
			return ERROR_TYPE_ITEM_ID;
		//ShowWarning("\nInside For loop 2\n");
		nameid = item_list[i].itemId = nd->u.shop.shop_item[j].nameid; //item_avail replacement
		//ShowWarning("\nInside For loop 3\n");
		if( !itemdb_isstackable2(id) && amount > 1 )
		{
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable item %u!\n", sd->status.name, sd->status.account_id, sd->status.char_id, amount, nameid);
			amount = item_list[i].amount = 1;
		}
		//ShowWarning("\nInside For loop\n");
		if( nd->master_nd ) { // Script-controlled shops decide by themselves, what can be bought and for what price.
			continue;
		}

		switch( pc_checkadditem(sd,nameid,amount) )
		{
			case CHKADDITEM_NEW:
				new_ += id->inventorySlotNeeded(amount);
				break;
			case CHKADDITEM_OVERAMOUNT:
				return ERROR_TYPE_INVENTORY_WEIGHT;
		}

		vt += nd->u.shop.shop_item[j].value2 * amount;
		w += itemdb_weight(nameid) * amount;
	}
	//ShowWarning("\nLeave For loop\n");
	if (nd->master_nd) //Script-based shops.
		return npc_barter_buylist_sub(sd,count,(struct s_barter_buy_list*)item_list,nd->master_nd,nd);

	if( w + sd->weight > sd->max_weight )
		return ERROR_TYPE_INVENTORY_WEIGHT;
	if( pc_inventoryblank(sd) < new_ )
		return ERROR_TYPE_INVENTORY_WEIGHT;

	if ((res = npc_barter_process_payment(nd, count, item_list, sd)) != ERROR_TYPE_NONE)
		return res;

	//ShowWarning("\nStart Delivery Process\n");
	
	// Delivery Process ----------------------------------------------------
	for( i = 0; i < count; i++ ) {
		nameid = item_list[i].itemId;
		amount = item_list[i].amount;

		if( !pet_create_egg(sd,nameid) ) {
			struct item item_tmp;
			unsigned short get_amt = amount;

			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = nameid;
			item_tmp.identify = 1;

			if ((itemdb_search(nameid))->flag.guid)
				get_amt = 1;

			for (j = 0; j < amount; j += get_amt)
				pc_additem(sd,&item_tmp,get_amt,LOG_TYPE_NPC);
		}
	}

	return ERROR_TYPE_NONE;
}

int npc_barter_ex_buylist(struct map_session_data *sd, int count, struct s_barter_buy_list *item_list)
{
	struct npc_data *nd = (struct npc_data *)map_id2bl(sd->npc_shopid);
	enum e_CASHSHOP_ACK res;
	struct npc_item_list *shop = NULL;
	struct npc_barter_currency *currency = NULL;
	int i, j, amount, new_, w, vt;
	t_itemid nameid;
	unsigned short shop_size = 0;
	item_data *id;
	int points = 0;
	
	nullpo_retr(1, sd);
	nullpo_retr(1, item_list);
		
	//ShowWarning("\nsd->npc_shopid: %u - type: %u\n",sd->npc_shopid, nd->subtype);
	//if (!nd->u.scr.shop) ShowWarning("\nu.scr.shop: fail\n");
	if (nd == NULL || nd->subtype != NPCTYPE_EXBARTERSHOP ||	count == 0 || !nd->u.shop.shop_item)
		return 11;

	shop = nd->u.shop.shop_item;
	shop_size = nd->u.shop.count;

	w = 0;
	new_ = 0;

	int items[MAX_INVENTORY] = { 0 };
	
	// Validating Process
	//ShowWarning("Start Ex Barter Validating\n");
	//ShowWarning("\ncount in npc_barter_buylist: %u",count);
	for( i = 0; i < count; i++ )
	{
		nameid = item_list[i].itemId;
		amount = item_list[i].amount;
		id = itemdb_exists(nameid);
		//ShowWarning("Info: %u %u\n",nameid,amount);

		if( !id || amount <= 0 )
			return ERROR_TYPE_ITEM_ID;

		ARR_FIND(0,nd->u.shop.count,j,nd->u.shop.shop_item[j].nameid == nameid || itemdb_viewid(nd->u.shop.shop_item[j].nameid) == nameid);
		/// Barter Shop
		//ShowWarning("Inside For loop 1\n");
		//ShowWarning("Inside For loop 1\tcount: %u / %u\tcur_count: %u\n",j,nd->u.shop.count,nd->u.shop.shop_item[j].value2);
		if( j == nd->u.shop.count || nd->u.shop.shop_item[j].value2 <= 0 )
			return ERROR_TYPE_ITEM_ID;
		//ShowWarning("Inside For loop 2\n");
		nameid = item_list[i].itemId = nd->u.shop.shop_item[j].nameid; //item_avail replacement
		//ShowWarning("Inside For loop 3\n");
		if( !itemdb_isstackable2(id) && amount > 1 )
		{
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable item %u!\n", sd->status.name, sd->status.account_id, sd->status.char_id, amount, nameid);
			amount = item_list[i].amount = 1;
		}
		//ShowWarning("Inside For loop\n");
		if( nd->master_nd ) { // Script-controlled shops decide by themselves, what can be bought and for what price.
			continue;
		}

		switch( pc_checkadditem(sd,nameid,amount) )
		{
			case CHKADDITEM_NEW:
				new_ += id->inventorySlotNeeded(amount);
				break;
			case CHKADDITEM_OVERAMOUNT:
				return ERROR_TYPE_INVENTORY_WEIGHT;
		}

		vt += nd->u.shop.shop_item[j].value2 * amount;
		w += itemdb_weight(nameid) * amount;
	}
	//ShowWarning("Leave For loop\n");
	if (nd->master_nd) //Script-based shops.
		return npc_barter_buylist_sub(sd,count,(struct s_barter_buy_list*)item_list,nd->master_nd,nd);

	if( w + sd->weight > sd->max_weight )
		return ERROR_TYPE_INVENTORY_WEIGHT;
	if( pc_inventoryblank(sd) < new_ )
		return ERROR_TYPE_INVENTORY_WEIGHT;

	if ((res = npc_barter_process_payment(nd, count, item_list, sd)) != ERROR_TYPE_NONE) {
		ShowWarning("Failed Barter Buy Here");
		return res;
	}

	//ShowWarning("Start Delivery Process\n");
	
	// Delivery Process ----------------------------------------------------
	for( i = 0; i < count; i++ ) {
		nameid = item_list[i].itemId;
		amount = item_list[i].amount;

		if( !pet_create_egg(sd,nameid) ) {
			struct item item_tmp;
			unsigned short get_amt = amount;

			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = nameid;
			item_tmp.identify = 1;

			if ((itemdb_search(nameid))->flag.guid)
				get_amt = 1;

			for (j = 0; j < amount; j += get_amt)
				pc_additem(sd,&item_tmp,get_amt,LOG_TYPE_NPC);
		}
	}

	return ERROR_TYPE_NONE;
}

