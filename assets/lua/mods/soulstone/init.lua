-- Soulstone Mod
--
-- Implements Diablo's Soulstone quest item entirely in Lua:
--   - Defines the soulstone item using the custom items API (no TSV, no custom sprite).
--   - Spawns the soulstone when Diablo dies (at the last frame of his death animation).
--   - Right-clicking the soulstone on level 16, after Diablo is defeated, triggers
--     the game-ending sequence on all clients (multiplayer included).

local events   = require("devilutionx.events")
local items    = require("devilutionx.items")
local monsters = require("devilutionx.monsters")
local player   = require("devilutionx.player")
local game     = require("devilutionx.game")

-- ─── Item definition ─────────────────────────────────────────────────────────

local SOULSTONE_MAPPING_ID = 10001
-- Frame 36 of objcurs.cel (the bloodstone graphic) is reused as the soulstone cursor.
local SOULSTONE_CURSOR = 36

items.addItem({
  mappingId     = SOULSTONE_MAPPING_ID,
  name          = "Soulstone",
  shortName     = "Soulstone",
  type          = items.ItemType.Misc,
  class         = items.ItemClass.Quest,
  equipType     = items.ItemEquipType.Unequipable,
  cursorGraphic = SOULSTONE_CURSOR,
  usable        = true,
  value         = 0,
  dropRate      = 0,
})

local soulstonItemIdx = items.getItemIndex(SOULSTONE_MAPPING_ID)

-- ─── Diablo death: resurrect beam + soulstone drop + wounded towner ──────────

-- C++ fires OnMonsterDeath (cancellable) at the last frame of the death animation.
-- Returning true skips the default corpse/invalidate; we handle cleanup ourselves.

events.OnMonsterDeath.add(function(monster, deathFrame)
  if monster.typeId ~= monsters.MonsterID.Diablo then
    return false
  end

  local pos = monster.position
  game.addResurrectBeamAt(pos.x, pos.y)
  items.spawnQuestItem(soulstonItemIdx, pos.x, pos.y, true)
  game.replaceMonsterWithWoundedTowner(monster)

  local self = player.self()
  if self ~= nil then
    self:say(player.HeroSpeech.VengeanceIsMine)
  end
  return true
end)

-- ─── Soulstone use ───────────────────────────────────────────────────────────

events.OnItemUse.add(function(p, item)
  if item.curs ~= SOULSTONE_CURSOR then
    return false
  end

  -- Diablo's lair is on dungeon level 16.
  local DIABLO_LEVEL = 16
  if not p:isOnLevel(DIABLO_LEVEL) then
    p:say(player.HeroSpeech.ThatWontWorkHere)
    return true
  end

  if not game.isQuestDone(game.QuestID.Diablo) then
    p:say(player.HeroSpeech.ICantUseThisYet)
    return true
  end

  -- Trigger the win-screen sequence. In multiplayer the player leaves with
  -- LEAVE_ENDING, which sets gbSomebodyWonGameKludge on other clients so
  -- they call PrepDoEnding when they next load a level.
  game.prepDoEnding()
  return true
end)
