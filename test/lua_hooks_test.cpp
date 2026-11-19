#include <filesystem>
#include <string_view>

#include <gtest/gtest.h>
#include <sol/sol.hpp>

#include "cursor.h"
#include "dead.h"
#include "diablo.h"
#include "engine/assets.hpp"
#include "engine/sound.h"
#include "game_mode.hpp"
#include "headless_mode.hpp"
#include "inv.h"
#include "items.h"
#include "levels/dun_tile_data.hpp"
#include "levels/gendung.h"
#include "lua/lua_event.hpp"
#include "lua/lua_global.hpp"
#include "monster.h"
#include "options.h"
#include "player.h"
#include "sound_effect_enums.h"
#include "storm/storm_net.hpp"
#include "tables/itemdat.h"
#include "tables/questdat.hpp"
#include "utils/paths.h"

namespace devilution {
namespace {

constexpr const char MissingMpqAssetsSkipReason[] = "MPQ assets (spawn.mpq or DIABDAT.MPQ) not found - skipping test suite";

class LuaHooksTest : public ::testing::Test {
public:
	static void SetUpTestSuite()
	{
		LoadCoreArchives();
		LoadGameArchives();
		missingMpqAssets_ = !HaveMainData();
		if (missingMpqAssets_)
			return;

		// Use an empty config and pref folder so that no installed or enabled mods are loaded.
		const std::filesystem::path folder = std::filesystem::path(::testing::TempDir()) / "lua_hooks_test";
		std::filesystem::remove_all(folder);
		std::filesystem::create_directories(folder);
		paths::SetPrefPath(folder.string());
		paths::SetConfigPath(folder.string());
		InitKeymapActions();
		LoadOptions();

		savedHeadlessMode_ = HeadlessMode;
		savedIsMultiplayer_ = gbIsMultiplayer;
		HeadlessMode = true;
	}

	static void TearDownTestSuite()
	{
		HeadlessMode = savedHeadlessMode_;
		gbIsMultiplayer = savedIsMultiplayer_;
	}

	void SetUp() override
	{
		if (missingMpqAssets_) {
			GTEST_SKIP() << MissingMpqAssetsSkipReason;
		}
		Players.resize(1);
		MyPlayer = &Players[0];
		InspectPlayer = MyPlayer;
		gbIsMultiplayer = false;
		gbSoundOn = false;
		savedDiabloQuestState_ = Quests[Q_DIABLO]._qactive;
		LuaInitialize();
	}

	void TearDown() override
	{
		if (missingMpqAssets_)
			return;
		LuaShutdown();
		Quests[Q_DIABLO]._qactive = savedDiabloQuestState_;
	}

protected:
	/** Runs a chunk the way a mod's init.lua runs and returns its result. */
	static sol::object RunLua(std::string_view code)
	{
		sol::state &lua = GetLuaState();
		const sol::environment env = CreateLuaSandbox();
		return SafeCallResult(lua.safe_script(code, env, sol::script_pass_on_error), /*optional=*/false);
	}

private:
	static inline bool missingMpqAssets_ = false;
	static inline bool savedHeadlessMode_ = false;
	static inline bool savedIsMultiplayer_ = false;
	quest_state savedDiabloQuestState_ = QUEST_NOTAVAIL;
};

TEST_F(LuaHooksTest, NothingIsCancelledWithoutHandlers)
{
	const Item item {};
	EXPECT_FALSE(lua::OnItemUse(MyPlayer, &item));
	EXPECT_FALSE(lua::OnDiabloDefeated(nullptr, false));
}

TEST_F(LuaHooksTest, OnItemUseIsCancelledIfAnyHandlerReturnsTrue)
{
	const sol::table state = RunLua(R"lua(
		local events = require("devilutionx.events")
		local state = { calls = 0 }
		events.OnItemUse.add(function(player, item) return item.IDidx == 42 end)
		events.OnItemUse.add(function(player, item) state.calls = state.calls + 1 end)
		return state
	)lua");

	Item item {};
	item.IDidx = static_cast<_item_indexes>(42);
	EXPECT_TRUE(lua::OnItemUse(MyPlayer, &item));
	item.IDidx = static_cast<_item_indexes>(43);
	EXPECT_FALSE(lua::OnItemUse(MyPlayer, &item));
	// Every handler runs, even after one has cancelled.
	EXPECT_EQ(state.get<int>("calls"), 2);
}

TEST_F(LuaHooksTest, CancelledItemUseDoesNotConsumeTheItem)
{
	Player &player = *MyPlayer;
	player = {};
	player._pMaxHP = player._pMaxHPBase = Fixed26_6::fromInt(100);
	player._pHitPoints = player._pHPBase = Fixed26_6::fromInt(10);
	InitializeItem(player.InvList[0], IDI_HEAL);
	player._pNumInv = 1;
	pcurs = CURSOR_HAND;

	RunLua(R"lua(
		local events = require("devilutionx.events")
		local items = require("devilutionx.items")
		events.OnItemUse.add(function(player, item) return item.IDidx == items.ItemIndex.Heal end)
	)lua");
	EXPECT_TRUE(UseInvItem(INVITEM_INV_FIRST));
	EXPECT_EQ(player._pNumInv, 1);
	EXPECT_EQ(player._pHitPoints, Fixed26_6::fromInt(10));

	// Without a cancelling handler the potion is drunk as usual.
	LuaShutdown();
	LuaInitialize();
	EXPECT_TRUE(UseInvItem(INVITEM_INV_FIRST));
	EXPECT_EQ(player._pNumInv, 0);
}

TEST_F(LuaHooksTest, OnDiabloDefeatedReceivesItsArguments)
{
	const sol::table state = RunLua(R"lua(
		local events = require("devilutionx.events")
		local state = {}
		events.OnDiabloDefeated.add(function(diablo, isLocalKill)
		  state.diabloIsNil = diablo == nil
		  state.isLocalKill = isLocalKill
		  return true
		end)
		return state
	)lua");

	EXPECT_TRUE(lua::OnDiabloDefeated(nullptr, false));
	EXPECT_TRUE(state.get<bool>("diabloIsNil"));
	EXPECT_FALSE(state.get<bool>("isLocalKill"));
}

TEST_F(LuaHooksTest, LoadingAGameAfterDiablosDefeatEndsItUnlessAModKeepsItRunning)
{
	Quests[Q_DIABLO]._qactive = QUEST_DONE;
	EXPECT_FALSE(IsDiabloAlive(false));

	RunLua(R"lua(
		require("devilutionx.events").OnDiabloDefeated.add(function() return true end)
	)lua");
	EXPECT_TRUE(IsDiabloAlive(false));
}

TEST_F(LuaHooksTest, GameModuleReportsQuestState)
{
	Quests[Q_DIABLO]._qactive = QUEST_ACTIVE;
	EXPECT_EQ(RunLua(R"lua(return require("devilutionx.game").QuestID.DIABLO)lua").as<int>(), static_cast<int>(Q_DIABLO));
	EXPECT_FALSE(RunLua(R"lua(local game = require("devilutionx.game") return game.isQuestDone(game.QuestID.DIABLO))lua").as<bool>());
	Quests[Q_DIABLO]._qactive = QUEST_DONE;
	EXPECT_TRUE(RunLua(R"lua(local game = require("devilutionx.game") return game.isQuestDone(game.QuestID.DIABLO))lua").as<bool>());
	EXPECT_FALSE(RunLua(R"lua(return require("devilutionx.game").isQuestDone(-1))lua").as<bool>());
}

TEST_F(LuaHooksTest, PlayerModuleExposesHeroSpeech)
{
	EXPECT_EQ(RunLua(R"lua(return require("devilutionx.player").HeroSpeech.ICantUseThisYet)lua").as<int>(),
	    static_cast<int>(HeroSpeech::ICantUseThisYet));
}

TEST_F(LuaHooksTest, GetItemIndexFindsItemsByMappingId)
{
	const sol::table result = RunLua(R"lua(
		local items = require("devilutionx.items")
		items.addItem({ mappingId = 900001, name = "Test Stone", type = items.ItemType.Misc, class = items.ItemClass.Quest })
		return { found = items.getItemIndex(900001), missing = items.getItemIndex(900002) }
	)lua");

	EXPECT_EQ(result.get<int>("found"), static_cast<int>(AllItemsList.size()) - 1);
	EXPECT_EQ(result.get<sol::object>("missing").get_type(), sol::type::lua_nil);
}

class DiabloDeathTest : public LuaHooksTest {
public:
	void SetUp() override
	{
		LuaHooksTest::SetUp();
		if (IsSkipped())
			return;
		savedCurrlevel_ = currlevel;
		savedLeveltype_ = leveltype;
		currlevel = 16;
		leveltype = DTYPE_HELL;
		setlevel = false;
		gbRunGame = true;
		gbProcessPlayers = true;
		cineflag = false;
		Quests[Q_DIABLO]._qactive = QUEST_ACTIVE;
		SNetInitializeProvider(SELCONN_LOOPBACK, nullptr);

		Player &player = *MyPlayer;
		player = {};
		player.setLevel(16);
		player.position.tile = { 40, 40 };

		InitItems();
		InitLevelMonsters();
		// Like a real level: the first monster slots are reserved for golems.
		ASSERT_TRUE(AddMonsterType(MT_GOLEM, PLACE_SPECIAL).has_value());
		InitGolems();
		const auto typeIndex = AddMonsterType(MT_DIABLO, PLACE_SPECIAL);
		ASSERT_TRUE(typeIndex.has_value()) << typeIndex.error();
		diablo_ = AddMonster({ 50, 50 }, Direction::South, *typeIndex, /*inMap=*/true);
		ASSERT_NE(diablo_, nullptr);
		InitCorpses();
	}

	void TearDown() override
	{
		if (!IsSkipped()) {
			currlevel = savedCurrlevel_;
			leveltype = savedLeveltype_;
			gbRunGame = false;
			gbProcessPlayers = true;
			cineflag = false;
		}
		LuaHooksTest::TearDown();
	}

protected:
	/** Kills Diablo and plays out his death sequence. */
	void KillDiablo()
	{
		M_StartKill(*diablo_, *MyPlayer);
		for (int tick = 0; tick < 300 && gbRunGame && !diablo_->isInvalid; tick++)
			ProcessMonsters();
	}

	Monster *diablo_ = nullptr;

private:
	uint8_t savedCurrlevel_ = 0;
	dungeon_type savedLeveltype_ = DTYPE_TOWN;
};

TEST_F(DiabloDeathTest, EndsTheGameWithoutHandlers)
{
	M_StartKill(*diablo_, *MyPlayer);
	EXPECT_EQ(Quests[Q_DIABLO]._qactive, QUEST_DONE);
	EXPECT_FALSE(gbProcessPlayers);

	KillDiablo();
	EXPECT_FALSE(gbRunGame);
	EXPECT_TRUE(cineflag);
}

TEST_F(DiabloDeathTest, KeepsTheGameRunningIfAHandlerSaysSo)
{
	const sol::table state = RunLua(R"lua(
		local events = require("devilutionx.events")
		local game = require("devilutionx.game")
		local items = require("devilutionx.items")
		local state = {}
		events.OnDiabloDefeated.add(function(diablo, isLocalKill)
		  state.gotDiablo = diablo ~= nil
		  state.isLocalKill = isLocalKill
		  state.questDone = game.isQuestDone(game.QuestID.DIABLO)
		  state.spawned = items.spawnQuestItem(items.ItemIndex.Heal, diablo.position, false)
		  return true
		end)
		return state
	)lua");

	const size_t itemsBefore = ActiveItemCount;
	KillDiablo();

	EXPECT_TRUE(state.get<bool>("gotDiablo"));
	EXPECT_TRUE(state.get<bool>("isLocalKill"));
	EXPECT_TRUE(state.get<bool>("questDone"));
	EXPECT_TRUE(state.get<bool>("spawned"));
	EXPECT_GT(ActiveItemCount, itemsBefore);

	// No ending sequence: players keep moving and Diablo leaves a corpse like any other monster.
	EXPECT_TRUE(gbProcessPlayers);
	EXPECT_TRUE(gbRunGame);
	EXPECT_FALSE(cineflag);
	EXPECT_TRUE(diablo_->isInvalid);
	EXPECT_EQ(dMonster[50][50], 0);
	EXPECT_NE(dCorpse[50][50], 0);
}

} // namespace
} // namespace devilution
