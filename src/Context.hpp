#pragma once

#include "../concurrentqueue/concurrentqueue.h"
#include <memory>

namespace CMD2 {

using moodycamel::ConcurrentQueue;

template <typename State>
class Context {
  public:
     class Command
     {
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
        if (incoming_commands_.try_dequeue(command)) {
          (*command)(*this);
        }
      }
    }

    State &get_state() { return state_; }

  private:
    State state_;
    ConcurrentQueue<::std::shared_ptr<Command>> incoming_commands_;
    bool should_stop_ = false;
};

}
