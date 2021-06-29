/* after
			struct npc_label_list {
			char name[NAME_LENGTH+1];
			int pos;
			};
*/


/////////////////
/// Barter Shop BACHNT
/////////////////
struct npc_barter_currency {
	t_itemid nameid;
	int refine;
	int amount;
};
struct npc_shop_data {
	unsigned char type;/* what am i */
	struct npc_item_list *item;/* list */
	unsigned short items;/* total */
	int shop_last_index;  // only for NST_EXPANDED_BARTER
};
struct s_barter_buy_list {
	t_itemid itemId;
	int amount;
	int invIndex;
	int shopIndex;
	unsigned short qty;		///< Amount of item will be bought
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 nameid;	///< ID of item will be bought
#else
	uint16 nameid;	///< ID of item will be bought
#endif
} __attribute__((packed));

int npc_barter_buylist(struct map_session_data *sd, int count, struct s_barter_buy_list *item_list);
int npc_barter_ex_buylist(struct map_session_data *sd, int count, struct s_barter_buy_list *item_list);
static int npc_barter_buylist_sub(struct map_session_data* sd, uint16 n, struct s_barter_buy_list *item_list, struct npc_data* ndmaster, struct npc_data* nd);