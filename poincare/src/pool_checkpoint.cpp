#include <assert.h>
#include <poincare/pool.h>
#include <poincare/pool_checkpoint.h>
#include <poincare/pool_object.h>

namespace Poincare {

PoolCheckpoint* PoolCheckpoint::s_topmost = nullptr;

PoolCheckpoint::PoolCheckpoint()
    : m_parent(s_topmost), m_endOfPool(Pool::sharedPool->last()) {
  assert(!m_parent || m_endOfPool >= m_parent->m_endOfPool);
  assert(Internal::TreeStack::SharedTreeStack->size() == 0);
}

void PoolCheckpoint::protectedDiscard() const {
  if (s_topmost == this) {
    s_topmost = m_parent;
  }
}

void PoolCheckpoint::rollback() const {
  /* NOTE: The flow here may come from a Home press inside an external app, if
   * this is the case, value from the TreeStack and the Pool cannot be trusted
   * as they may have been corrupted by the external app heap usage.
   * Re-init the TreeStack.
   * The call to [freePoolFromObject] is enough to properly reset the pool.
   * TODO: maybe find a safer alternative */
#if ASSERTIONS
  /* NOTE: In this case, we need to deinit before init is called because an
   * assertion checks the initialization status of the GlobalBox */
  Internal::TreeStack::SharedTreeStack.deinit();
#endif
  Internal::TreeStack::SharedTreeStack.init();
  Pool::sharedPool->freePoolFromObject(m_endOfPool);
}

void PoolCheckpoint::rollbackException() {
  assert(s_topmost == this);
  discard();
  m_parent->rollbackException();
}

}  // namespace Poincare
