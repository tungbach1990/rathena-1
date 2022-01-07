#ifndef GRADER_HPP
#define GRADER_HPP

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

struct s_grade_info {
    int success_chance;
    enum grade_announce_condition announce;
    struct grade_material materials[MAX_GRADE_MATERIALS];
    struct grade_blessing blessing;
};

enum grade_ui_failure_behavior {
    GRADE_FAILURE_BEHAVIOR_KEEP      = 0,
    GRADE_FAILURE_BEHAVIOR_DESTROY   = 1,
    GRADE_FAILURE_BEHAVIOR_DOWNGRADE = 2,
};
void grader_enchant_add_item (struct map_session_data *sd, int idx);
#endif /* BATTLE_HPP */