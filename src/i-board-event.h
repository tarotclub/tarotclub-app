#ifndef IBOARDEVENT_H
#define IBOARDEVENT_H

#include "Card.h"
#include <string>

class IBoardEvent
{
public:
    virtual ~IBoardEvent() {}

    virtual void ChatMessage(const std::string &msg) = 0;
    virtual void SendMyBid() = 0;
    virtual void SendMyCard(const Card &c) = 0;
    virtual void ConfigChanged() = 0;
    virtual void ExitGame() = 0;
    virtual void ClickOnBoard() = 0;
};


#endif // IBOARDEVENT_H
