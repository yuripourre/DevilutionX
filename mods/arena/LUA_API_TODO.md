# Lua API Extensions Needed for Arena Mod

This document outlines the Lua API extensions needed to support custom levels, items, and commands in DevilutionX mods.

## Priority: High
These extensions will enable complete mod support for custom content beyond just asset replacement.

## 1. Custom Level API

### registerCustomLevel()
**Purpose:** Allow mods to register custom dungeon levels

**Interface:**
```cpp
// C++ side (in Source/lua/modules/levels.cpp or similar)
int Lua_RegisterCustomLevel(lua_State* L) {
    // Parse level definition from Lua table
    const char* levelId = luaL_checkstring(L, 1);
    // Get table fields: name, mapFile, dungeonType, spawn coords, etc.
    
    // Register level in custom level registry
    CustomLevelRegistry::Register(levelId, levelDef);
    return 0;
}
```

**Lua Usage:**
```lua
DevilutionX.registerCustomLevel({
    id = "ARENA_CHURCH",
    name = "Church Arena",
    mapFile = "arena/church.dun",  -- Relative to mod directory
    dungeonType = "DTYPE_CATHEDRAL",
    playerSpawn = {x = 28, y = 20},
    viewPosition = {x = 29, y = 22},
    exitTriggers = {
        {x = 28, y = 20, destination = "town"}
    }
})
```

### loadCustomLevel()
**Purpose:** Trigger transition to a custom level

**Interface:**
```cpp
int Lua_LoadCustomLevel(lua_State* L) {
    const char* levelId = luaL_checkstring(L, 1);
    // Look up level in CustomLevelRegistry
    // Call level loading logic (similar to StartNewLvl)
    return 0;
}
```

**Lua Usage:**
```lua
DevilutionX.loadCustomLevel("ARENA_CHURCH")
```

### getCurrentLevel()
**Purpose:** Get the ID of the current level

**Lua Usage:**
```lua
local levelId = DevilutionX.getCurrentLevel()
if levelId == "ARENA_CHURCH" then
    -- Arena-specific logic
end
```

## 2. Custom Item API

### registerCustomItem()
**Purpose:** Register custom item types with special behaviors

**Interface:**
```cpp
int Lua_RegisterCustomItem(lua_State* L) {
    // Parse item definition
    CustomItemDefinition def;
    def.id = lua_tostring(L, -1, "id");
    def.name = lua_tostring(L, -1, "name");
    def.type = lua_tostring(L, -1, "type");
    def.behavior = lua_tostring(L, -1, "behavior");
    
    // Register in CustomItemRegistry
    CustomItemRegistry::Register(def);
    return 1;  // Return custom item ID
}
```

**Lua Usage:**
```lua
local arenaPotionId = DevilutionX.registerCustomItem({
    id = "ARENA_POTION",
    name = "Arena Potion",
    type = "IMISC",
    baseItem = "POTION_OF_FULL_REJUVENATION",
    behavior = "custom",  -- Triggers Lua callback
    sprite = "items/arenapot",
    onUse = function(player)
        if player:isOnCustomLevel("ARENA_.*") then
            player:restoreFullLife()
            player:restoreFullMana()
            return true  -- Item consumed
        else
            player:say("This won't work here")
            return false  -- Item not consumed
        end
    end
})
```

### giveItem()
**Purpose:** Add items to player inventory

**Lua Usage:**
```lua
DevilutionX.giveItem(player, arenaPotionId, count)
```

## 3. Chat Command API

### registerCommand()
**Purpose:** Register custom chat commands

**Interface:**
```cpp
int Lua_RegisterCommand(lua_State* L) {
    const char* command = luaL_checkstring(L, 1);
    // Store Lua function reference for callback
    int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    CommandRegistry::Register(command, callbackRef);
    return 0;
}
```

**Lua Usage:**
```lua
DevilutionX.registerCommand("/arena", function(player, args)
    local arenaNum = tonumber(args[1])
    if not arenaNum or arenaNum < 1 or arenaNum > 3 then
        player:sendMessage("Invalid arena number. Use 1-3")
        return
    end
    
    local levels = {"ARENA_CHURCH", "ARENA_HELL", "ARENA_CIRCLE_OF_LIFE"}
    DevilutionX.loadCustomLevel(levels[arenaNum])
end, {
    description = "Enter a PvP Arena",
    usage = "<arena-number>"
})
```

## 4. Player API Extensions

### Player Methods Needed
```lua
-- Level queries
player:isOnLevel(levelNum)
player:isOnCustomLevel(levelPattern)  -- Pattern matching support
player:getCurrentLevel()

-- Inventory
player:addItem(itemId, count)
player:hasItem(itemId)
player:removeItem(itemId, count)

-- Status
player:restoreFullLife()
player:restoreFullMana()
player:say(message)
player:sendMessage(message)
```

## 5. Game State API

### Multiplayer Support
```lua
DevilutionX.isMultiplayer()
DevilutionX.getPlayers()  -- Returns table of all players
DevilutionX.broadcastMessage(message)
```

### Level Information
```lua
DevilutionX.getLevelType(levelId)  -- Returns DTYPE_*
DevilutionX.isValidLevel(levelId)
```

## Implementation Notes

### Custom Level Registry
A new system is needed to manage custom levels:

```cpp
// Source/levels/custom_levels.h
class CustomLevelRegistry {
public:
    struct LevelDefinition {
        std::string id;
        std::string name;
        std::string mapFile;
        dungeon_type type;
        Point playerSpawn;
        Point viewPosition;
        std::vector<TriggerDef> triggers;
    };
    
    static void Register(const std::string& id, const LevelDefinition& def);
    static const LevelDefinition* Get(const std::string& id);
    static bool IsCustomLevel(const std::string& id);
};
```

### Custom Item Registry
Similar registry for custom items:

```cpp
// Source/items/custom_items.h
class CustomItemRegistry {
public:
    struct ItemDefinition {
        std::string id;
        std::string name;
        item_misc_id baseType;
        int luaCallbackRef;  // For onUse callback
    };
    
    static int Register(const ItemDefinition& def);
    static const ItemDefinition* Get(int customItemId);
};
```

### Integration Points

1. **Level Loading** - Modify `LoadSetMap()` to check CustomLevelRegistry
2. **Item Usage** - Modify `UseItem()` to check for custom item callbacks
3. **Chat Commands** - Add command dispatch to check CommandRegistry
4. **Save/Load** - Custom levels must serialize properly

## Testing Strategy

1. Create unit tests for each API function
2. Integration test with arena mod
3. Multiplayer synchronization tests
4. Save game compatibility tests

## Backward Compatibility

- Custom level IDs should not conflict with existing `_setlevels` values
- Custom items should use a separate ID range
- Existing mods (like Hellfire) should continue working

## Estimated Effort

- **Custom Level API**: 1-2 weeks
- **Custom Item API**: 1-2 weeks  
- **Command API**: 3-5 days
- **Player/Game API extensions**: 1 week
- **Testing & Polish**: 1 week

**Total**: 5-8 weeks (matches original estimate)

## Next Steps

1. Review this design with DevilutionX maintainers
2. Create GitHub issue/RFC for Lua API extensions
3. Implement APIs incrementally
4. Update arena mod as APIs become available
5. Document APIs for other mod creators

## Related Files

- `Source/lua/lua.cpp` - Main Lua integration
- `Source/lua/modules/` - Existing Lua modules to extend
- `Source/levels/gendung.h` - Level system integration
- `Source/items.cpp` - Item system integration
- `Source/control.cpp` - Command system integration
