#include "Context.hpp"
#include <cstdint>
#include <vector>
#include <utility>
#include <memory>
#include <chrono>
#include <iostream>

int main(int, char **);
class TimerState;
using ticker_ctx_t = ::skythedragon::ACE::Context<TimerState>;

static void tick(ticker_ctx_t& ctx);

class TimerState {
 public:
   using tick_t = ::std::uint64_t;
   using time_point_t = ::std::chrono::steady_clock::time_point;
   class Tick;
   friend class Tick;
   friend int main(int, char **);
   friend void tick(ticker_ctx_t& ctx);

 private:
   using cmd_ptr_t = ::std::shared_ptr<void>;
   tick_t tick_count_ = 0;
   ::std::vector<time_point_t> commandlog_;
};

using ticker_ctx_t = ::skythedragon::ACE::Context<TimerState>;  // a -> b -> c _> a   d -> c

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

static void tick(ticker_ctx_t& ctx)
{
   auto &state = ctx.get_state();
   state.tick_count_++;
   state.commandlog_.push_back(std::chrono::steady_clock::now());
}

::std::atomic<int> Stop::num_instances_ = 0;

int main(int, char **)
{
   using ::std::shared_ptr;
   using ::std::make_shared;
   using ::std::cout;
   ticker_ctx_t test_ctx(100'000, tick);

   ::std::jthread context_thread{&ticker_ctx_t::run, &test_ctx};

   test_ctx.start_ticker();

   ::std::this_thread::sleep_for(::std::chrono::seconds(10));

   test_ctx.queue_to_context().enqueue(::std::make_shared<Stop>());
   context_thread.join();
   ::std::cout << "Tick count: " << test_ctx.get_state().tick_count_ << '\n';
   for (auto const& entry : test_ctx.get_state().commandlog_) {
      ::std::cout << "At: " << entry.time_since_epoch() << '\n';
   }

   return 0;
}