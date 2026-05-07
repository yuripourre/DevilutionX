#include "lua/modules/game.hpp"

#include <algorithm>
#include <unordered_set>
#include <vector>

#include <sol/sol.hpp>

#include "effects.h"
#include "engine/points_in_rectangle_range.hpp"
#include "inv.h"
#include "items.h"
#include "lua/metadoc.hpp"
#include "missiles.h"
#include "monster.h"
#include "multi.h"
#include "qol/stash.h"
#include "quests.h"
#include "tables/itemdat.h"
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

	LuaSetDocFn(table, "addResurrectBeamAt", "(x: integer, y: integer)",
	    "Spawns a ResurrectBeam visual effect at (x, y). Visual only, no caster.",
	    [](int x, int y) {
		    LuaAddResurrectBeamAt(x, y);
	    });

	LuaSetDocFn(table, "invalidateMonster", "(monster: Monster)",
	    "Removes the monster from the map and marks it invalid. Call when Lua handles death (e.g. no corpse).",
	    [](const Monster &monster) {
		    LuaInvalidateMonster(const_cast<Monster &>(monster));
	    });

	LuaSetDocFn(table, "replaceMonsterWithWoundedTowner", "(monster: Monster)",
	    "Replaces the monster's sprite with the wounded towner (dead body graphic), frozen at frame 0.",
	    [](const Monster &monster) {
		    LuaReplaceMonsterWithWoundedTowner(const_cast<Monster &>(monster));
	    });

	LuaSetDocFn(table, "toggleCube", "()",
	    "Opens the Cube (12-slot container) if closed, closes it if open.",
	    []() { ToggleReservedStash(); });

	LuaSetDocFn(table, "getCubeContents", "() -> table",
	    "Returns array of {miscId, curs, name} for each item in the Cube.",
	    [](sol::this_state s) {
		    sol::state_view lua(s);
		    sol::table result = lua.create_table();
		    auto it = Stash.stashGrids.find(ReservedStashPage);
		    if (it == Stash.stashGrids.end()) return result;
		    std::unordered_set<uint16_t> seen;
		    int idx = 1;
		    for (int x = 0; x < ReservedGridSize.width; x++) {
			    for (int y = 0; y < ReservedGridSize.height; y++) {
				    const uint16_t cell = it->second[x][y];
				    if (!cell || !seen.insert(cell - 1).second) continue;
				    const Item &item = Stash.stashList[cell - 1];
				    result[idx++] = lua.create_table_with(
				        "miscId", static_cast<int>(item._iMiscId),
				        "curs", item._iCurs,
				        "name", item._iIName);
			    }
		    }
		    return result;
	    });

	LuaSetDocFn(table, "clearCube", "()",
	    "Removes all items from the Cube (stash page 99).",
	    []() {
		    auto it = Stash.stashGrids.find(ReservedStashPage);
		    if (it == Stash.stashGrids.end()) return;
		    std::vector<uint16_t> toRemove;
		    for (int x = 0; x < ReservedGridSize.width; x++) {
			    for (int y = 0; y < ReservedGridSize.height; y++) {
				    const uint16_t cell = it->second[x][y];
				    if (cell) toRemove.push_back(cell - 1);
			    }
		    }
		    std::sort(toRemove.begin(), toRemove.end(), std::greater<uint16_t>());
		    toRemove.erase(std::unique(toRemove.begin(), toRemove.end()), toRemove.end());
		    for (uint16_t id : toRemove) Stash.RemoveStashItem(id);
	    });

	LuaSetDocFn(table, "getItemIndexByMiscId", "(miscId: integer) -> integer|nil",
	    "Returns first AllItemsList index whose iMiscId matches, or nil.",
	    [](int miscId) -> sol::optional<int> {
		    for (size_t i = 0; i < AllItemsList.size(); i++) {
			    if (static_cast<int>(AllItemsList[i].iMiscId) == miscId)
				    return static_cast<int>(i);
		    }
		    return sol::nullopt;
	    });

	LuaSetDocFn(table, "placeItemInCube", "(itemIdx: integer) -> boolean",
	    "Creates an item from AllItemsList and places it in the first free slot of the Cube. Returns true on success.",
	    [](int itemIdx) -> bool {
		    auto &grid = Stash.stashGrids[ReservedStashPage];
		    int slotX = -1, slotY = -1;
		    for (int y = 0; y < ReservedGridSize.height && slotX == -1; y++) {
			    for (int x = 0; x < ReservedGridSize.width && slotX == -1; x++) {
				    if (grid[x][y] == 0) { slotX = x; slotY = y; }
			    }
		    }
		    if (slotX == -1) return false;

		    Item newItem;
		    GetItemAttrs(newItem, static_cast<_item_indexes>(itemIdx), 1);
		    SetupItem(newItem);
		    const Size itemSize = GetInventorySize(newItem);
		    newItem.position = Point { slotX, slotY + itemSize.height - 1 };

		    Stash.stashList.push_back(newItem);
		    const auto stashIndex = static_cast<uint16_t>(Stash.stashList.size() - 1);
		    for (const Point pt : PointsInRectangle(Rectangle { Point { slotX, slotY }, itemSize })) {
			    if (pt.x < ReservedGridSize.width && pt.y < ReservedGridSize.height)
				    grid[pt.x][pt.y] = stashIndex + 1;
		    }
		    Stash.dirty = true;
		    Stash.RefreshItemStatFlags();
		    PlaySFX(GetItemInvSnd(Stash.stashList[stashIndex]));
		    return true;
	    });

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
