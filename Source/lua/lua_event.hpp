#pragma once

#include <cstdint>
#include <string_view>

namespace devilution {

struct Item;
struct Player;
struct Monster;

namespace lua {

void MonsterDataLoaded();
void UniqueMonsterDataLoaded();
void ItemDataLoaded();
void UniqueItemDataLoaded();
void PlayerDataLoaded();

void StoreOpened(std::string_view name);

void OnMonsterTakeDamage(const Monster *monster, int damage, int damageType);

void OnPlayerGainExperience(const Player *player, uint32_t exp);
void OnPlayerTakeDamage(const Player *player, int damage, int damageType);

/**
 * @brief Called when the local player uses an item.
 * @return true if a handler cancelled the default use behaviour.
 */
bool OnItemUse(const Player *player, const Item *item);

/**
 * @brief Called when Diablo's defeat is about to end the game.
 * @param diablo The Diablo monster, or nullptr when a single-player game is loaded after his defeat.
 * @param isLocalKill Whether this client killed Diablo (and is therefore responsible for syncing state to other clients).
 * @return true if a handler wants the game to keep running instead of showing the ending.
 */
bool OnDiabloDefeated(const Monster *diablo, bool isLocalKill);

void LoadModsComplete();
void GameDrawComplete();
void GameStart();

} // namespace lua

} // namespace devilution
