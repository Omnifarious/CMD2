
#include "../src/Context.hpp"
#include <unordered_map>

#define STATE_TYPE int

namespace skythedragon::CMD2 {
   using ACE::Context;

   struct State {
      STATE_TYPE game_state;
   };

   using commandfunc_t = ::std::function<void(State&)>;
   class Command : public Context<State>::Command {
      commandfunc_t func;

      void operator()(Context<State>& ctx) override
      {
         func(ctx.get_state());
      }
   };

   ::std::unordered_map<::std::string, commandfunc_t> commands;
}