-- Soulstone Mod
--
-- Implements Diablo's Soulstone quest item:
--   - Spawns the soulstone when Diablo dies (at the moment the death explosion fires).
--   - Right-clicking the soulstone on level 16, after Diablo is defeated, triggers
--     the game-ending sequence on all clients (multiplayer included).
--
-- This mod requires the soulstone item to be defined in itemdat.tsv with:
--   IDidx = IDI_SOULSTONE  (ItemIndex.Soulstone in Lua)
--   miscId = IMISC_SOULSTONE
-- and the SOULSTONE cursor sprite to be present in data/inv/soulstone.clx.

local events   = require("devilutionx.events")
local items    = require("devilutionx.items")
local monsters = require("devilutionx.monsters")
local player   = require("devilutionx.player")
local game     = require("devilutionx.game")

-- The death animation frame at which Diablo's explosion fires and the
-- soulstone should appear on the ground.
local DIABLO_EXPLOSION_FRAME = 140

-- ─── Soulstone spawn ─────────────────────────────────────────────────────────

events.OnMonsterDeath.add(function(monster, deathFrame)
  if monster.typeId ~= monsters.MonsterID.Diablo then
    return
  end
  if deathFrame ~= DIABLO_EXPLOSION_FRAME then
    return
  end

  local pos = monster.position
  -- sendmsg=true syncs the item across all clients in multiplayer.
  items.spawnQuestItem(items.ItemIndex.Soulstone, pos.x, pos.y, true)

  local self = player.self()
  if self ~= nil then
    self:say(player.HeroSpeech.VengeanceIsMine)
  end
end)

-- ─── Soulstone use ───────────────────────────────────────────────────────────

events.OnItemUse.add(function(p, item)
  if item.IDidx ~= items.ItemIndex.Soulstone then
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
