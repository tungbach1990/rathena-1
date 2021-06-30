/*
#ifdef PACKET_OBFUSCATION
/// Keys based on packet versions
struct s_packet_keys {
	unsigned int keys[3]; ///< 3-Keys
};
#endif
*/
////////////////////
// Barter Shop [Bachnt]
////////////////////

void clif_npc_barter_open(struct map_session_data *sd, struct npc_data *nd);
void clif_npc_expanded_barter_open(struct map_session_data *sd, struct npc_data *nd);
void clif_parse_NPCBarterPurchase(int fd, struct map_session_data *sd);
void clif_parse_npc_expanded_barter_purchase(int fd, struct map_session_data *sd);
void clif_parse_npc_expanded_barter_closed(int fd, struct map_session_data *sd);
void clif_parse_npc_barter_closed(int fd, struct map_session_data *sd);
struct barter_itemlist_entry {
	int addId;
	int addAmount;
	int removeIndex;
	int shopIndex;
};
