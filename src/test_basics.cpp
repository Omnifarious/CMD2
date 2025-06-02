#include "Context.hpp"
#include <memory>
#include <thread>
#include <iostream>
#include <atomic>

using int_ctx_t = skythedragon::ACE::Context<int>;

class Increment : public int_ctx_t::Command {
   public:
      Increment() {
         ++num_instances_;
      }
      ~Increment() override {
         --num_instances_;
      }
      void operator()(int_ctx_t& ctx) override
      {
         ctx.get_state() += 1;
      }

      static ::std::atomic<int> num_instances_;
};

class Stop : public int_ctx_t::Command {
   public:
     Stop() {
        ++num_instances_;
     }
     ~Stop() override {
        --num_instances_;
     }
     void operator()(int_ctx_t& ctx) override
     {
        stop(ctx);
     }

   static ::std::atomic<int> num_instances_;
};

::std::atomic<int> Increment::num_instances_ = 0;
::std::atomic<int> Stop::num_instances_ = 0;

int main(int, char **)
{
   using ::std::shared_ptr;
   using ::std::make_shared;
   using ::std::cout;

   int_ctx_t test_ctx{0};
   test_ctx.queue_to_context().enqueue(make_shared<Increment>());

   cout << "Before state: " << test_ctx.get_state() << '\n';
   cout << "Num increment instances: " << Increment::num_instances_ << '\n';
   ::std::jthread context_thread{&int_ctx_t::run, &test_ctx};
   test_ctx.queue_to_context().enqueue(make_shared<Stop>());
   context_thread.join();
   cout << " After state: " << test_ctx.get_state() << '\n';
   cout << "Num increment instances: " << Increment::num_instances_ << '\n';
   cout << "Num stop instances: " << Stop::num_instances_ << '\n';
   return 0;
}
