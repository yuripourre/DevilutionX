#include "lua/modules/game.hpp"

#include <sol/sol.hpp>

#include "lua/metadoc.hpp"
#include "monster.h"
#include "multi.h"
#include "quests.h"
#include "tables/objdat.h"

namespace devilution {

sol::table LuaGameModule(sol::state_view &lua)
{
	sol::table table = lua.create_table();

	LuaSetDocFn(table, "prepDoEnding", "()",
	    "Triggers the game-ending sequence (win condition). Safe to call in multiplayer.",
	    PrepDoEnding);

	LuaSetDocFn(table, "isQuestDone", "(questId: integer) -> boolean",
	    "Returns true if the quest with the given ID has been completed.",
	    [](int questId) {
		    return Quests[questId]._qactive == QUEST_DONE;
	    });

	LuaSetDocFn(table, "isMultiplayer", "() -> boolean",
	    "Returns true when running in a multiplayer session.",
	    []() { return gbIsMultiplayer; });

	// Named constants for quest IDs (values match the quest_id enum in tables/objdat.h).
	table["QuestID"] = lua.create_table_with(
	    "Rock", static_cast<int>(Q_ROCK),
	    "SkeletonKing", static_cast<int>(Q_SKELKING),
	    "Butcher", static_cast<int>(Q_BUTCHER),
	    "Ogden", static_cast<int>(Q_LTBANNER),
	    "Blood", static_cast<int>(Q_BLOOD),
	    "Blind", static_cast<int>(Q_BLIND),
	    "Mushroom", static_cast<int>(Q_MUSHROOM),
	    "Diablo", static_cast<int>(Q_DIABLO),
	    "Warlord", static_cast<int>(Q_WARLORD));

	return table;
}

} // namespace devilution
