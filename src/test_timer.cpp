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
class TimerState::Tick : public ticker_ctx_t::Command {};


// Holds the context for a thread that sends Tick Commands to the ticker_ctx_t
class Ticker {
 public:
   // The context must live while the Ticker is alive.
   Ticker(::std::uint32_t microseconds, ticker_ctx_t &ctx)
      : microseconds_(microseconds), ctx_(ctx) {
   }
   ~Ticker() {}

   void run()
   {}

 private:
   ::std::uint32_t microseconds_;
   ticker_ctx_t &ctx_;
   ::std::chrono::steady_clock::time_point last_tick_;
};
