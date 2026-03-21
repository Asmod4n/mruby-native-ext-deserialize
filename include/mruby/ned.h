#pragma once

#include <mruby.h>

MRB_BEGIN_DECL

/*
 * mrb_net_schema - retrieve the native_ext_type schema hash for a class.
 *
 * Returns a Hash of { mrb_sym(ivar_name) => mrb_value(Class) },
 * or mrb_nil_value() if no schema has been declared.
 *
 * The schema value is always a Class or Module object (mrb_class_p / mrb_module_p).
 * Serialization gems must not assume it is an integer bitmask.
 */
MRB_API mrb_value
mrb_net_schema(mrb_state *mrb, struct RClass *klass);

/*
 * mrb_ned_check_type - validate that `actual` satisfies the schema type.
 *
 * `schema_type` must be a Class or Module object stored by native_ext_type.
 * Uses mrb_obj_is_kind_of so inheritance works correctly: if the schema
 * declares Animal and the value is a Dog < Animal, it passes.
 *
 * Returns TRUE on match, FALSE on mismatch.
 * Never raises — the caller is responsible for producing a meaningful error.
 */
MRB_API mrb_bool
mrb_ned_check_type(mrb_state *mrb, mrb_value schema_type, mrb_value actual);

MRB_END_DECL