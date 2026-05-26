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

#ifndef FIX_EVENT_H
#define FIX_EVENT_H

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace FIX {
/// Portable implementation of an event/conditional mutex
class Event {
public:
  void signal() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cv.notify_all();
  }

  void wait(double s) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait_for(lock, std::chrono::duration<double>(s));
  }

private:
  std::mutex m_mutex;
  std::condition_variable m_cv;
};
} // namespace FIX

#endif
