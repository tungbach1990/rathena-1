
struct npc_data {
	struct block_list bl;
	struct unit_data ud; //Because they need to be able to move....
	struct view_data vd;
	struct status_change sc; //They can't have status changes, but.. they want the visual opt values.
	struct npc_data *master_nd;
	short class_,speed;
	char name[NPC_NAME_LENGTH+1];// display name
	char exname[NPC_NAME_LENGTH+1];// unique npc name
	int chat_id,touching_id;
	unsigned int next_walktime;
	int instance_id;
	unsigned size : 2;

	struct status_data status;
	unsigned int level,stat_point;
	struct s_npc_params {
		unsigned short str, agi, vit, int_, dex, luk;
	} params;

	void* chatdb; // pointer to a npc_parse struct (see npc_chat.cpp)
	char* path;/* path dir */
	enum npc_subtype subtype;
	bool trigger_on_hidden;
	int src_id;
	union {
		struct {
			struct script_code *script;
			short xs,ys; // OnTouch area radius
			int guild_id;
			t_tick timer;
			int timerid,timeramount,rid;
			t_tick timertick;
			struct npc_timerevent_list *timer_event;
			int label_list_num;
			struct npc_label_list *label_list;
			/* BACHNT */
			struct npc_shop_data *shop;
			bool trader;
		} scr;
		struct {
			struct npc_item_list *shop_item;
			uint16 count;
			int totalcurrency; // Bartershop BACHNT
			t_itemid itemshop_nameid; // Item Shop cost item ID
			char pointshop_str[32]; // Point Shop cost variable name
			bool discount;
			
			//unsigned char type;/* what am i */
			//struct npc_item_list *item;/* list */
			//unsigned short items;/* total */
			int shop_last_index;  // only for NPCTYPE_EXBARTER_SHOP BACHNT

		} shop;
		struct {
			short xs,ys; // OnTouch area radius
			short x,y; // destination coords
			unsigned short mapindex; // destination map
		} warp;
		struct {
			struct mob_data *md;
			time_t kill_time;
			char killer_name[NAME_LENGTH];
			int spawn_timer;
		} tomb;
	} u;

	struct sc_display_entry **sc_display;
	unsigned char sc_display_count;

	std::vector<std::shared_ptr<s_questinfo>> qi_data;

	struct {
		t_tick timeout;
		unsigned long color;
	} progressbar;
};