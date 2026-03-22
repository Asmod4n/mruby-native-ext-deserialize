#include <mruby.h>
#include <mruby/class.h>
#include <mruby/hash.h>
#include <mruby/variable.h>
#include <mruby/presym.h>
#include <mruby/error.h>
#include <mruby/array.h>
#include <mruby/ned.h>

/*
 * Ruby DSL: native_ext_type :@field, SomeClass
 *
 * Stores { :@field => SomeClass } in the class-level ivar __native_ext_type__.
 * The second argument must be a Class or Module; raises TypeError otherwise.

 *
 *   class Address; end
 *
 *   class Person
 *     native_ext_type :@name,    String
 *     native_ext_type :@age,     Integer
 *     native_ext_type :@address, Address
 *   end
 */
static mrb_value
mrb_native_ext_type(mrb_state *mrb, mrb_value self)
{
  mrb_sym ivar;
  mrb_value *types;
  mrb_int ntypes;
  mrb_get_args(mrb, "n*", &ivar, &types, &ntypes);

  if (ntypes == 0)
    mrb_raise(mrb, E_ARGUMENT_ERROR, "native_ext_type requires at least one type");

  for (mrb_int i = 0; i < ntypes; i++) {
    if (!mrb_class_p(types[i]) && !mrb_module_p(types[i]))
      mrb_raisef(mrb, E_TYPE_ERROR,
        "type argument %d is not a Class or Module", i + 1);
  }

  mrb_value schema = mrb_net_schema(mrb, mrb_class_ptr(self));
  if (mrb_nil_p(schema)) {
    schema = mrb_hash_new_capa(mrb, 16);
    mrb_iv_set(mrb, self, MRB_SYM(__native_ext_type__), schema);
  }
  mrb_value type_ary = mrb_ary_new_from_values(mrb, ntypes, types);
  struct RBasic *schema_basic = mrb_basic_ptr(schema);
  schema_basic->frozen = FALSE;
  mrb_hash_set(mrb, schema, mrb_symbol_value(ivar), type_ary);
  schema_basic->frozen = TRUE;
  return mrb_nil_value();
}

static mrb_value
net_schema_rb(mrb_state *mrb, mrb_value self)
{
  return mrb_net_schema(mrb, mrb_class_ptr(self));
}

static mrb_value
net_check_type_rb(mrb_state *mrb, mrb_value self)
{
  mrb_sym ivar;
  mrb_value actual;
  mrb_get_args(mrb, "no", &ivar, &actual);

  mrb_value schema = mrb_net_schema(mrb, mrb_class_ptr(self));
  if (!mrb_hash_p(schema))
    return mrb_false_value();

  mrb_value schema_type = mrb_hash_fetch(mrb, schema, mrb_symbol_value(ivar), mrb_nil_value());
  return mrb_bool_value(mrb_net_check_type(mrb, schema_type, actual));
}

MRB_BEGIN_DECL

MRB_API mrb_value
mrb_net_schema(mrb_state *mrb, struct RClass *klass)
{
  return mrb_iv_get(mrb, mrb_obj_value(klass), MRB_SYM(__native_ext_type__));
}

MRB_API mrb_bool
mrb_net_check_type(mrb_state *mrb, mrb_value schema_type, mrb_value actual)
{
  if (!mrb_array_p(schema_type)) return FALSE;

  mrb_int len = RARRAY_LEN(schema_type);
  mrb_value *ptr = RARRAY_PTR(schema_type);
  for (mrb_int i = 0; i < len; i++) {
    if ((mrb_class_p(ptr[i]) || mrb_module_p(ptr[i])) &&
        mrb_obj_is_kind_of(mrb, actual, mrb_class_ptr(ptr[i])))
      return TRUE;
  }
  return FALSE;
}

void
mrb_mruby_native_ext_type_gem_init(mrb_state *mrb)
{
  mrb_define_class_method_id(mrb,
    mrb->class_class,
    MRB_SYM(native_ext_type),
    mrb_native_ext_type,
    MRB_ARGS_REQ(2)|MRB_ARGS_REST());

  mrb_define_module_function_id(mrb,
    mrb->module_class,
    MRB_SYM(native_ext_type),
    mrb_native_ext_type,
    MRB_ARGS_REQ(2)|MRB_ARGS_REST());

  mrb_define_method_id(mrb, mrb->class_class,
    MRB_SYM(net_schema),
    net_schema_rb,
    MRB_ARGS_NONE());

  mrb_define_method_id(mrb, mrb->module_class,
    MRB_SYM(net_schema),
    net_schema_rb,
    MRB_ARGS_NONE());

  mrb_define_method_id(mrb, mrb->class_class,
    MRB_SYM(net_check_type),
    net_check_type_rb,
    MRB_ARGS_REQ(2));

  mrb_define_method_id(mrb, mrb->module_class,
    MRB_SYM(net_check_type),
    net_check_type_rb,
    MRB_ARGS_REQ(2));
}

void
mrb_mruby_native_ext_type_gem_final(mrb_state *mrb)
{
  (void)mrb;
}

MRB_END_DECL