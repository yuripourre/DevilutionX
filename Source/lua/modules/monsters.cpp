#include "lua/modules/monsters.hpp"

#include <string_view>

#include <fmt/format.h>
#include <sol/sol.hpp>

#include "data/file.hpp"
#include "engine/point.hpp"
#include "lua/metadoc.hpp"
#include "monster.h"
#include "tables/monstdat.h"
#include "utils/language.h"
#include "utils/str_split.hpp"

namespace devilution {

namespace {

void AddMonsterDataFromTsv(const std::string_view path)
{
	DataFile dataFile = DataFile::loadOrDie(path);
	LoadMonstDatFromFile(dataFile, path, true);
}

void AddUniqueMonsterDataFromTsv(const std::string_view path)
{
	DataFile dataFile = DataFile::loadOrDie(path);
	LoadUniqueMonstDatFromFile(dataFile, path);
}

void InitPointUserType(sol::state_view &lua)
{
	sol::usertype<Point> pointType = lua.new_usertype<Point>(sol::no_constructor);
	pointType["x"] = &Point::x;
	pointType["y"] = &Point::y;
}

void InitMonsterUserType(sol::state_view &lua)
{
	sol::usertype<Monster> monsterType = lua.new_usertype<Monster>(sol::no_constructor);
	LuaSetDocReadonlyProperty(monsterType, "position", "Point",
	    "Monster's current position (readonly)",
	    [](const Monster &monster) {
		    return Point { monster.position.tile };
	    });
	LuaSetDocReadonlyProperty(monsterType, "id", "integer",
	    "Monster's unique ID (readonly)",
	    [](const Monster &monster) {
		    return static_cast<int>(reinterpret_cast<uintptr_t>(&monster));
	    });
	LuaSetDocReadonlyProperty(monsterType, "typeId", "integer",
	    "Monster type ID matching monsters.MonsterID constants (readonly)",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.type().type);
	    });
	LuaSetDocReadonlyProperty(monsterType, "name", "string",
	    "Monster's display name (readonly)",
	    [](const Monster &monster) -> std::string_view {
		    return monster.name();
	    });
}

} // namespace

sol::table LuaMonstersModule(sol::state_view &lua)
{
	InitPointUserType(lua);
	InitMonsterUserType(lua);
	sol::table table = lua.create_table();
	LuaSetDocFn(table, "addMonsterDataFromTsv", "(path: string)", AddMonsterDataFromTsv);
	LuaSetDocFn(table, "addUniqueMonsterDataFromTsv", "(path: string)", AddUniqueMonsterDataFromTsv);

	// Named constants for the most commonly referenced built-in monster types.
	// Values correspond to the _monster_id enum in tables/monstdat.h.
	table["MonsterID"] = lua.create_table_with(
	    "SkeletonKing", static_cast<int>(MT_SKING),
	    "Butcher", static_cast<int>(MT_CLEAVER),
	    "Golem", static_cast<int>(MT_GOLEM),
	    "Diablo", static_cast<int>(MT_DIABLO),
	    "DarkMage", static_cast<int>(MT_DARKMAGE),
	    "NaKrul", static_cast<int>(MT_NAKRUL));

	return table;
}

} // namespace devilution
