/* after
			else if( (!strcasecmp(w2,"shop") || !strcasecmp(w2,"cashshop") || !strcasecmp(w2,"itemshop") || !strcasecmp(w2,"pointshop") || !strcasecmp(w2,"marketshop") ) && count > 3 )
			p = npc_parse_shop(w1,w2,w3,w4, p, buffer, filepath);
*/

else if( (!strcasecmp(w2,"bartershop") || !strcasecmp(w2,"exbartershop")) && count > 3 )
	p = npc_parse_shop(w1,w2,w3,w4, p, buffer, filepath);