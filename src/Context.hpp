#pragma once

#include "../concurrentqueue/blockingconcurrentqueue.h"
#include <memory>
#include <functional>

namespace skythedragon::ACE {

using moodycamel::BlockingConcurrentQueue;

template <typename State>
class Context {
  public:
    using tickfunc_t = ::std::function<void(Context<State>&)>;
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

    class Ticker {
     public:
       class Tick : public Command {
         public:
           explicit Tick(const tickfunc_t& tickfunc) : tickfunc_(tickfunc) {}
           ~Tick() override = default;
           void operator()(Context& context) override
           {
              tickfunc_(context);
           }
         private:
           const tickfunc_t& tickfunc_;
       };

       Ticker(::std::uint32_t microseconds, Context& ctx, const tickfunc_t& tickfunc) :
            microseconds_(microseconds),
            ctx_(ctx),
            last_tick_(::std::chrono::steady_clock::now()),
            thread_([this](::std::stop_token stop_token) {
              this->run(stop_token);
            }),
            tickfunc_(tickfunc)
      {}
      ~Ticker()
      {
         thread_.request_stop();
      }

      void run(::std::stop_token stop_token)
      {
        while (!stop_token.stop_requested()) {
          auto now = std::chrono::steady_clock::now();
          auto  diff = ::std::chrono::duration_cast<::std::chrono::microseconds>(now - last_tick_);
          if (diff < microseconds_) {
            auto const remaing_time = microseconds_ - diff;
            ::std::this_thread::sleep_for(remaing_time);
          }
          last_tick_ += microseconds_;

          //TODO add a better pausing method for when a problem finally crops up
          if (!paused_) ctx_.queue_to_context().enqueue(::std::make_shared<Tick>(tickfunc_));
        }
      }

      public:
        void start()
        {
          paused_ = false;
        }

        void stop()
        {
          paused_ = true;
        }

      private:
        std::atomic<bool> paused_ = true;
        ::std::chrono::microseconds microseconds_;
        Context& ctx_;
        ::std::chrono::steady_clock::time_point last_tick_;
        ::std::jthread thread_;
        tickfunc_t tickfunc_;
    };

    //TODO add logging system

    Context() = default;
    explicit Context(State state) : state_(::std::move(state)) {}
    explicit Context(State state, ::std::uint32_t microseconds, tickfunc_t tickfunc) : state_(::std::move(state))
    {
      ticker_ = ::std::make_unique<Ticker>(microseconds, *this, tickfunc);
    }
    explicit Context(::std::uint32_t microseconds, tickfunc_t tickfunc)
    {
      ticker_ = ::std::make_unique<Ticker>(microseconds, *this, tickfunc);
    }

    auto &queue_to_context() { return incoming_commands_; };

    void run()
    {
      while (!should_stop_) {
        ::std::shared_ptr<Command> command;
        incoming_commands_.wait_dequeue(command);
        (*command)(*this);
      }
    }

    void start_ticker()
    {
      if (!ticker_) {
        //TODO: log error
        return;
      }

      ticker_->start();
    }

    void stop_ticker()
    {
      if (!ticker_) {
        //TODO log error
        return;
      }

      ticker_->stop();
    }

    State &get_state() { return state_; }

  private:
    std::shared_ptr<Ticker> ticker_;
    State state_;
    BlockingConcurrentQueue<::std::shared_ptr<Command>> incoming_commands_;
    bool should_stop_ = false;
};

}
