#include <gtest/gtest.h>

#include "stores.h"

using namespace devilution;

namespace {

/** Helper to find the options vector for a towner by short name, or nullptr. */
std::vector<TownerDialogOption> *FindOptions(std::string_view name)
{
	for (auto &[key, opts] : ExtraTownerOptions) {
		if (key == name)
			return &opts;
	}
	return nullptr;
}

TEST(Stores, AddStoreHoldRepair_magic)
{
	devilution::Item *item;

	item = &PlayerItems[0];

	item->_iMaxDur = 60;
	item->_iDurability = item->_iMaxDur;
	item->_iMagical = ITEM_QUALITY_MAGIC;
	item->_iIdentified = true;
	item->_ivalue = 2000;
	item->_iIvalue = 19000;

	for (int i = 1; i < item->_iMaxDur; i++) {
		item->_ivalue = 2000;
		item->_iIvalue = 19000;
		item->_iDurability = i;
		CurrentItemIndex = 0;
		AddStoreHoldRepair(item, 0);
		EXPECT_EQ(1, CurrentItemIndex);
		EXPECT_EQ(95 * (item->_iMaxDur - i) / 2, item->_ivalue);
	}

	item->_iDurability = 59;
	CurrentItemIndex = 0;
	item->_ivalue = 500;
	item->_iIvalue = 30; // To cheap to repair
	AddStoreHoldRepair(item, 0);
	EXPECT_EQ(0, CurrentItemIndex);
	EXPECT_EQ(30, item->_iIvalue);
	EXPECT_EQ(500, item->_ivalue);
}

TEST(Stores, AddStoreHoldRepair_normal)
{
	devilution::Item *item;

	item = &PlayerItems[0];

	item->_iMaxDur = 20;
	item->_iDurability = item->_iMaxDur;
	item->_iMagical = ITEM_QUALITY_NORMAL;
	item->_iIdentified = true;
	item->_ivalue = 2000;
	item->_iIvalue = item->_ivalue;

	for (int i = 1; i < item->_iMaxDur; i++) {
		item->_ivalue = 2000;
		item->_iIvalue = item->_ivalue;
		item->_iDurability = i;
		CurrentItemIndex = 0;
		AddStoreHoldRepair(item, 0);
		EXPECT_EQ(1, CurrentItemIndex);
		EXPECT_EQ(50 * (item->_iMaxDur - i), item->_ivalue);
	}

	item->_iDurability = 19;
	CurrentItemIndex = 0;
	item->_ivalue = 10; // less than 1 per dur
	item->_iIvalue = item->_ivalue;
	AddStoreHoldRepair(item, 0);
	EXPECT_EQ(1, CurrentItemIndex);
	EXPECT_EQ(1, item->_ivalue);
	EXPECT_EQ(1, item->_iIvalue);
}

TEST(Stores, TownerNameForTalkID_knownTowners)
{
	EXPECT_STREQ(TownerNameForTalkID(TalkID::Smith), "griswold");
	EXPECT_STREQ(TownerNameForTalkID(TalkID::Witch), "adria");
	EXPECT_STREQ(TownerNameForTalkID(TalkID::Boy), "wirt");
	EXPECT_STREQ(TownerNameForTalkID(TalkID::Healer), "pepin");
	EXPECT_STREQ(TownerNameForTalkID(TalkID::Storyteller), "cain");
	EXPECT_STREQ(TownerNameForTalkID(TalkID::Tavern), "ogden");
	EXPECT_STREQ(TownerNameForTalkID(TalkID::Drunk), "farnham");
	EXPECT_STREQ(TownerNameForTalkID(TalkID::Barmaid), "gillian");
}

TEST(Stores, TownerNameForTalkID_subPagesReturnNull)
{
	// Sub-pages (buy/sell screens) should not fire StoreOpened
	EXPECT_EQ(TownerNameForTalkID(TalkID::None), nullptr);
	EXPECT_EQ(TownerNameForTalkID(TalkID::SmithBuy), nullptr);
	EXPECT_EQ(TownerNameForTalkID(TalkID::SmithSell), nullptr);
	EXPECT_EQ(TownerNameForTalkID(TalkID::SmithRepair), nullptr);
	EXPECT_EQ(TownerNameForTalkID(TalkID::WitchBuy), nullptr);
	EXPECT_EQ(TownerNameForTalkID(TalkID::Gossip), nullptr);
	EXPECT_EQ(TownerNameForTalkID(TalkID::StorytellerIdentify), nullptr);
	EXPECT_EQ(TownerNameForTalkID(TalkID::StorytellerIdentifyShow), nullptr);
}

TEST(Stores, ClearTownerDialogOptions_removesRegistrations)
{
	RegisterTownerDialogOption("farnham", []() { return std::string("A"); }, []() {});
	ASSERT_FALSE(ExtraTownerOptions.empty());

	ClearTownerDialogOptions();

	EXPECT_TRUE(ExtraTownerOptions.empty());
}

TEST(Stores, RegisterTownerDialogOption_afterClearDoesNotAccumulate)
{
	RegisterTownerDialogOption("farnham", []() { return std::string("First"); }, []() {});
	ClearTownerDialogOptions();
	RegisterTownerDialogOption("farnham", []() { return std::string("Second"); }, []() {});

	auto *opts = FindOptions("farnham");
	ASSERT_NE(opts, nullptr);
	ASSERT_EQ(opts->size(), 1u);
	EXPECT_EQ((*opts)[0].getLabel(), "Second");

	ClearTownerDialogOptions();
}

TEST(Stores, RegisterTownerDialogOption_storesOption)
{
	ClearTownerDialogOptions();

	RegisterTownerDialogOption("farnham", []() { return std::string("Go to Tiny Town"); }, []() {});

	auto *opts = FindOptions("farnham");
	ASSERT_NE(opts, nullptr);
	ASSERT_EQ(opts->size(), 1u);
	EXPECT_EQ((*opts)[0].getLabel(), "Go to Tiny Town");

	ClearTownerDialogOptions();
}

TEST(Stores, RegisterTownerDialogOption_callsOnSelect)
{
	ClearTownerDialogOptions();

	bool called = false;
	RegisterTownerDialogOption("farnham", []() { return std::string("Travel"); }, [&called]() { called = true; });

	auto *opts = FindOptions("farnham");
	ASSERT_NE(opts, nullptr);
	ASSERT_EQ(opts->size(), 1u);
	(*opts)[0].onSelect();
	EXPECT_TRUE(called);

	ClearTownerDialogOptions();
}

TEST(Stores, RegisterTownerDialogOption_emptyLabelHidesOption)
{
	ClearTownerDialogOptions();

	RegisterTownerDialogOption("farnham", []() { return std::string(""); }, []() {});

	auto *opts = FindOptions("farnham");
	ASSERT_NE(opts, nullptr);
	ASSERT_EQ(opts->size(), 1u);
	EXPECT_TRUE((*opts)[0].getLabel().empty());

	ClearTownerDialogOptions();
}

TEST(Stores, RegisterTownerDialogOption_multipleTowners)
{
	ClearTownerDialogOptions();

	RegisterTownerDialogOption("farnham", []() { return std::string("A"); }, []() {});
	RegisterTownerDialogOption("griswold", []() { return std::string("B"); }, []() {});

	auto *farnhamOpts = FindOptions("farnham");
	auto *griswoldOpts = FindOptions("griswold");
	ASSERT_NE(farnhamOpts, nullptr);
	ASSERT_NE(griswoldOpts, nullptr);
	EXPECT_EQ(farnhamOpts->size(), 1u);
	EXPECT_EQ(griswoldOpts->size(), 1u);
	EXPECT_EQ((*farnhamOpts)[0].getLabel(), "A");
	EXPECT_EQ((*griswoldOpts)[0].getLabel(), "B");

	ClearTownerDialogOptions();
}

constexpr int TestMenuMinLine = 10;
constexpr int TestTalkLine = 12;
constexpr int TestLeaveLine = 18;

TownerMenuEntry MakeEntry(int line, std::string text, std::function<void()> onSelect = {})
{
	return { line, std::move(text), UiFlags::ColorWhite, true, std::move(onSelect) };
}

/** A menu shaped like Farnham's: Talk, Leave. */
TownerMenu MakeTalkLeaveMenu()
{
	TownerMenu menu;
	menu.minLine = TestMenuMinLine;
	menu.entries.push_back(MakeEntry(TestTalkLine, "Talk"));
	menu.entries.push_back(MakeEntry(TestLeaveLine, "Leave"));
	return menu;
}

std::vector<TownerMenuEntry> MakeExtras(std::initializer_list<const char *> labels)
{
	std::vector<TownerMenuEntry> extras;
	for (const char *label : labels)
		extras.push_back(MakeEntry(0, label));
	return extras;
}

std::vector<std::string> Texts(const TownerMenu &menu)
{
	std::vector<std::string> texts;
	for (const TownerMenuEntry &entry : menu.entries)
		texts.push_back(entry.text);
	return texts;
}

/** Lines must strictly increase so that on-screen order matches menu order. */
void ExpectIncreasingLines(const TownerMenu &menu)
{
	for (size_t i = 1; i < menu.entries.size(); ++i)
		EXPECT_GT(menu.entries[i].line, menu.entries[i - 1].line) << "entry " << i;
}

TEST(Stores, PlaceExtraMenuEntries_threeOptionsStayInRegistrationOrder)
{
	TownerMenu menu = MakeTalkLeaveMenu();

	EXPECT_EQ(PlaceExtraMenuEntries(menu, MakeExtras({ "Option A", "Option B", "Option C" })), 0u);

	EXPECT_EQ(Texts(menu), (std::vector<std::string> { "Talk", "Option A", "Option B", "Option C", "Leave" }));
	ExpectIncreasingLines(menu);
	// Talk had to move up to make room, and leave stayed where it was.
	EXPECT_EQ(menu.entries.front().line, TestMenuMinLine);
	EXPECT_EQ(menu.entries.back().line, TestLeaveLine);
}

TEST(Stores, PlaceExtraMenuEntries_singleOptionLeavesBuiltInsInPlace)
{
	TownerMenu menu = MakeTalkLeaveMenu();

	EXPECT_EQ(PlaceExtraMenuEntries(menu, MakeExtras({ "Option A" })), 0u);

	EXPECT_EQ(Texts(menu), (std::vector<std::string> { "Talk", "Option A", "Leave" }));
	EXPECT_EQ(menu.entries[0].line, TestTalkLine);
	EXPECT_EQ(menu.entries[1].line, TestTalkLine + TownerMenuLineStep);
	EXPECT_EQ(menu.entries[2].line, TestLeaveLine);
}

TEST(Stores, PlaceExtraMenuEntries_noExtrasChangesNothing)
{
	TownerMenu menu = MakeTalkLeaveMenu();

	EXPECT_EQ(PlaceExtraMenuEntries(menu, {}), 0u);

	EXPECT_EQ(Texts(menu), (std::vector<std::string> { "Talk", "Leave" }));
	EXPECT_EQ(menu.entries[0].line, TestTalkLine);
	EXPECT_EQ(menu.entries[1].line, TestLeaveLine);
}

TEST(Stores, PlaceExtraMenuEntries_leaveMovesDownWhenMenuIsFull)
{
	TownerMenu menu = MakeTalkLeaveMenu();

	EXPECT_EQ(PlaceExtraMenuEntries(menu, MakeExtras({ "A", "B", "C", "D" })), 0u);

	EXPECT_EQ(Texts(menu), (std::vector<std::string> { "Talk", "A", "B", "C", "D", "Leave" }));
	ExpectIncreasingLines(menu);
	EXPECT_EQ(menu.entries.front().line, TestMenuMinLine);
	EXPECT_EQ(menu.entries.back().line, TownerMenuMaxLine);
}

TEST(Stores, PlaceExtraMenuEntries_dropsOptionsThatDoNotFit)
{
	TownerMenu menu = MakeTalkLeaveMenu();

	EXPECT_EQ(PlaceExtraMenuEntries(menu, MakeExtras({ "A", "B", "C", "D", "E", "F" })), 2u);

	EXPECT_EQ(Texts(menu), (std::vector<std::string> { "Talk", "A", "B", "C", "D", "Leave" }));
	ExpectIncreasingLines(menu);
	EXPECT_LE(menu.entries.back().line, TownerMenuMaxLine);
	EXPECT_GE(menu.entries.front().line, TestMenuMinLine);
}

TEST(Stores, PlaceExtraMenuEntries_keepsGapsBetweenBuiltInEntries)
{
	// Shaped like Wirt's menu with an item: Talk, three labels, "What have you got?", Leave.
	constexpr int BoyMinLine = 6;
	TownerMenu menu;
	menu.minLine = BoyMinLine;
	menu.entries.push_back(MakeEntry(8, "Talk"));
	menu.entries.push_back(MakeEntry(12, "Label 1"));
	menu.entries.push_back(MakeEntry(14, "Label 2"));
	menu.entries.push_back(MakeEntry(16, "Label 3"));
	menu.entries.push_back(MakeEntry(18, "Buy"));
	menu.entries.push_back(MakeEntry(20, "Leave"));

	EXPECT_EQ(PlaceExtraMenuEntries(menu, MakeExtras({ "A", "B" })), 1u);

	EXPECT_EQ(Texts(menu), (std::vector<std::string> { "Talk", "Label 1", "Label 2", "Label 3", "Buy", "A", "Leave" }));
	ExpectIncreasingLines(menu);
	// Everything above the extras moved up by one step as a block.
	EXPECT_EQ(menu.entries[0].line, 6);
	EXPECT_EQ(menu.entries[1].line, 10);
	EXPECT_EQ(menu.entries[4].line, 16);
	EXPECT_EQ(menu.entries[5].line, 18);
	EXPECT_EQ(menu.entries[6].line, 20);
}

TEST(Stores, PlaceExtraMenuEntries_menuWithoutBuiltInsDropsEverything)
{
	TownerMenu menu;
	menu.minLine = TestMenuMinLine;
	menu.entries.push_back(MakeEntry(TestLeaveLine, "Leave"));

	EXPECT_EQ(PlaceExtraMenuEntries(menu, MakeExtras({ "A", "B" })), 2u);
	EXPECT_EQ(Texts(menu), (std::vector<std::string> { "Leave" }));
}

TEST(Stores, PlaceExtraMenuEntries_preservesOnSelect)
{
	TownerMenu menu = MakeTalkLeaveMenu();
	int selected = 0;
	std::vector<TownerMenuEntry> extras;
	extras.push_back(MakeEntry(0, "A", [&selected]() { selected = 1; }));
	extras.push_back(MakeEntry(0, "B", [&selected]() { selected = 2; }));

	PlaceExtraMenuEntries(menu, std::move(extras));

	menu.entries[2].onSelect();
	EXPECT_EQ(selected, 2);
	menu.entries[1].onSelect();
	EXPECT_EQ(selected, 1);
}

} // namespace
