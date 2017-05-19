/**
 * @file: family.c
 *
 * @description: Family relationships definitions.
 *
 * @author: Peter Mawhorter (pmawhorter@gmail.com)
 */

#include <malloc.h>

#include "family.h"

/*************************
 * Structure Definitions *
 *************************/

struct acy_family_info_s {
  id birth_rate_per_day;
  id minimum_age_at_first_child;
  id childbearing_days;
  id average_children_per_mother; // so that population is stable TODO: gender!?
  id max_children_per_mother;
  double child_age_distribution_shape;
  id seed;
};
typedef struct acy_family_info_s acy_family_info;

/*************
 * Constants *
 *************/

acy_family_info const DEFAULT_FAMILY_INFO = {
  .birth_rate_per_day = 10000,
  .minimum_age_at_first_child = 15 * ONE_EARTH_YEAR,
  .childbearing_days = 25 * ONE_EARTH_YEAR,
  .average_children_per_mother = 1,
  .max_children_per_mother = 32,
  .child_age_distribution_shape = 10.0,
  .seed = 9728182391
};

/*************
 * Functions *
 *************/

acy_family_info *acy_create_family_info() {
  return (acy_family_info*) malloc(sizeof(acy_family_info));
}

// Destroys a heap-allocated family info object.
void acy_destroy_family_info(acy_family_info *info) {
  free(info);
}

void acy_copy_family_info(
  acy_family_info const * const src,
  acy_family_info *dst
) {
  dst->birth_rate_per_day = src->birth_rate_per_day;
  dst->minimum_age_at_first_child = src->minimum_age_at_first_child;
  dst->childbearing_days = src->childbearing_days;
  dst->average_children_per_mother = src->average_children_per_mother;
  dst->max_children_per_mother = src->max_children_per_mother;
  dst->child_age_distribution_shape = src->child_age_distribution_shape;
  dst->seed = src->seed;
}

void acy_set_info_seed(acy_family_info *info, id seed) {
  info->seed = seed;
}

id acy_get_info_seed(acy_family_info *info) {
  return info->seed;
}

id acy_birthdate(id person, acy_family_info const * const info) {
  return acy_mixed_cohort(person, info->birth_rate_per_day, 0);
  // TODO: Remove this
  //return acy_cohort(person, info->birth_rate_per_day, 0);
}

id acy_first_born_on(id day, acy_family_info const * const info) {
  return acy_mixed_cohort_outer(day, 0, info->birth_rate_per_day, 0);
  // TODO: Remove this
  //return acy_cohort_outer(day, 0, info->birth_rate_per_day, 0);
}

id acy_get_child_id_adjust(acy_family_info const * const info) {
  return info->birth_rate_per_day * info->minimum_age_at_first_child;
}

id acy_mother(id person, acy_family_info const * const info) {
  id parent, index;
  acy_mother_and_index(person, info, &parent, &index);
  return parent;
}

#define FAMILY_COHORT_LAYERS 4
static inline id acy_family_children_cohort_size(
  acy_family_info const * const info
) {
  return (
    (info->birth_rate_per_day * info->childbearing_days)
  / (info->max_children_per_mother*FAMILY_COHORT_LAYERS)
  );
}

void acy_mother_and_index(
  id person,
  acy_family_info const * const info,
  id *r_mother,
  id *r_index
) {
  if (person == NONE) {
    *r_mother = NONE;
    *r_index = 0;
    return;
  }

  // Correct age gap:
  id adjusted = person - acy_get_child_id_adjust(info);
  /*
   * TODO: Check underflow!
  if (adjusted > person) { // underflow
    *r_mother = NONE;
    *r_index = 0;
    return;
  }
  */
  id cohort_size = acy_family_children_cohort_size(info);
  acy_select_exp_parent_and_index(
    adjusted,
    info->average_children_per_mother,
    info->max_children_per_mother,
    info->child_age_distribution_shape,
    cohort_size,
    FAMILY_COHORT_LAYERS,
    info->seed,
    r_mother,
    r_index
  );
  /*
   * TODO: Check underflow
  if (*r_mother > person) { // underflow
    *r_mother = NONE;
    *r_index = 0;
    return;
  }
  */
}

id acy_child(id person, id nth, acy_family_info const * const info) {
  id cohort_size = acy_family_children_cohort_size(info);
  id child = acy_select_exp_nth_child(
    person,
    nth,
    info->average_children_per_mother,
    info->max_children_per_mother,
    info->child_age_distribution_shape,
    cohort_size,
    FAMILY_COHORT_LAYERS,
    info->seed
  );
  if (child == NONE) { return NONE; } // mother doesn't have this many children
  // Introduce age gap:
  id adjusted = person + acy_get_child_id_adjust(info);
  /*
   * TODO: Check overflow
  if (adjusted < child || child < person) { return NONE; } // overflow
  */
  return adjusted;
}
