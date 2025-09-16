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
#if ASSERTIONS
  /* NOTE: This is a hack to bypass the assert in GlobalBox::init, indeed
   * this function is called in 2 differents cases, only one triggering the
   * assert:
   * - On a standard PoolCheckpoint::Raise:
   *     The TreeStack is already initialized and calling `init` triggers an
   *     assert
   * - On a CircuitBreakerCheckpoint when inside Code/External app:
   *     The TreeStack is uninitialized and the call to `init` is safe
   */
  Internal::TreeStack::SharedTreeStack.m_isInitialized = false;
#endif
  /* NOTE: A rollback may be triggered when:
   * - Pressing Home in an external app/code app
   * - On a PoolCheckpoint::Raise
   *
   * In both cases the TreeStack must be emptied but in the former the values it
   * contains can be corrupted by the heap usage so we re-init the TreeStack for
   * safety. */
  Internal::TreeStack::SharedTreeStack.init();
  /* Technically the [sharedPool] as the same corruption issue as the TreeStack
   * but in the latter case, it must not be emptied entirely. So we cannot
   * re-init it.
   * The call to [freePoolFromObject] seems resilient enough to properly reset
   * the pool even with a corrupt heap.
   * TODO : find a safer alternative: TrackedGlobalBox for those sharedObject */
  Pool::sharedPool->freePoolFromObject(m_endOfPool);
}

void PoolCheckpoint::rollbackException() {
  assert(s_topmost == this);
  discard();
  m_parent->rollbackException();
}

}  // namespace Poincare
