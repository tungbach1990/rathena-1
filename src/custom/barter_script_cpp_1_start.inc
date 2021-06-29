/* before
BUILDIN_FUNC(callshop)
{
	TBL_PC *sd = NULL;
	struct npc_data *nd;
	const char *shopname;
	int flag = 0;
	if (!script_rid2sd(sd)) {
		script_pushint(st,0);