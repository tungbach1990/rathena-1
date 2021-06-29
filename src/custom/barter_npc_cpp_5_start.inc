


/**
 * Parses a shop/cashshop npc.
 * Line definition :
 * <map name>,<x>,<y>,<facing>%TAB%shop%TAB%<NPC Name>%TAB%<sprite id>,<itemid>:<price>{,<itemid>:<price>...}
 * <map name>,<x>,<y>,<facing>%TAB%cashshop%TAB%<NPC Name>%TAB%<sprite id>,<itemid>:<price>{,<itemid>:<price>...}
 * <map name>,<x>,<y>,<facing>%TAB%itemshop%TAB%<NPC Name>%TAB%<sprite id>,<costitemid>{:<discount>},<itemid>:<price>{,<itemid>:<price>...}
 * <map name>,<x>,<y>,<facing>%TAB%pointshop%TAB%<NPC Name>%TAB%<sprite id>,<costvariable>{:<discount>},<itemid>:<price>{,<itemid>:<price>...}
 * <map name>,<x>,<y>,<facing>%TAB%marketshop%TAB%<NPC Name>%TAB%<sprite id>,<itemid>:<price>:<quantity>{,<itemid>:<price>:<quantity>...}
 * <map name>,<x>,<y>,<facing>%TAB%bartershop%TAB%<NPC Name>%TAB%<sprite id>,<itemid>:<quantity>:<itemidneed>:<quantityneed>{,<itemid>:<quantity>:<itemidneed>:<quantityneed>...}
 * @param w1 : word 1 before tab (<from map name>,<x>,<y>,<facing>)
 * @param w2 : word 2 before tab (shop|cashshop|itemshop|pointshop|marketshop|bartershop|exbartershop), keyword that sent us in this parsing
 * @param w3 : word 3 before tab (<NPC Name>)
 * @param w4 : word 4 before tab (<sprited id>,<shop definition...>)
 * @param start : index to start parsing
 * @param buffer : lines to parses
 * @param filepath : filename with path wich we are parsing
 * @return new index for next parsing
 */
static const char* npc_parse_shop(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath)
{
	char *p, point_str[32];
	int m, is_discount = 0;
	uint16 dir;
	short x, y;
	t_itemid nameid = 0;
	/// Barter Shop [BACHNT]
	char *blist; // Barter BACHNT
	t_itemid nameidtrade = 0;
	int quantity,quantityneed = 0;
	int	refinelevel = 0;
	int currencycount = 0, totalcurrency = 0;
	/// Barter Shop
	struct npc_data *nd;
	enum npc_subtype type;

	if( strcmp(w1,"-") == 0 )
	{// 'floating' shop?
		x = y = dir = 0;
		m = -1;
	}
	else
	{// w1=<map name>,<x>,<y>,<facing>
		char mapname[MAP_NAME_LENGTH_EXT];
		if( sscanf(w1, "%15[^,],%6hd,%6hd,%4hd", mapname, &x, &y, &dir) != 4
		||	strchr(w4, ',') == NULL )
		{
			ShowError("npc_parse_shop: Invalid shop definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
			return strchr(start,'\n');// skip and continue
		}

		m = map_mapname2mapid(mapname);
	}

	struct map_data *mapdata = map_getmapdata(m);

	if( m != -1 && ( x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys ) ) {
		ShowWarning("npc_parse_shop: coordinates %d/%d are out of bounds in map %s(%dx%d), in file '%s', line '%d'\n", x, y, mapdata->name, mapdata->xs, mapdata->ys,filepath,strline(buffer,start-buffer));
	}

	if( !strcasecmp(w2,"cashshop") )
		type = NPCTYPE_CASHSHOP;
	else if( !strcasecmp(w2,"itemshop") )
		type = NPCTYPE_ITEMSHOP;
	else if( !strcasecmp(w2,"pointshop") )
		type = NPCTYPE_POINTSHOP;
	else if( !strcasecmp(w2, "marketshop") )
		type = NPCTYPE_MARKETSHOP;
	// Barter Shop [BACHNT]
	else if( !strcasecmp(w2, "bartershop") )
		type = NPCTYPE_BARTERSHOP;
	else if( !strcasecmp(w2, "exbartershop") )
		type = NPCTYPE_EXBARTERSHOP;
	else
		type = NPCTYPE_SHOP;

	p = strchr(w4,',');
	memset(point_str,'\0',sizeof(point_str));

	switch(type) {
		//////////////////////
		// Barter Shop [BACHNT]
		//////////////////////
		// Scan first value check >> Maybe item cost
		case NPCTYPE_BARTERSHOP: {
			break;
		}			
		case NPCTYPE_EXBARTERSHOP: {
			break;
		}
		case NPCTYPE_ITEMSHOP: {
			if (sscanf(p,",%u:%11d,",&nameid,&is_discount) < 1) {
				ShowError("npc_parse_shop: Invalid item cost definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
				return strchr(start,'\n'); // skip and continue
			}
			if (itemdb_exists(nameid) == NULL) {
				ShowWarning("npc_parse_shop: Invalid item ID cost in file '%s', line '%d' (id '%u').\n", filepath, strline(buffer,start-buffer), nameid);
				return strchr(start,'\n'); // skip and continue
			}
			p = strchr(p+1,',');
			break;
		}
		case NPCTYPE_POINTSHOP: {
			if (sscanf(p, ",%32[^,:]:%11d,",point_str,&is_discount) < 1) {
				ShowError("npc_parse_shop: Invalid item cost definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
				return strchr(start,'\n'); // skip and continue
			}
			switch(point_str[0]) {
				case '$':
				case '.':
				case '\'':
					ShowWarning("npc_parse_shop: Invalid item cost variable type (must be permanent character or account based) in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
					return strchr(start,'\n'); // skip and continue
					break;
			}
			if (point_str[strlen(point_str) - 1] == '$') {
				ShowWarning("npc_parse_shop: Invalid item cost variable type (must be integer) in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
				return strchr(start,'\n'); // skip and continue
			}
			p = strchr(p+1,',');
			break;
		}
		case NPCTYPE_MARKETSHOP:
#if PACKETVER < 20131223
			ShowError("npc_parse_shop: (MARKETSHOP) Feature is disabled, need client 20131223 or newer. Ignoring file '%s', line '%d\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer, start - buffer), w1, w2, w3, w4);
			return strchr(start, '\n'); // skip and continue
#else
			is_discount = 0;
			break;
#endif
		default:
			if( sscanf( p, ",%32[^,:]:%11d,", point_str, &is_discount ) == 2 ){
				is_discount = 1;
			}else{
				if( !strcasecmp( point_str, "yes" ) ){
					is_discount = 1;
				}else if( !strcasecmp( point_str, "no" ) ){
					is_discount = 0;
				}else{
					ShowError( "npc_parse_shop: unknown discount setting %s\n", point_str );
					return strchr( start, '\n' ); // skip and continue
				}

				p = strchr( p + 1, ',' );
			}
			break;
		}
	
	nd = npc_create_npc(m, x, y);
	nd->u.shop.count = 0;
	while ( p ) {
		t_itemid nameid2;
		unsigned short qty = 0;
		int value;
		struct item_data* id;
		bool skip = false;
		// Bartershop BACHNT
		int maxbartercurrency = 8;	// Max 8 Item Currency
		//struct npc_barter_currency* bartercurrency[maxbartercurrency];
		struct npc_barter_currency bartercurrency[maxbartercurrency];
		char c_barteritemlist[250];
		char *c_barteriteminfo,*c_temp;
		int barterexid,barterexrefine,barterexamount;
		////////////

		if( p == NULL )
			break;
		switch(type) {
			// BarterShop BACHNT
			case NPCTYPE_EXBARTERSHOP:
#if PACKETVER >= 20190116
				if(sscanf(p, ",%u:%d:%d%[0-9:-]", &nameid2, &quantity,&value,&c_barteritemlist) != 4) {
					ShowError("npc_parse_shop: (EXBARTERSHOP) Invalid item sell definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer, start - buffer), w1, w2, w3, w4);
					skip = true;
				}
				//ShowError("npc_parse_shop: (NPCTYPE_EXBARTERSHOP) nameid2 %d",nameid2);
				c_barteriteminfo = strchr((char *)c_barteritemlist,'-');
				currencycount = 0;
				//ShowWarning("p %s \t\n",p);
				while (c_barteriteminfo) {
					//ShowWarning("c_barteriteminfo %s \t\n",c_barteriteminfo);
					if(sscanf(c_barteriteminfo,"-%u:%u:%u",&barterexid,&barterexrefine,&barterexamount) != 3) {
						ShowError("npc_parse_shop: (EXBARTERSHOP) Invalid item COST definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer, start - buffer), w1, w2, w3, w4);
						skip = true;
					}
					//ShowWarning("c_barteriteminfo %s \tbarterexid %d \tbarterexrefine %d \tbarterexamount %d \t\n",c_barteriteminfo,barterexid,barterexrefine,barterexamount);
					//bartercurrency[currencycount] = new struct npc_barter_currency;
					//bartercurrency[currencycount]->nameid = barterexid;
					//bartercurrency[currencycount]->refine = barterexrefine;
					//bartercurrency[currencycount]->amount = barterexamount;
					bartercurrency[currencycount].nameid = barterexid;
					bartercurrency[currencycount].refine = barterexrefine;
					bartercurrency[currencycount].amount = barterexamount;
					c_barteriteminfo = strchr(c_barteriteminfo + 1,'-');
					currencycount++;
					totalcurrency++;
				}	
#endif
				break;		
			case NPCTYPE_BARTERSHOP:
#if PACKETVER >= 20190116
				if (sscanf(p, ",%u:%d:%u:%d", &nameid2, &quantity,&nameidtrade, &quantityneed) != 4) {
					ShowError("npc_parse_shop: (BARTERSHOP) Invalid item definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer, start - buffer), w1, w2, w3, w4);
					skip = true;
			}
#endif
				break;
		///////////////
			case NPCTYPE_MARKETSHOP:
#if PACKETVER >= 20131223
				if (sscanf(p, ",%u:%11d:%6hu", &nameid2, &value, &qty) != 3) {
					ShowError("npc_parse_shop: (MARKETSHOP) Invalid item definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer, start - buffer), w1, w2, w3, w4);
					skip = true;
				}
#endif
				break;
			default:
				if (sscanf(p, ",%u:%11d", &nameid2, &value) != 2) {
					ShowError("npc_parse_shop: Invalid item definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer, start - buffer), w1, w2, w3, w4);
					skip = true;
				}

				break;
			}

		if (skip)
			break;

		if( (id = itemdb_exists(nameid2)) == NULL ) {
			ShowWarning("npc_parse_shop: Invalid sell item in file '%s', line '%d' (id '%u').\n", filepath, strline(buffer,start-buffer), nameid2);
			p = strchr(p+1,',');
			continue;
		}
		if( value < 0 ) {
			if (type == NPCTYPE_SHOP || type == NPCTYPE_MARKETSHOP) value = id->value_buy;
			else value = 0; // Cashshop doesn't have a "buy price" in the item_db
		}
		if (value == 0 && (type == NPCTYPE_SHOP || type == NPCTYPE_MARKETSHOP)) { // NPC selling items for free!
			ShowWarning("npc_parse_shop: Item %s [%u] is being sold for FREE in file '%s', line '%d'.\n",
				id->name.c_str(), nameid2, filepath, strline(buffer,start-buffer));
		}
		if( type == NPCTYPE_SHOP && value*0.75 < id->value_sell*1.24 ) { // Exploit possible: you can buy and sell back with profit
			ShowWarning("npc_parse_shop: Item %s [%u] discounted buying price (%d->%d) is less than overcharged selling price (%d->%d) at file '%s', line '%d'.\n",
				id->name.c_str(), nameid2, value, (int)(value*0.75), id->value_sell, (int)(id->value_sell*1.24), filepath, strline(buffer,start-buffer));
		}
		if (type == NPCTYPE_MARKETSHOP && (!qty || qty > UINT16_MAX)) {
			ShowWarning("npc_parse_shop: Item %s [%u] is stocked with invalid value %d, changed to 1. File '%s', line '%d'.\n",
				id->name.c_str(), nameid2, qty, filepath, strline(buffer,start-buffer));
			qty = 1;
		}
		//for logs filters, atcommands and iteminfo script command
		if( id->maxchance == 0 )
			id->maxchance = -1; // -1 would show that the item's sold in NPC Shop
		
#if PACKETVER >= 20131223
		if (nd->u.shop.count && type == NPCTYPE_MARKETSHOP) {
			uint16 i;
			// Duplicate entry? Replace the value
			ARR_FIND(0, nd->u.shop.count, i, nd->u.shop.shop_item[i].nameid == nameid);
			if (i != nd->u.shop.count) {
				nd->u.shop.shop_item[i].qty = qty;
				nd->u.shop.shop_item[i].value = value;
				p = strchr(p+1,',');
				continue;
			}
		}
#endif

		if(type == NPCTYPE_BARTERSHOP) { // Barter shop BACHNT
		RECREATE(nd->u.shop.shop_item, struct npc_item_list,nd->u.shop.count+1);
		nd->u.shop.shop_item[nd->u.shop.count].nameid = nameid2;
			nd->u.shop.shop_item[nd->u.shop.count].qty = quantity;
			nd->u.shop.shop_item[nd->u.shop.count].nameidtrade = nameidtrade;
			nd->u.shop.shop_item[nd->u.shop.count].value2 = quantityneed;
			
#if PACKETVER >= 20131223
			nd->u.shop.shop_item[nd->u.shop.count].flag = 0;
#endif
		}
		else if(type == NPCTYPE_EXBARTERSHOP) {
			RECREATE(nd->u.shop.shop_item, struct npc_item_list,nd->u.shop.count+1);
			nd->u.shop.shop_item[nd->u.shop.count].nameid = nameid2;
			nd->u.shop.shop_item[nd->u.shop.count].qty = quantity;
			nd->u.shop.shop_item[nd->u.shop.count].value = value;
			nd->u.shop.shop_item[nd->u.shop.count].value2 = currencycount;
			nd->u.shop.shop_item[nd->u.shop.count].currency = new struct npc_barter_currency[currencycount];
			for(int n = 0; n < currencycount; n++){				
				nd->u.shop.shop_item[nd->u.shop.count].currency[n].nameid = bartercurrency[n].nameid;
				nd->u.shop.shop_item[nd->u.shop.count].currency[n].refine = bartercurrency[n].refine;
				nd->u.shop.shop_item[nd->u.shop.count].currency[n].amount = bartercurrency[n].amount;
			}
			//aFree((struct npc_barter_currency *)(char *)bartercurrency);
#if PACKETVER >= 20131223
			nd->u.shop.shop_item[nd->u.shop.count].flag = 0;
#endif
		}		
		else {			
			RECREATE(nd->u.shop.shop_item, struct npc_item_list,nd->u.shop.count+1);
			nd->u.shop.shop_item[nd->u.shop.count].nameid = nameid2;
			nd->u.shop.shop_item[nd->u.shop.count].value = value;
#if PACKETVER >= 20131223
		nd->u.shop.shop_item[nd->u.shop.count].flag = 0;
		if (type == NPCTYPE_MARKETSHOP )
			nd->u.shop.shop_item[nd->u.shop.count].qty = qty;
#endif
		} // Bartershop BACHNT
		nd->u.shop.count++;
		p = strchr(p+1,',');
	}
	if( nd->u.shop.count == 0 ) {
		ShowWarning("npc_parse_shop: Ignoring empty shop in file '%s', line '%d'.\n", filepath, strline(buffer,start-buffer));
		aFree(nd);
		return strchr(start,'\n');// continue
	}

	if( type == NPCTYPE_ITEMSHOP ){
		// Item shop currency
		nd->u.shop.itemshop_nameid = nameid;
	}else if( type == NPCTYPE_POINTSHOP ){
		// Point shop currency
		safestrncpy( nd->u.shop.pointshop_str, point_str, strlen( point_str ) + 1 );
	}else if( type == NPCTYPE_EXBARTERSHOP ){ // Bartershop BACHNT
		// Total Currency for Expanded Barter Shop
		nd->u.shop.totalcurrency = totalcurrency;
	}

	nd->u.shop.discount = is_discount > 0;

	npc_parsename(nd, w3, start, buffer, filepath);
	nd->class_ = m == -1 ? JT_FAKENPC : npc_parseview(w4, start, buffer, filepath);
	nd->speed = 200;

	++npc_shop;
	nd->bl.type = BL_NPC;
	nd->subtype = type;
#if PACKETVER >= 20131223
	// Insert market data to table
	if (nd->subtype == NPCTYPE_MARKETSHOP) {
		uint16 i;
		for (i = 0; i < nd->u.shop.count; i++)
			npc_market_tosql(nd->exname, &nd->u.shop.shop_item[i]);
	}
#endif
	if( m >= 0 )
	{// normal shop npc
		map_addnpc(m,nd);
		if(map_addblock(&nd->bl))
			return strchr(start,'\n');
		status_change_init(&nd->bl);
		unit_dataset(&nd->bl);
		nd->ud.dir = (uint8)dir;
		if( nd->class_ != JT_FAKENPC ){
			status_set_viewdata(&nd->bl, nd->class_);
			if( map_getmapdata(nd->bl.m)->users )
				clif_spawn(&nd->bl);
		}
	} else
	{// 'floating' shop?
		map_addiddb(&nd->bl);
	}
	strdb_put(npcname_db, nd->exname, nd);
	return strchr(start,'\n');// continue
}


// beforce

/**
 * Parses a shop/cashshop npc.
 * Line definition :
 * <map name>,<x>,<y>,<facing>%TAB%shop%TAB%<NPC Name>%TAB%<sprite id>,<itemid>:<price>{,<itemid>:<price>...}
 * <map name>,<x>,<y>,<facing>%TAB%cashshop%TAB%<NPC Name>%TAB%<sprite id>,<itemid>:<price>{,<itemid>:<price>...}
 * <map name>,<x>,<y>,<facing>%TAB%itemshop%TAB%<NPC Name>%TAB%<sprite id>,<costitemid>{:<discount>},<itemid>:<price>{,<itemid>:<price>...}
 * <map name>,<x>,<y>,<facing>%TAB%pointshop%TAB%<NPC Name>%TAB%<sprite id>,<costvariable>{:<discount>},<itemid>:<price>{,<itemid>:<price>...}
 * <map name>,<x>,<y>,<facing>%TAB%marketshop%TAB%<NPC Name>%TAB%<sprite id>,<itemid>:<price>:<quantity>{,<itemid>:<price>:<quantity>...}
 * @param w1 : word 1 before tab (<from map name>,<x>,<y>,<facing>)
 * @param w2 : word 2 before tab (shop|cashshop|itemshop|pointshop|marketshop), keyword that sent us in this parsing
 * @param w3 : word 3 before tab (<NPC Name>)
 * @param w4 : word 4 before tab (<sprited id>,<shop definition...>)
 * @param start : index to start parsing
 * @param buffer : lines to parses
 * @param filepath : filename with path wich we are parsing
 * @return new index for next parsing
 */
/*