/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @copyright 2019 Drake (https://drake.mit.edu)
 * @licence Apache-2.0 license
 */
#include "smats/symbolic/variable.h"

#include <atomic>

#include "smats/util/exception.h"

namespace smats {

Variable::Id Variable::GetNextId(const Variable::Type type) {
  static std::atomic<Variable::Id> next_id{1};
  const std::size_t counter = next_id.fetch_add(1);
  // We'll assume that size_t is at least 8 bytes wide, so that we can pack the
  // counter into the lower 7 bytes of `id_` and the `Type` into the upper byte.
  static_assert(sizeof(Id) >= 8);
  return counter | (static_cast<std::size_t>(type) << (7 * 8));
}

Variable::Variable(const std::string& name, const Type type)
    : id_{GetNextId(type)}, name_{std::make_shared<const std::string>(name)} {
  SMATS_ASSERT(id_ > 0, "Variable ID must be greater than 0");
}

std::string Variable::name() const { return name_ != nullptr ? *name_ : std::string{"dummy"}; }

std::ostream& operator<<(std::ostream& os, const Variable& var) { return os << var.name(); }

std::ostream& operator<<(std::ostream& os, Variable::Type type) {
  switch (type) {
    case Variable::Type::CONTINUOUS:
      return os << "Continuous";
    case Variable::Type::BINARY:
      return os << "Binary";
    case Variable::Type::INTEGER:
      return os << "Integer";
    case Variable::Type::BOOLEAN:
      return os << "Boolean";
    default:
      SMATS_UNREACHABLE();
  }
}

}  // namespace smats
