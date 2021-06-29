/*
void clif_parse_Auction_bid(int fd, struct map_session_data *sd){
	struct s_packet_db* info = &packet_db[RFIFOW(fd,0)];
	unsigned int auction_id = RFIFOL(fd,info->pos[0]);
	int bid = RFIFOL(fd,info->pos[1]);

	if( !pc_can_give_items(sd) ) { //They aren't supposed to give zeny [Inkfish]
		clif_displaymessage(sd->fd, msg_txt(sd,246));
		return;
	}

	if( bid <= 0 )
		clif_Auction_message(fd, 0); // You have failed to bid into the auction
	else if( bid > sd->status.zeny )
		clif_Auction_message(fd, 8); // You do not have enough zeny
	else if ( CheckForCharServer() ) // char server is down (bugreport:1138)
		clif_Auction_message(fd, 0); // You have failed to bid into the auction
	else {
		pc_payzeny(sd, bid, LOG_TYPE_AUCTION, NULL);
		intif_Auction_bid(sd->status.char_id, sd->status.name, auction_id, bid);
	}
}

*/


//////////////////////
/// Barter Shop
//////////////////////

void clif_npc_barter_open(struct map_session_data *sd, struct npc_data *nd)
{
#if PACKETVER_MAIN_NUM >= 20181226 || PACKETVER_RE_NUM >= 20181226 || PACKETVER_ZERO_NUM >= 20181226
	nullpo_retv(sd);
	nullpo_retv(nd);

	int fd = sd->fd;

	if( !session_isActive( fd ) ){
		return;
	}

	sd->npc_shopid = nd->bl.id;

	uint16 len = sizeof( struct PACKET_ZC_NPC_BARTER_OPEN ) + nd->u.shop.count * sizeof( struct PACKET_ZC_NPC_BARTER_OPEN_sub );
	WFIFOHEAD( fd, len );
	struct PACKET_ZC_NPC_BARTER_OPEN* p = (struct PACKET_ZC_NPC_BARTER_OPEN *)WFIFOP( fd, 0 );

	p->packetType = HEADER_ZC_NPC_BARTER_OPEN;
	p->packetLength = len;
	
	int c = 0;
	
	for( int i = 0; i < nd->u.shop.count; i++ ) {
		struct item_data* id = itemdb_search( nd->u.shop.shop_item[i].nameid );

		if (id == NULL)
			continue;
		
		p->list[c].nameid = nd->u.shop.shop_item[i].nameid;
		p->list[c].type = id->type;
		p->list[c].amount = nd->u.shop.shop_item[i].qty;
		p->list[c].currencyNameid = nd->u.shop.shop_item[i].nameidtrade;
		p->list[c].currencyAmount = nd->u.shop.shop_item[i].value2;
		p->list[c].weight = id->weight * 10;
		p->list[c].index = i;
		c++;
	}

	WFIFOSET( fd, len );
	
	sd->state.trading = 1;
	
#endif
}

void clif_parse_NPCBarterPurchase(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
void clif_parse_NPCBarterPurchase(int fd, struct map_session_data *sd)
{
#if PACKETVER_MAIN_NUM >= 20190116 || PACKETVER_RE_NUM >= 20190116 || PACKETVER_ZERO_NUM >= 20181226
	nullpo_retv( sd );
	const struct PACKET_CZ_NPC_BARTER_PURCHASE *p = (struct PACKET_CZ_NPC_BARTER_PURCHASE *)RFIFOP( fd, 0 );
	int count = (p->packetLength - sizeof(struct PACKET_CZ_NPC_BARTER_PURCHASE)) / sizeof(struct PACKET_CZ_NPC_BARTER_PURCHASE_sub);
	struct s_barter_buy_list *item_list;
	CREATE(item_list,struct s_barter_buy_list, count);

	for (int i = 0; i < count; i++) {
		item_list[i].itemId = p->list[i].itemId;
		item_list[i].amount = p->list[i].amount;
		item_list[i].invIndex = p->list[i].invIndex;
		item_list[i].shopIndex = p->list[i].shopIndex;
	}
	uint8 response = npc_barter_buylist(sd, count, (struct s_barter_buy_list*)item_list);
	
	clif_npc_buy_result(sd, response);
	aFree(item_list);
#endif
}

void clif_parse_npc_expanded_barter_closed(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
void clif_parse_npc_expanded_barter_closed(int fd, struct map_session_data *sd)
{
	nullpo_retv(sd);
	sd->npc_shopid = 0;
	sd->state.trading = 0;
}

void clif_parse_npc_barter_closed(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
void clif_parse_npc_barter_closed(int fd, struct map_session_data *sd)
{
	nullpo_retv(sd);
	sd->npc_shopid = 0;
	sd->state.trading = 0;
}

#if PACKETVER_MAIN_NUM >= 20191120 || PACKETVER_RE_NUM >= 20191106 || PACKETVER_ZERO_NUM >= 20191127
#define NEXT_EXPANDED_BARTER_ITEM(var, count) \
	var = (struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub *)((char*)var + \
		sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub) - \
		sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2) + \
		count * sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2))
#endif


void clif_npc_expanded_barter_open(struct map_session_data *sd, struct npc_data *nd)
{
#if PACKETVER_MAIN_NUM >= 20181226 || PACKETVER_RE_NUM >= 20181226 || PACKETVER_ZERO_NUM >= 20181226
	nullpo_retv(sd);
	nullpo_retv(nd);

	int fd = sd->fd;

	if( !session_isActive( fd ) ){
		return;
	}

	sd->npc_shopid = nd->bl.id;
	
	int c = 0, l = 0, len = 0;
	
	//len = sizeof( struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN );
	
	len = sizeof( struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN );
	len += nd->u.shop.count * sizeof( struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub );
	len += nd->u.shop.totalcurrency * sizeof( struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2 );
	
	//ShowWarning("Total Currency: %u --- Total Items: %u\n",nd->u.shop.totalcurrency,nd->u.shop.count);
/*
	for(int i = 0; i < nd->u.shop.count; i++){
		struct item_data* id = itemdb_search( nd->u.shop.shop_item[i].nameid );
		if (id == NULL) 
			continue;
		for(int j = 0; j < nd->u.shop.shop_item[i].value2; j++){
			struct item_data* id2 = itemdb_search( nd->u.shop.shop_item[i].currency[j].nameid );
			if (id2 == NULL) 
				continue;
			len += sizeof( struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2 );
		}
		c++;
		len += sizeof( PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub );
	}
*/	
	WFIFOHEAD( fd, len );
	struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN* p = (struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN *)WFIFOP( fd, 0 );

	p->packetType = HEADER_ZC_NPC_EXPANDED_BARTER_OPEN;
	p->packetLength = len;
	p->items_count = nd->u.shop.count;

	struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub *item = &p->items[0];	
	
	//c = 0;
	for(int i = 0; i < nd->u.shop.count; i++){
		struct item_data* id = itemdb_search( nd->u.shop.shop_item[i].nameid );
		
		if (id == NULL) 
			continue;
		
		//ShowWarning("___________________\n");
		//ShowWarning("Item %u:\t%u\t%u\t%u\t\tMemLoc: %X\n",i,nd->u.shop.shop_item[i].nameid,nd->u.shop.shop_item[i].qty,nd->u.shop.shop_item[i].value,item);
		
		item->nameid = nd->u.shop.shop_item[i].nameid;
		item->type = id->type;
		item->amount = nd->u.shop.shop_item[i].qty;
		item->weight = id->weight * 10;
		item->index = i;
		item->zeny = nd->u.shop.shop_item[i].value;
		
		l = 0;
		for(int j = 0; j < nd->u.shop.shop_item[i].value2; j++){
			struct item_data* id2 = itemdb_search( nd->u.shop.shop_item[i].currency[j].nameid );
			if (id2 == NULL) 
				continue;
			
			//ShowWarning("------------------\n");
			//ShowWarning("Currencies %u:\t%u\t%u\t%u\tMemLoc: %X\n",l,nd->u.shop.shop_item[i].currency[j].nameid,nd->u.shop.shop_item[i].currency[j].refine,nd->u.shop.shop_item[i].currency[j].amount,&item->currencies[l]);
			
			item->currencies[l].nameid = nd->u.shop.shop_item[i].currency[j].nameid;
			item->currencies[l].refine_level = nd->u.shop.shop_item[i].currency[j].refine;
			item->currencies[l].amount = nd->u.shop.shop_item[i].currency[j].amount;
			item->currencies[l].type = id2->type;
			
			l++;		
		}
		item->currency_count = l;
		
		//NEXT_EXPANDED_BARTER_ITEM(item, l);
		
		item = (struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub *)((char*)item + 
					sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub) - 
					sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2) + 
					l * sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2));
				
		//ShowWarning("It_P %u:\t%u\t%u\t%u\n",i,item->nameid,item->amount,item->zeny);
		//c++;
	}
	
	WFIFOSET( fd, len );
	
	sd->state.trading = 1;
#endif
}
/*
/////// Herc style
void clif_npc_expanded_barter_open(struct map_session_data *sd, struct npc_data *nd)
{
	ShowWarning("=============================\n");
	ShowWarning("NPC Name: %s\n",nd->name);
#if PACKETVER_MAIN_NUM >= 20191120 || PACKETVER_RE_NUM >= 20191106 || PACKETVER_ZERO_NUM >= 20191127
	nullpo_retv(sd);
	nullpo_retv(nd);
	
	int fd = sd->fd;
	char packet_buf[0xffff];
	if( !session_isActive( fd ) ){
		return;
	}

	sd->npc_shopid = nd->bl.id;
	
	int len = sizeof( struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN );
	
	struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN* p = (struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN *)&packet_buf[0];
	
	int c = 0, l = 0, totalcurrency = 0;;
	
	struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub *item = &p->items[0];		
	
	for(int i = 0; i < nd->u.shop.count; i++){
		struct item_data* id = itemdb_search( nd->u.shop.shop_item[i].nameid );
		
		if (id == NULL) 
			continue;
		
		ShowWarning("___________________\n");
		ShowWarning("Item %u:\t%u\t%u\t%u\t\tMemLoc: %X\n",i,nd->u.shop.shop_item[i].nameid,nd->u.shop.shop_item[i].qty,nd->u.shop.shop_item[i].value,item);
		
		item->nameid = nd->u.shop.shop_item[i].nameid;
		item->type = id->type;
		item->amount = nd->u.shop.shop_item[i].qty;
		item->weight = id->weight * 10;
		item->index = i;
		item->zeny = nd->u.shop.shop_item[i].value;
		
		l = 0;
		for(int j = 0; j < nd->u.shop.shop_item[i].value2; j++){
			struct item_data* id2 = itemdb_search( nd->u.shop.shop_item[i].currency[j].nameid );
			if (id2 == NULL) 
				continue;
			
			ShowWarning("------------------\n");
			ShowWarning("Currencies %u:\t%u\t%u\t%u\tMemLoc: %X\n",l,nd->u.shop.shop_item[i].currency[j].nameid,nd->u.shop.shop_item[i].currency[j].refine,nd->u.shop.shop_item[i].currency[j].amount,&item->currencies[l]);
			
			item->currencies[l].nameid = nd->u.shop.shop_item[i].currency[j].nameid;
			item->currencies[l].refine_level = nd->u.shop.shop_item[i].currency[j].refine;
			item->currencies[l].amount = nd->u.shop.shop_item[i].currency[j].amount;
			item->currencies[l].type = id2->type;
			
			l++;
			totalcurrency++;
			len += sizeof( struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2 );
			
		}
		item->currency_count = l;
		
		//NEXT_EXPANDED_BARTER_ITEM(item, l);
		
		item = (struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub *)((char*)item + 
					sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub) - 
					sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2) + 
					l * sizeof(struct PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub2))
		
		len += sizeof( PACKET_ZC_NPC_EXPANDED_BARTER_OPEN_sub );
		
		ShowWarning("It_P %u:\t%u\t%u\t%u\n",i,item->nameid,item->amount,item->zeny);
		c++;
	}
	ShowWarning("Total Items : %u/%u --- len: %u\n",nd->u.shop.count,c,len);
	
	p->packetType = HEADER_ZC_NPC_EXPANDED_BARTER_OPEN;
	p->packetLength = len;
	p->items_count = c;
	//ShowWarning("\n%x\n",p);
	ShowWarning("\n%p\n",p);
	clif_send(p, p->packetLength, &sd->bl, SELF);
	
#endif
}
#undef NEXT_EXPANDED_BARTER_ITEM
*/



void clif_parse_npc_expanded_barter_purchase(int fd, struct map_session_data *sd) __attribute__((nonnull (2)));
void clif_parse_npc_expanded_barter_purchase(int fd, struct map_session_data *sd)
{
	nullpo_retv( sd );
	const struct PACKET_CZ_NPC_EXPANDED_BARTER_PURCHASE *p = (struct PACKET_CZ_NPC_EXPANDED_BARTER_PURCHASE *)RFIFOP( fd, 0 );
	int count = (p->packetLength - sizeof(struct PACKET_CZ_NPC_EXPANDED_BARTER_PURCHASE)) / sizeof p->list[0];
	struct s_barter_buy_list *item_list;
	CREATE(item_list,struct s_barter_buy_list, count);
	
	//ShowWarning("============================\n");
	//ShowWarning("Received buy list\tCount: %u\n",count);
	
	for (int i = 0; i < count; i++) {
		item_list[i].itemId = p->list[i].itemId;
		item_list[i].amount = p->list[i].amount;
		item_list[i].invIndex = -1;
		item_list[i].shopIndex = p->list[i].shopIndex;
		//ShowWarning("-------- Item %u\tID: %u\tAmount: %u\n",i,p->list[i].itemId,p->list[i].amount);
	}
	uint8 response = npc_barter_ex_buylist(sd, count, (struct s_barter_buy_list*)item_list);
	if(response == ERROR_TYPE_PURCHASE_FAIL)		
		//clif_messagecolor(fd, "You don't have correct items to trade");
		clif_messagecolor(&sd->bl, color_table[COLOR_RED],"You don't have correct items to trade",false,SELF);
	clif_npc_buy_result(sd, response);
	aFree(item_list);
	
/*
#if PACKETVER_MAIN_NUM >= 20190904 || PACKETVER_RE_NUM >= 20190904 || PACKETVER_ZERO_NUM >= 20190828
	if (sd->state.trading || pc_isdead(sd) || pc_isvending(sd))
		return;

	const struct PACKET_CZ_NPC_EXPANDED_BARTER_PURCHASE *const p = RP2PTR(fd);
	int count = (p->packetLength - sizeof(struct PACKET_CZ_NPC_EXPANDED_BARTER_PURCHASE)) / sizeof p->list[0];
	struct barteritemlist item_list;

	Assert_retv(count >= 0 && count <= sd->status.inventorySize);

	VECTOR_INIT(item_list);
	VECTOR_ENSURE(item_list, count, 1);

	for (int i = 0; i < count; i++) {
		struct barter_itemlist_entry entry = { 0 };
		entry.addId = p->list[i].itemId;
		entry.addAmount = p->list[i].amount;
		entry.removeIndex = -1;
		entry.shopIndex = p->list[i].shopIndex;
		VECTOR_PUSH(item_list, entry);
	}

	int response = npc->expanded_barter_buylist(sd, &item_list);
	clif->npc_buy_result(sd, response);

	VECTOR_CLEAR(item_list);
#endif
*/
}