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

void RegisterMonsterFlagEnum(sol::state_view &lua)
{
	lua.new_enum<monster_flag>("MonsterFlag",
	    {
	        { "Hidden", MFLAG_HIDDEN },
	        { "LockAnimation", MFLAG_LOCK_ANIMATION },
	        { "AllowSpecial", MFLAG_ALLOW_SPECIAL },
	        { "TargetsMonster", MFLAG_TARGETS_MONSTER },
	        { "Golem", MFLAG_GOLEM },
	        { "QuestComplete", MFLAG_QUEST_COMPLETE },
	        { "Knockback", MFLAG_KNOCKBACK },
	        { "Search", MFLAG_SEARCH },
	        { "AllowOpenDoor", MFLAG_ALLOW_OPEN_DOOR },
	        { "NoEnemy", MFLAG_NO_ENEMY },
	        { "Berserk", MFLAG_BERSERK },
	        { "NoLifeSteal", MFLAG_NOLIFESTEAL },
	    });
}

void RegisterUniqueMonsterTypeEnum(sol::state_view &lua)
{
	lua.new_enum<UniqueMonsterType>("UniqueMonsterType",
	    {
	        { "Garbud", UniqueMonsterType::Garbud },
	        { "SkeletonKing", UniqueMonsterType::SkeletonKing },
	        { "Zhar", UniqueMonsterType::Zhar },
	        { "SnotSpill", UniqueMonsterType::SnotSpill },
	        { "Lazarus", UniqueMonsterType::Lazarus },
	        { "RedVex", UniqueMonsterType::RedVex },
	        { "BlackJade", UniqueMonsterType::BlackJade },
	        { "Lachdan", UniqueMonsterType::Lachdan },
	        { "WarlordOfBlood", UniqueMonsterType::WarlordOfBlood },
	        { "Butcher", UniqueMonsterType::Butcher },
	        { "HorkDemon", UniqueMonsterType::HorkDemon },
	        { "Defiler", UniqueMonsterType::Defiler },
	        { "NaKrul", UniqueMonsterType::NaKrul },
	        { "None", UniqueMonsterType::None },
	    });
}

void RegisterMonsterModeEnum(sol::state_view &lua)
{
	lua.new_enum<MonsterMode>("MonsterMode",
	    {
	        { "Stand", MonsterMode::Stand },
	        { "MoveNorthwards", MonsterMode::MoveNorthwards },
	        { "MoveSouthwards", MonsterMode::MoveSouthwards },
	        { "MoveSideways", MonsterMode::MoveSideways },
	        { "MeleeAttack", MonsterMode::MeleeAttack },
	        { "HitRecovery", MonsterMode::HitRecovery },
	        { "Death", MonsterMode::Death },
	        { "SpecialMeleeAttack", MonsterMode::SpecialMeleeAttack },
	        { "FadeIn", MonsterMode::FadeIn },
	        { "FadeOut", MonsterMode::FadeOut },
	        { "RangedAttack", MonsterMode::RangedAttack },
	        { "SpecialStand", MonsterMode::SpecialStand },
	        { "SpecialRangedAttack", MonsterMode::SpecialRangedAttack },
	        { "Delay", MonsterMode::Delay },
	        { "Charge", MonsterMode::Charge },
	        { "Petrified", MonsterMode::Petrified },
	        { "Heal", MonsterMode::Heal },
	        { "Talk", MonsterMode::Talk },
	    });
}

void RegisterMonsterGoalEnum(sol::state_view &lua)
{
	lua.new_enum<MonsterGoal>("MonsterGoal",
	    {
	        { "None", MonsterGoal::None },
	        { "Normal", MonsterGoal::Normal },
	        { "Retreat", MonsterGoal::Retreat },
	        { "Healing", MonsterGoal::Healing },
	        { "Move", MonsterGoal::Move },
	        { "Attack", MonsterGoal::Attack },
	        { "Inquiring", MonsterGoal::Inquiring },
	        { "Talking", MonsterGoal::Talking },
	    });
}

void RegisterLeaderRelationEnum(sol::state_view &lua)
{
	lua.new_enum<LeaderRelation>("LeaderRelation",
	    {
	        { "None", LeaderRelation::None },
	        { "Leashed", LeaderRelation::Leashed },
	        { "Separated", LeaderRelation::Separated },
	    });
}

void RegisterMonsterAIIDEnum(sol::state_view &lua)
{
	lua.new_enum<MonsterAIID>("MonsterAIID",
	    {
	        { "Zombie", MonsterAIID::Zombie },
	        { "Fat", MonsterAIID::Fat },
	        { "SkeletonMelee", MonsterAIID::SkeletonMelee },
	        { "SkeletonRanged", MonsterAIID::SkeletonRanged },
	        { "Scavenger", MonsterAIID::Scavenger },
	        { "Rhino", MonsterAIID::Rhino },
	        { "GoatMelee", MonsterAIID::GoatMelee },
	        { "GoatRanged", MonsterAIID::GoatRanged },
	        { "Fallen", MonsterAIID::Fallen },
	        { "Magma", MonsterAIID::Magma },
	        { "SkeletonKing", MonsterAIID::SkeletonKing },
	        { "Bat", MonsterAIID::Bat },
	        { "Gargoyle", MonsterAIID::Gargoyle },
	        { "Butcher", MonsterAIID::Butcher },
	        { "Succubus", MonsterAIID::Succubus },
	        { "Sneak", MonsterAIID::Sneak },
	        { "Storm", MonsterAIID::Storm },
	        { "FireMan", MonsterAIID::FireMan },
	        { "Gharbad", MonsterAIID::Gharbad },
	        { "Acid", MonsterAIID::Acid },
	        { "AcidUnique", MonsterAIID::AcidUnique },
	        { "Golem", MonsterAIID::Golem },
	        { "Zhar", MonsterAIID::Zhar },
	        { "Snotspill", MonsterAIID::Snotspill },
	        { "Snake", MonsterAIID::Snake },
	        { "Counselor", MonsterAIID::Counselor },
	        { "Mega", MonsterAIID::Mega },
	        { "Diablo", MonsterAIID::Diablo },
	        { "Lazarus", MonsterAIID::Lazarus },
	        { "LazarusSuccubus", MonsterAIID::LazarusSuccubus },
	        { "Lachdanan", MonsterAIID::Lachdanan },
	        { "Warlord", MonsterAIID::Warlord },
	        { "FireBat", MonsterAIID::FireBat },
	        { "Torchant", MonsterAIID::Torchant },
	        { "HorkDemon", MonsterAIID::HorkDemon },
	        { "Lich", MonsterAIID::Lich },
	        { "ArchLich", MonsterAIID::ArchLich },
	        { "Psychorb", MonsterAIID::Psychorb },
	        { "Necromorb", MonsterAIID::Necromorb },
	        { "BoneDemon", MonsterAIID::BoneDemon },
	        { "Invalid", MonsterAIID::Invalid },
	    });
}

void RegisterDirectionEnum(sol::state_view &lua)
{
	lua.new_enum<Direction>("Direction",
	    {
	        { "South", Direction::South },
	        { "SouthWest", Direction::SouthWest },
	        { "West", Direction::West },
	        { "NorthWest", Direction::NorthWest },
	        { "North", Direction::North },
	        { "NorthEast", Direction::NorthEast },
	        { "East", Direction::East },
	        { "SouthEast", Direction::SouthEast },
	        { "NoDirection", Direction::NoDirection },
	    });
}

void InitMonsterUserType(sol::state_view &lua)
{
	sol::usertype<Monster> monsterType = lua.new_usertype<Monster>(sol::no_constructor);

	LuaSetDocReadonlyProperty(monsterType, "position", "Point",
	    "Monster's current position",
	    [](const Monster &monster) {
		    return Point { monster.position.tile };
	    });
	LuaSetDocReadonlyProperty(monsterType, "id", "integer",
	    "Monster's unique ID",
	    [](const Monster &monster) {
		    return static_cast<int>(reinterpret_cast<uintptr_t>(&monster));
	    });
	LuaSetDocReadonlyProperty(monsterType, "hitPoints", "integer",
	    "Monster's current hit points",
	    [](const Monster &monster) {
		    return monster.hitPoints >> 6;
	    });
	LuaSetDocReadonlyProperty(monsterType, "maxHitPoints", "integer",
	    "Monster's maximum hit points",
	    [](const Monster &monster) {
		    return monster.maxHitPoints >> 6;
	    });
	LuaSetDocReadonlyProperty(monsterType, "armorClass", "integer",
	    "Monster's armor class",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.armorClass);
	    });
	LuaSetDocReadonlyProperty(monsterType, "resistance", "integer",
	    "Monster's damage type resistance bitmask",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.resistance);
	    });
	LuaSetDocReadonlyProperty(monsterType, "flags", "integer",
	    "Monster's flag bitmask (MonsterFlag values)",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.flags);
	    });
	LuaSetDocReadonlyProperty(monsterType, "minDamage", "integer",
	    "Monster's minimum melee damage",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.minDamage);
	    });
	LuaSetDocReadonlyProperty(monsterType, "maxDamage", "integer",
	    "Monster's maximum melee damage",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.maxDamage);
	    });
	LuaSetDocReadonlyProperty(monsterType, "minDamageSpecial", "integer",
	    "Monster's minimum special attack damage",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.minDamageSpecial);
	    });
	LuaSetDocReadonlyProperty(monsterType, "maxDamageSpecial", "integer",
	    "Monster's maximum special attack damage",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.maxDamageSpecial);
	    });
	LuaSetDocReadonlyProperty(monsterType, "direction", "Direction",
	    "Monster's facing direction",
	    [](const Monster &monster) {
		    return monster.direction;
	    });
	LuaSetDocReadonlyProperty(monsterType, "mode", "MonsterMode",
	    "Monster's current action mode",
	    [](const Monster &monster) {
		    return monster.mode;
	    });
	LuaSetDocReadonlyProperty(monsterType, "goal", "MonsterGoal",
	    "Monster's current AI goal",
	    [](const Monster &monster) {
		    return monster.goal;
	    });
	LuaSetDocReadonlyProperty(monsterType, "ai", "MonsterAIID",
	    "Monster's AI routine identifier",
	    [](const Monster &monster) {
		    return monster.ai;
	    });
	LuaSetDocReadonlyProperty(monsterType, "uniqueType", "UniqueMonsterType",
	    "Which unique monster this is, or UniqueMonsterType.None",
	    [](const Monster &monster) {
		    return monster.uniqueType;
	    });
	LuaSetDocReadonlyProperty(monsterType, "intelligence", "integer",
	    "Monster's AI aggressiveness level",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.intelligence);
	    });
	LuaSetDocReadonlyProperty(monsterType, "isInvalid", "boolean",
	    "Whether this monster slot is inactive",
	    [](const Monster &monster) {
		    return monster.isInvalid;
	    });
	LuaSetDocReadonlyProperty(monsterType, "packSize", "integer",
	    "Number of minions in this monster's pack",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.packSize);
	    });
	LuaSetDocReadonlyProperty(monsterType, "leader", "integer",
	    "Index of this monster's pack leader",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.leader);
	    });
	LuaSetDocReadonlyProperty(monsterType, "leaderRelation", "LeaderRelation",
	    "This monster's relationship to its pack leader",
	    [](const Monster &monster) {
		    return monster.leaderRelation;
	    });
	LuaSetDocReadonlyProperty(monsterType, "enemy", "integer",
	    "Index of this monster's current target",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.enemy);
	    });
	LuaSetDocReadonlyProperty(monsterType, "levelType", "integer",
	    "Index into LevelMonsterTypes for this monster's type",
	    [](const Monster &monster) {
		    return static_cast<int>(monster.levelType);
	    });

	LuaSetDocFn(monsterType, "name", "() -> string",
	    "Monster's translated display name",
	    [](const Monster &monster) {
		    return monster.name();
	    });
	LuaSetDocFn(monsterType, "exp", "(difficulty: integer) -> integer",
	    "Monster's experience point value for the given difficulty (0=Normal, 1=Nightmare, 2=Hell)",
	    [](const Monster &monster, int difficulty) {
		    return monster.exp(static_cast<_difficulty>(difficulty));
	    });
	LuaSetDocFn(monsterType, "level", "(difficulty: integer) -> integer",
	    "Monster's level for the given difficulty (0=Normal, 1=Nightmare, 2=Hell)",
	    [](const Monster &monster, int difficulty) {
		    return monster.level(static_cast<_difficulty>(difficulty));
	    });
	LuaSetDocFn(monsterType, "toHit", "(difficulty: integer) -> integer",
	    "Monster's melee chance-to-hit for the given difficulty (0=Normal, 1=Nightmare, 2=Hell)",
	    [](const Monster &monster, int difficulty) {
		    return monster.toHit(static_cast<_difficulty>(difficulty));
	    });
	LuaSetDocFn(monsterType, "toHitSpecial", "(difficulty: integer) -> integer",
	    "Monster's special attack chance-to-hit for the given difficulty (0=Normal, 1=Nightmare, 2=Hell)",
	    [](const Monster &monster, int difficulty) {
		    return monster.toHitSpecial(static_cast<_difficulty>(difficulty));
	    });
	LuaSetDocFn(monsterType, "isUnique", "() -> boolean",
	    "Whether this is a unique monster",
	    &Monster::isUnique);
	LuaSetDocFn(monsterType, "isPlayerMinion", "() -> boolean",
	    "Whether this monster is a player's golem",
	    &Monster::isPlayerMinion);
	LuaSetDocFn(monsterType, "hasNoLife", "() -> boolean",
	    "Whether this monster has zero or fewer hit points",
	    &Monster::hasNoLife);
	LuaSetDocFn(monsterType, "distanceToEnemy", "() -> integer",
	    "Distance in tiles to this monster's current target",
	    &Monster::distanceToEnemy);
}

} // namespace

sol::table LuaMonstersModule(sol::state_view &lua)
{
	RegisterMonsterFlagEnum(lua);
	RegisterUniqueMonsterTypeEnum(lua);
	RegisterMonsterModeEnum(lua);
	RegisterMonsterGoalEnum(lua);
	RegisterLeaderRelationEnum(lua);
	RegisterMonsterAIIDEnum(lua);
	RegisterDirectionEnum(lua);
	InitMonsterUserType(lua);

	sol::table table = lua.create_table();
	LuaSetDocFn(table, "addMonsterDataFromTsv", "(path: string)", AddMonsterDataFromTsv);
	LuaSetDocFn(table, "addUniqueMonsterDataFromTsv", "(path: string)", AddUniqueMonsterDataFromTsv);

	table["MonsterFlag"] = lua["MonsterFlag"];
	table["UniqueMonsterType"] = lua["UniqueMonsterType"];
	table["MonsterMode"] = lua["MonsterMode"];
	table["MonsterGoal"] = lua["MonsterGoal"];
	table["LeaderRelation"] = lua["LeaderRelation"];
	table["MonsterAIID"] = lua["MonsterAIID"];
	table["Direction"] = lua["Direction"];

	return table;
}

} // namespace devilution
