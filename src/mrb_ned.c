#include <mruby.h>
#include <mruby/class.h>
#include <mruby/hash.h>
#include <mruby/variable.h>
#include <mruby/presym.h>
#include <mruby/error.h>

#include <mruby/ned.h>

/*
 * Ruby DSL: native_ext_type :@field, SomeClass
 *
 * Stores { :@field => SomeClass } in the class-level ivar __native_ext_type__.
 * The second argument must be a Class or Module; raises TypeError otherwise.
 *
 * Any Ruby class works, including user-defined ones registered with
 * CBOR.register_tag or equivalent:
 *
 *   class Address; end
 *   CBOR.register_tag(1001, Address)
 *
 *   class Person
 *     native_ext_type :@name,    String
 *     native_ext_type :@age,     Integer
 *     native_ext_type :@address, Address   # previously registered class
 *   end
 */
static mrb_value
mrb_native_ext_type(mrb_state *mrb, mrb_value self)
{
  mrb_sym   name;
  mrb_value type;

  mrb_get_args(mrb, "no", &name, &type);

  if (!mrb_class_p(type) && !mrb_module_p(type)) {
    mrb_raise(mrb, E_TYPE_ERROR,
      "native_ext_type: second argument must be a Class or Module");
  }

  mrb_value schema = mrb_iv_get(mrb, self, MRB_SYM(__native_ext_type__));
  if (mrb_nil_p(schema)) {
    schema = mrb_hash_new_capa(mrb, 16);
    mrb_iv_set(mrb, self, MRB_SYM(__native_ext_type__), schema);
  }

  mrb_hash_set(mrb, schema, mrb_symbol_value(name), type);

  return mrb_nil_value();
}

MRB_API mrb_value
mrb_net_schema(mrb_state *mrb, struct RClass *klass)
{
  return mrb_iv_get(mrb, mrb_obj_value(klass), MRB_SYM(__native_ext_type__));
}

MRB_API mrb_bool
mrb_ned_check_type(mrb_state *mrb, mrb_value schema_type, mrb_value actual)
{
  if (!mrb_class_p(schema_type) && !mrb_module_p(schema_type)) {
    /* Schema was stored with a non-class value — programming error,
       fail closed rather than accepting everything. */
    return FALSE;
  }
  return mrb_obj_is_kind_of(mrb, actual, mrb_class_ptr(schema_type));
}

void
mrb_mruby_native_ext_type_gem_init(mrb_state *mrb)
{
  mrb_define_class_method_id(mrb,
    mrb->class_class,
    MRB_SYM(native_ext_type),
    mrb_native_ext_type,
    MRB_ARGS_REQ(2));

  mrb_define_module_function_id(mrb,
    mrb->module_class,
    MRB_SYM(native_ext_type),
    mrb_native_ext_type,
    MRB_ARGS_REQ(2));
}

void
mrb_mruby_native_ext_type_gem_final(mrb_state *mrb)
{
}