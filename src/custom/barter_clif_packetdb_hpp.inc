
///////////////////////
/// Barter Shop BACHNT
///////////////////////
parseable_packet(0x0b12,2,clif_parse_npc_barter_closed,0);
parseable_packet(0x0b58,2,clif_parse_npc_expanded_barter_closed,0);

parseable_packet(0x0b57,-1,clif_parse_npc_expanded_barter_purchase,2,6);
parseable_packet(0x0b0f,-1,clif_parse_NPCBarterPurchase,2,6);


/*

#endif /* CLIF_PACKETDB_HPP */
