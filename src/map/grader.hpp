#ifndef GRADER_HPP
#define GRADER_HPP

#include "battle.hpp"

enum grade_level {
    ITEM_GRADE_NONE = 0,
    ITEM_GRADE_D    = 1,
    ITEM_GRADE_C    = 2,
    ITEM_GRADE_B    = 3,
    ITEM_GRADE_A    = 4,
    ITEM_GRADE_R    = 5,
    ITEM_GRADE_S    = 6,
    ITEM_GRADE_SS   = 7,
#ifndef ITEM_GRADE_MAX
    ITEM_GRADE_MAX
#endif
};

#ifndef MAX_ITEM_GRADE
#define MAX_ITEM_GRADE ITEM_GRADE_MAX - 1
#endif

enum grade_ui_failure_behavior {
    GRADE_FAILURE_BEHAVIOR_KEEP      = 0,
    GRADE_FAILURE_BEHAVIOR_DESTROY   = 1,
    GRADE_FAILURE_BEHAVIOR_DOWNGRADE = 2,
};

enum grade_announce_condition {
    GRADE_ANNOUNCE_NONE    = 0,
    GRADE_ANNOUNCE_SUCCESS = 1,
    GRADE_ANNOUNCE_FAILURE = 2,
    GRADE_ANNOUNCE_ALWAYS  = GRADE_ANNOUNCE_SUCCESS | GRADE_ANNOUNCE_FAILURE,
};

/* Structures */
struct grade_material {
    int nameid;
    int amount;
    int zeny_cost;
    enum grade_ui_failure_behavior failure_behavior;
};

struct grade_blessing {
    int nameid;
    int amount;
    int bonus;
    int max_blessing;
};

struct grade_interface_dbs {
    struct s_grade_info grade_info[battle_config.grader_max_used];
};

struct s_grade_info {
    int success_chance;
    enum grade_announce_condition announce;
    struct grade_material materials[MAX_GRADE_MATERIALS];
    struct grade_blessing blessing;
};

void grader_enchant_add_item (struct map_session_data *sd, int idx);
#endif /* BATTLE_HPP */