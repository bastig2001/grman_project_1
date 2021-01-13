#pragma once

#include "event.h"


// The interface for a Presenter which shows events to the user
class Presenter {
  public:
    virtual void show(Event* event) = 0;

    virtual ~Presenter() = default;
};


// An implementation of Presenter which doesn't do anything
class NoPresenter: public Presenter {
  public:
    void show(Event*) override {}
};
