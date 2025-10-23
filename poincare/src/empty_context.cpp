#include <poincare/context_with_parent.h>
#include <poincare/empty_context.h>

namespace Poincare {

const Internal::Tree* EmptyContext::expressionForUserNamed(
    const Internal::Tree* symbolt) {
  return nullptr;  // TODO KUndef ?
}

}  // namespace Poincare
