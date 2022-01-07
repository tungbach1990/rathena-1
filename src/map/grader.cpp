#include "grader.hpp"
#include "clif.hpp"


#ifndef MAP_GRADER_H
#define MAP_GRADER_H

void grader_enchant_add_item(struct map_session_data *sd, int idx)
{
	nullpo_retv(sd);

	if (idx < 0 || idx >= sd->status.inventorySize)
		return;

	if (sd->inventory_data[idx] == NULL || sd->inventory_data[idx]->flag.no_grade == 0) {
		clif_grade_enchant_add_item_result_fail(sd);
		return;
	}

	if (sd->status.inventory[idx].grade < ITEM_GRADE_NONE || sd->status.inventory[idx].grade >= battle->bc->grader_max_used) {
		clif_grade_enchant_add_item_result_fail(sd);
		return;
	}

	const struct s_grade_info *gi = grader->get_grade_info(sd->status.inventory[idx].grade);
	clif_grade_enchant_add_item_result_success(sd, idx, gi);
}