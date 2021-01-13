#pragma once

#include "event.h"


class Presenter {
  public:
    virtual void show(Event* event) = 0;

    virtual ~Presenter() = default;
};

class NoPresenter: public Presenter {
  public:
    void show(Event*) override {}
};
