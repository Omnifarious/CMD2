#include "Context.hpp"
#include <cstdint>
#include <vector>
#include <utility>
#include <memory>
#include <chrono>

class TimerState {
 public:
   using tick_t = ::std::uint64_t;
   class Tick;
   friend class Tick;

 private:
   using cmd_ptr_t = ::std::shared_ptr<void>;
   tick_t tick_count_;
   using log_entry_t = ::std::pair<tick_t, cmd_ptr_t>;
   ::std::vector<log_entry_t> commandlog_;
};

using ticker_ctx_t = ::CMD2::Context<TimerState>;

// A Command that actually executes the tick on the TimerState.
class TimerState::Tick : public ticker_ctx_t::Command, ::std::enable_shared_from_this<Tick> {
   public:
      Tick() = default;
      ~Tick() override = default;
      void operator()(ticker_ctx_t &ctx) override
      {
         auto &state = ctx.get_state();
         state.tick_count_++;
         state.commandlog_.emplace_back(ctx.get_state().tick_count_, shared_from_this());
      }
};


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
