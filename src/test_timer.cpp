#include "Context.hpp"
#include <cstdint>
#include <vector>
#include <utility>
#include <memory>
#include <chrono>
#include <iostream>

int main(int, char **);

class TimerState {
 public:
   using tick_t = ::std::uint64_t;
   using time_point_t = ::std::chrono::steady_clock::time_point;
   class Tick;
   friend class Tick;
   friend int main(int, char **);

 private:
   using cmd_ptr_t = ::std::shared_ptr<void>;
   tick_t tick_count_ = 0;
   using log_entry_t = ::std::pair<time_point_t, cmd_ptr_t>;
   ::std::vector<log_entry_t> commandlog_;
};

using ticker_ctx_t = ::CMD2::Context<TimerState>;  // a -> b -> c _> a   d -> c

// A Command that actually executes the tick on the TimerState.
class TimerState::Tick : public ticker_ctx_t::Command, public ::std::enable_shared_from_this<Tick> {
   public:
      Tick() = default;
      ~Tick() override = default;
      void operator()(ticker_ctx_t &ctx) override
      {
         auto &state = ctx.get_state();
         state.tick_count_++;
         auto me = shared_from_this();
         state.commandlog_.emplace_back(::std::chrono::steady_clock::now(), me);
      }
};

class Stop : public ticker_ctx_t::Command {
   public:
      Stop() {
         ++num_instances_;
      }
      ~Stop() override {
         --num_instances_;
      }
      void operator()(ticker_ctx_t& ctx) override
      {
         stop(ctx);
      }

      static ::std::atomic<int> num_instances_;
};

::std::atomic<int> Stop::num_instances_ = 0;

// Holds the context for a thread that sends Tick Commands to the ticker_ctx_t
class Ticker {
 public:
   // The context must live while the Ticker is alive.
   Ticker(::std::uint32_t microseconds, ticker_ctx_t &ctx)
      : microseconds_(microseconds),
      ctx_(ctx), last_tick_(::std::chrono::steady_clock::now()),
      thread_([this](::std::stop_token stop_token)
   {
         this->run(stop_token);
   })
   {
   }
   ~Ticker()
   {
      thread_.request_stop();
   };

   void run(::std::stop_token stop_token)
   {
      while (!stop_token.stop_requested()) {
         auto now = ::std::chrono::steady_clock::now();
         auto diff = ::std::chrono::duration_cast<::std::chrono::microseconds>(now - last_tick_);
         if (diff < microseconds_) {
            auto const remaining_time = microseconds_ - diff;
            ::std::this_thread::sleep_for(remaining_time);
         }
         last_tick_ += microseconds_;
         ctx_.queue_to_context().enqueue(::std::make_shared<TimerState::Tick>());
      }
   }
   void stop()
   {
      thread_.request_stop();
   }

 private:
   ::std::chrono::microseconds microseconds_;
   ticker_ctx_t &ctx_;
   ::std::chrono::steady_clock::time_point last_tick_;
   ::std::jthread thread_;
};

int main(int, char **)
{
   using ::std::shared_ptr;
   using ::std::make_shared;
   using ::std::cout;
   ticker_ctx_t test_ctx;

   ::std::jthread context_thread{&ticker_ctx_t::run, &test_ctx};

   Ticker ticker(100000, test_ctx);

   ::std::this_thread::sleep_for(::std::chrono::seconds(10));

   ticker.stop();

   test_ctx.queue_to_context().enqueue(::std::make_shared<Stop>());
   context_thread.join();
   ::std::cout << "Tick count: " << test_ctx.get_state().tick_count_ << '\n';
   for (auto const& entry : test_ctx.get_state().commandlog_) {
      ::std::cout << "At: " << entry.first.time_since_epoch()
                  << " Command: " << entry.second.get() << '\n';
   }
   return 0;
}