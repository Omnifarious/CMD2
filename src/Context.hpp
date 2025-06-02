#pragma once

#include "../concurrentqueue/blockingconcurrentqueue.h"
#include <memory>

namespace skythedragon::ACE {

using moodycamel::BlockingConcurrentQueue;

template <typename State>
class Context {
  public:
     class Command
     {
      public:
       Command() = default;
       virtual ~Command() = default;
       virtual void operator()(Context &context) = 0;

       protected:
          void stop(Context &context) { context.should_stop_ = true; }
     };
     friend class Command;

    Context() = default;
    explicit Context(State state) : state_(std::move(state)) {}

    auto &queue_to_context() { return incoming_commands_; };

    void run()
    {
      while (!should_stop_) {
        ::std::shared_ptr<Command> command;
        incoming_commands_.wait_dequeue(command);
        (*command)(*this);
      }
    }

    State &get_state() { return state_; }

  private:
    State state_;
    BlockingConcurrentQueue<::std::shared_ptr<Command>> incoming_commands_;
    bool should_stop_ = false;
};

}
