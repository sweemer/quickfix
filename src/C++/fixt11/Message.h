#ifndef FIXT11_MESSAGES_H
#define FIXT11_MESSAGES_H

#include "../Message.h"
#include "../Group.h"

namespace FIXT11
{
  using Header = FIX::Header;
  using Trailer = FIX::Trailer;

  class Message : public FIX::Message
  {
  public:
    Message( const FIX::MsgType& msgtype )
    : FIX::Message(
      FIX::BeginString("FIXT.1.1"), msgtype )
     {} 

    Message(const FIX::Message& m) : FIX::Message(m) {}
    Message(const Message& m) = default;
    Message(Message&& m) = default;
    Message& operator=(Message&&) = default;
    Message& operator=(const Message&) = default;
    Header& getHeader() { return m_header; }
    const Header& getHeader() const { return m_header; }
    Trailer& getTrailer() { return m_trailer; }
    const Trailer& getTrailer() const { return m_trailer; }
  };

}

#endif
