# mruby-native-ext-type

A small mruby gem that gives serialization gems a shared, standardised way to declare and check field types on Ruby classes — without coupling the schema to any one serialization format.

## Motivation

Gems like `mruby-cbor` and `mruby-msgpack` need to know which ivar should hold which type when decoding into a registered class. Without a shared convention each gem invents its own DSL, and user classes have to repeat the declarations for every format they support.

`mruby-native-ext-type` (NED) provides a single DSL that any gem can read via the C API.

## Installation

Add to your `mrbgem.rake`:

```ruby
spec.add_dependency 'mruby-native-ext-type', github: 'Asmod4n/mruby-native-ext-type'
```

## Ruby API

### `native_ext_type(ivar_sym, *Types)` — class/module method

Declares that an ivar must be an instance of one of the given classes or modules. Multiple types form a union (value must satisfy at least one).

```ruby
class Address
  native_ext_type :@street, String
  native_ext_type :@zip,    Integer
end

class Person
  native_ext_type :@name,    String
  native_ext_type :@age,     Integer
  native_ext_type :@address, Address
end

# Union: @kind may be Animal or Dog
class Pet
  native_ext_type :@kind, Animal, Dog
end
```

Works on modules too:

```ruby
module Configurable
  native_ext_type :@timeout, Integer
end
```

### `ClassName.net_schema` → `Hash | nil`

Returns the raw schema hash `{ :@ivar => [Type, ...] }` for the class, or `nil` if nothing has been declared. Primarily useful for debugging or for gem authors consuming the schema in Ruby.

```ruby
Person.net_schema
# => { :@name => [String], :@age => [Integer], :@address => [Address] }

Class.new.net_schema
# => nil
```

### `ClassName.net_check_type(:@ivar, value)` → `true | false`

Checks whether `value` satisfies the declared type(s) for the given ivar. Uses `kind_of?` semantics so subclasses pass automatically. Returns `false` (never raises) when the ivar is undeclared or the class has no schema at all.

```ruby
Person.net_check_type(:@name, "Alice")   # => true
Person.net_check_type(:@name, 42)        # => false

# Inheritance
class Dog < Animal; end
Person.net_check_type(:@pet, Dog.new)   # => true  (Dog is kind_of? Animal)

# Union
Pet.net_check_type(:@kind, Animal.new)   # => true
Pet.net_check_type(:@kind, Dog.new)      # => true
Pet.net_check_type(:@kind, "Fido")       # => false
```

## C API

Include `<mruby/ned.h>` and link against the gem.

### `mrb_net_schema`

```c
MRB_API mrb_value mrb_net_schema(mrb_state *mrb, struct RClass *klass);
```

Returns the schema hash for `klass`, or `mrb_nil_value()` if none exists.

### `mrb_net_check_type`

```c
MRB_API mrb_bool mrb_net_check_type(mrb_state *mrb, mrb_value schema_type, mrb_value actual);
```

`schema_type` is the `Array` of classes/modules stored under a given ivar key (i.e. one value from the hash returned by `mrb_net_schema`). Returns `TRUE` if `actual` is a `kind_of?` match for any entry, `FALSE` otherwise. Never raises.

Typical usage inside a decoder:

```c
mrb_value schema = mrb_net_schema(mrb, mrb_class_ptr(klass));
if (mrb_hash_p(schema)) {
  mrb_value schema_type = mrb_hash_fetch(mrb, schema, mrb_symbol_value(ivar), mrb_nil_value());
  if (!mrb_net_check_type(mrb, schema_type, value)) {
    mrb_raisef(mrb, E_TYPE_ERROR, "%s type check failed (got %C)",
               mrb_sym_name(mrb, ivar), mrb_class(mrb, value));
  }
}
```

## Schema storage

The schema is stored as an ivar `__native_ext_type__` directly on the class object. It is a plain `Hash` so it survives GC normally and is visible to any gem that knows the ivar name, though the C API functions are the stable interface.

## License

Apache 2.0 — see [LICENSE](LICENSE).