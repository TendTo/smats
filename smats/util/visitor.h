/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Visitor class.
 *
 * Base class for visitors.
 */
#pragma once

namespace smats {

template <class T>
class Visitor {
  virtual void visit(const T &e) = 0;
};

}  // namespace smats
