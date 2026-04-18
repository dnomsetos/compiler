#include <utility/output.hpp>

std::ostream& operator<<(std::ostream& out, const Dummy&) {
  out << "void";
  return out;
}
