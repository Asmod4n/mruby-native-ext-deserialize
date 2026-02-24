#pragma once

#include <mruby.h>
#include <mruby/presym.h>
#include <mruby/variable.h>

MRB_BEGIN_DECL

MRB_API mrb_value
mrb_ned_schema(mrb_state* mrb, struct RClass* klass);

MRB_END_DECL