#pragma once

#include <string_view>

#include <expected.hpp>
#include <function_ref.hpp>
#include <sol/forward.hpp>

namespace devilution {

struct Item;
struct Player;
struct Monster;

void LuaInitialize();
void LuaReloadActiveMods();
void LuaShutdown();

/**
 * @brief Fires a cancellable Lua event.
 * @return true if any handler returned true (i.e. default behaviour should be cancelled).
 */
bool LuaEventCancellable(std::string_view name, const Player *player, const Item *item);
/**
 * @brief Fires a cancellable Lua event for monster death.
 * @return true if any handler returned true (i.e. default corpse/invalidate should be skipped).
 */
bool LuaEventCancellable(std::string_view name, const Monster *monster, int arg1);
sol::state &GetLuaState();
sol::environment CreateLuaSandbox();
sol::object SafeCallResult(sol::protected_function_result result, bool optional);
sol::table *GetLuaEvents();

/** Adds a handler to be called when mods status changes after the initial startup. */
void AddModsChangedHandler(tl::function_ref<void()> callback);

} // namespace devilution
