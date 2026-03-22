##
# mruby-native-ext-type tests
#
# Run via:  rake test
##

# ── fixtures ─────────────────────────────────────────────────────────────────

class Animal; end
class Dog < Animal; end

module Walkable; end
class Cat
  include Walkable
end

class Address
  native_ext_type :@street, String
  native_ext_type :@zip,    Integer
end

class Person
  native_ext_type :@name,    String
  native_ext_type :@age,     Integer
  native_ext_type :@address, Address
end

class Pet
  native_ext_type :@name,    String
  native_ext_type :@kind,    Animal, Dog   # union: Animal OR Dog
end

module Configurable
  native_ext_type :@timeout, Integer
end

# ── native_ext_type DSL ───────────────────────────────────────────────────────

assert("native_ext_type raises ArgumentError with no type arguments") do
  assert_raise(ArgumentError) do
    class NedArgErrorTest; native_ext_type :@name; end
  end
end

assert("native_ext_type raises TypeError for non-Class/Module type") do
  assert_raise(TypeError) do
    class NedTypeErrorTest1; native_ext_type :@name, 42; end
  end
  assert_raise(TypeError) do
    class NedTypeErrorTest2; native_ext_type :@name, "String"; end
  end
end

# ── net_schema ────────────────────────────────────────────────────────────────

assert("net_schema returns a Hash for a class with declared types") do
  s = Person.net_schema
  assert_true s.is_a?(Hash)
end

assert("net_schema contains declared ivar keys as symbols") do
  s = Person.net_schema
  assert_true s.key?(:@name)
  assert_true s.key?(:@age)
  assert_true s.key?(:@address)
end

assert("net_schema values are Arrays of Classes/Modules") do
  s = Person.net_schema
  assert_true s[:@name].is_a?(Array)
  assert_equal [String],  s[:@name]
  assert_equal [Integer], s[:@age]
  assert_equal [Address], s[:@address]
end

assert("net_schema returns nil for a class with no declarations") do
  klass = Class.new
  assert_nil klass.net_schema
end

assert("net_schema on a Module returns its schema") do
  s = Configurable.net_schema
  assert_true s.is_a?(Hash)
  assert_true s.key?(:@timeout)
end

# ── net_check_type ────────────────────────────────────────────────────────────

assert("net_check_type returns true when value matches declared type") do
  assert_true Person.net_check_type(:@name, "Alice")
  assert_true Person.net_check_type(:@age,  30)
end

assert("net_check_type returns false when value does not match") do
  assert_false Person.net_check_type(:@name, 42)
  assert_false Person.net_check_type(:@age,  "thirty")
end

assert("net_check_type works for nested registered classes") do
  addr = Address.new
  assert_true  Person.net_check_type(:@address, addr)
  assert_false Person.net_check_type(:@address, "not an address")
end

class NedInheritanceTest
  native_ext_type :@pet, Animal
end

assert("net_check_type respects inheritance (kind_of? semantics)") do
  assert_true Person.net_check_type(:@name, "Rex")

  # Dog is a kind_of Animal, so it satisfies an Animal type declaration
  assert_true  NedInheritanceTest.net_check_type(:@pet, Dog.new)
  assert_false NedInheritanceTest.net_check_type(:@pet, "not an animal")
end

assert("net_check_type works with Module (include) type constraints") do
  assert_true  Configurable.net_check_type(:@timeout, 5)
  assert_false Configurable.net_check_type(:@timeout, "five")
end

assert("net_check_type returns false for unknown ivar") do
  assert_false Person.net_check_type(:@nonexistent, "value")
end

assert("net_check_type returns false when class has no schema") do
  klass = Class.new
  assert_false klass.net_check_type(:@name, "Alice")
end

# ── union types ───────────────────────────────────────────────────────────────

assert("union types: net_check_type passes if value matches any declared type") do
  # Pet declares :@kind as Animal OR Dog
  assert_true Pet.net_check_type(:@kind, Animal.new)
  assert_true Pet.net_check_type(:@kind, Dog.new)
end

assert("union types: net_check_type fails if value matches none") do
  assert_false Pet.net_check_type(:@kind, Cat.new)
  assert_false Pet.net_check_type(:@kind, "Fido")
end

assert("union types: schema stores all declared types in a single Array") do
  ary = Pet.net_schema[:@kind]
  assert_equal [Animal, Dog], ary
end

# ── schema isolation ──────────────────────────────────────────────────────────

class NedIsolationA; native_ext_type :@x,        String;  end
class NedIsolationB; native_ext_type :@x,        Integer; end
class NedIsolationC; native_ext_type :@only_in_c, String; end
class NedIsolationD; end

assert("schemas are isolated per class") do
  assert_true  NedIsolationA.net_check_type(:@x, "hello")
  assert_false NedIsolationA.net_check_type(:@x, 1)
  assert_true  NedIsolationB.net_check_type(:@x, 1)
  assert_false NedIsolationB.net_check_type(:@x, "hello")
end

assert("adding a field to one class does not affect another") do
  assert_false NedIsolationD.net_check_type(:@only_in_c, "value")
end
