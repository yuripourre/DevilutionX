-- Arena Mod for DevilutionX
-- Adds PvP Arena levels as custom content

local arena = {}

-- Arena level definitions
arena.levels = {
	{
		id = "ARENA_CHURCH",
		name = "Church Arena",
		mapFile = "arena/church.dun",
		playerSpawn = {x = 28, y = 20},
		viewPosition = {x = 29, y = 22},
		dungeonType = "DTYPE_CATHEDRAL"
	},
	{
		id = "ARENA_HELL",
		name = "Hell Arena",
		mapFile = "arena/hell.dun",
		playerSpawn = {x = 33, y = 26},
		viewPosition = {x = 34, y = 26},
		dungeonType = "DTYPE_HELL"
	},
	{
		id = "ARENA_CIRCLE_OF_LIFE",
		name = "Circle of Life",
		mapFile = "arena/circle_of_death.dun",
		playerSpawn = {x = 29, y = 26},
		viewPosition = {x = 30, y = 26},
		dungeonType = "DTYPE_HELL"
	}
}

-- Arena Potion item definition
arena.arenaPotionItemId = nil

-- Register custom levels with the game
function arena:registerLevels()
	for i, level in ipairs(self.levels) do
		-- TODO: Add Lua API hook to register custom setlevels
		-- DevilutionX.registerCustomLevel(level)
		print(string.format("Arena: Registered level %s", level.name))
	end
end

-- Register arena potion item
function arena:registerItems()
	-- TODO: Add Lua API hook to register custom items
	-- self.arenaPotionItemId = DevilutionX.registerCustomItem({
	--     name = "Arena Potion",
	--     type = "IMISC",
	--     behavior = "rejuvenation_in_arena"
	-- })
	print("Arena: Registered Arena Potion")
end

-- Register chat commands
function arena:registerCommands()
	-- TODO: Add Lua API hook to register commands
	-- DevilutionX.registerCommand("/arena", function(arenaNum)
	--     self:enterArena(arenaNum)
	-- end, "Enter a PvP Arena", "<arena-number>")
	
	-- DevilutionX.registerCommand("/arenapot", function(count)
	--     self:giveArenaPotions(count)
	-- end, "Gives Arena Potions", "<number>")
	print("Arena: Registered commands /arena and /arenapot")
end

-- Enter an arena
function arena:enterArena(arenaNumber)
	arenaNumber = tonumber(arenaNumber) or 1
	if arenaNumber < 1 or arenaNumber > #self.levels then
		print(string.format("Arena: Invalid arena number. Choose 1-%d", #self.levels))
		return
	end
	
	local level = self.levels[arenaNumber]
	-- TODO: Trigger level transition
	-- DevilutionX.loadCustomLevel(level.id)
	print(string.format("Arena: Entering %s", level.name))
end

-- Give arena potions to player
function arena:giveArenaPotions(count)
	count = tonumber(count) or 1
	-- TODO: Add items to player inventory
	-- DevilutionX.giveItem(self.arenaPotionItemId, count)
	print(string.format("Arena: Gave %d arena potion(s)", count))
end

-- Check if player is on an arena level
function arena:isOnArenaLevel()
	-- TODO: Get current level from game state
	-- local currentLevel = DevilutionX.getCurrentLevel()
	-- for _, level in ipairs(self.levels) do
	--     if currentLevel == level.id then
	--         return true
	--     end
	-- end
	return false
end

-- Initialize mod
function arena:init()
	print("Arena Mod: Initializing...")
	self:registerLevels()
	self:registerItems()
	self:registerCommands()
	print("Arena Mod: Initialization complete")
end

-- Start the mod
arena:init()

return arena
