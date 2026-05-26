/* -*- C++ -*- */

/****************************************************************************
** Copyright (c) 2001-2014
**
** This file is part of the QuickFIX FIX Engine
**
** This file may be distributed under the terms of the quickfixengine.org
** license as defined by quickfixengine.org and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.quickfixengine.org/LICENSE for licensing information.
**
** Contact ask@quickfixengine.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifndef FIX_MUTEX_H
#define FIX_MUTEX_H

#include <mutex>

namespace FIX {
/// Portable implementation of a recursive mutex.
using Mutex = std::recursive_mutex;

/// Locks/Unlocks a mutex using RAII.
using Locker = std::lock_guard<Mutex>;

/// Does the opposite of the Locker to ensure mutex ends up in a locked state.
class ReverseLocker {
public:
  ReverseLocker(Mutex &mutex)
      : m_mutex(mutex) {
    m_mutex.unlock();
  }

  ~ReverseLocker() { m_mutex.lock(); }

private:
  Mutex &m_mutex;
};
} // namespace FIX

#endif
