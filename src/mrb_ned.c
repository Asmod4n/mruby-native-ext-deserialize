#include <mruby.h>
#include <mruby/class.h>
#include <mruby/hash.h>
#include <mruby/variable.h>
#include <mruby/presym.h>

#include <mruby/ned.h>

// Ruby-DSL: native_ext_deserialize :field, Type
static mrb_value
mrb_native_ext_deserialize(mrb_state* mrb, mrb_value self)
{
  mrb_sym name;
  mrb_value type;

  mrb_get_args(mrb, "nc", &name, &type);

  mrb_value schema = mrb_iv_get(mrb, self, MRB_SYM(__native_ext_deserialize__));
  if (mrb_nil_p(schema)) {
    schema = mrb_hash_new_capa(mrb, 16);
    mrb_iv_set(mrb, self, MRB_SYM(__native_ext_deserialize__), schema);
  }

  mrb_hash_set(mrb, schema, mrb_symbol_value(name), type);

  return mrb_nil_value();
}

MRB_API mrb_value
mrb_ned_schema(mrb_state* mrb, struct RClass* klass)
{
  return mrb_iv_get(mrb, mrb_obj_value(klass), MRB_SYM(__native_ext_deserialize__));
}

void
mrb_mruby_native_ext_deserialize_gem_init(mrb_state* mrb)
{
  mrb_define_class_method_id(mrb,
                    mrb->class_class,
                    MRB_SYM(native_ext_deserialize),
                    mrb_native_ext_deserialize,
                    MRB_ARGS_REQ(2));

  mrb_define_module_function_id(mrb,
                    mrb->module_class,
                    MRB_SYM(native_ext_deserialize),
                    mrb_native_ext_deserialize,
                    MRB_ARGS_REQ(2));
}

void
mrb_mruby_native_ext_deserialize_gem_final(mrb_state* mrb)
{
}
