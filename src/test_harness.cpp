#include "Context.hpp"
#include <memory>

using int_ctx_t = CMD2::Context<int>;

class Increment : public int_ctx_t::Command {
   public:
      Increment() = default;
      ~Increment() override = default;
      void operator()(int_ctx_t& ctx) override
      {
         ctx.get_state() += 1;
      }
};

class Stop : public int_ctx_t::Command {
   public:
     void operator()(int_ctx_t& ctx) override
     {
        stop(ctx);
     }
};

int main(int, char **)
{
   using ::std::shared_ptr;
   using ::std::make_shared;

   int_ctx_t test_ctx{0};
   test_ctx.queue_to_context().enqueue(make_shared<Increment>());
}
