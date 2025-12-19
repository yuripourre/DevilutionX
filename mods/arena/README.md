# Arena Mod for DevilutionX

## Overview
This mod adds PvP Arena functionality to DevilutionX as external content, allowing players to battle in three custom arena levels:

1. **Church Arena** - Cathedral-themed combat arena
2. **Hell Arena** - Hell-themed combat arena  
3. **Circle of Life** - Ultimate arena challenge

## Status: ⚠️ Work in Progress

This mod structure has been created but requires **Lua API extensions** to function. The core game has been successfully refactored to remove hardcoded arena code.

## What's Complete ✅

- ✅ Mod directory structure created
- ✅ Arena `.dun` map files migrated
- ✅ Lua mod skeleton with arena definitions
- ✅ Core game refactored (arena code removed)
- ✅ `mod.json` configuration file

## What's Needed 🚧

The following Lua API extensions need to be implemented in DevilutionX:

### 1. Custom Level Registration
```lua
DevilutionX.registerCustomLevel({
    id = "ARENA_CHURCH",
    name = "Church Arena",
    mapFile = "arena/church.dun",
    dungeonType = "DTYPE_CATHEDRAL",
    playerSpawn = {x = 28, y = 20},
    viewPosition = {x = 29, y = 22}
})
```

### 2. Custom Item Registration
```lua
DevilutionX.registerCustomItem({
    id = "ARENA_POTION",
    name = "Arena Potion",
    type = "IMISC",
    behavior = "rejuvenation_in_custom_level",
    levelRestriction = {"ARENA_CHURCH", "ARENA_HELL", "ARENA_CIRCLE_OF_LIFE"}
})
```

### 3. Chat Command Registration
```lua
DevilutionX.registerCommand("/arena", function(param)
    -- Custom command logic
end, "Enter a PvP Arena", "<arena-number>")
```

### 4. Level Transition API
```lua
DevilutionX.loadCustomLevel(levelId, transitionType)
DevilutionX.getCurrentLevel()
DevilutionX.isPlayerOnCustomLevel(levelId)
```

### 5. Inventory Management
```lua
DevilutionX.giveItem(itemId, count)
DevilutionX.createItem(itemDefinition)
```

## Implementation Roadmap

### Phase 1: Core Lua API (2-3 weeks)
- Add `registerCustomLevel()` API
- Add `registerCustomItem()` API
- Add `registerCommand()` API
- Level loading/transition infrastructure

### Phase 2: Arena Integration (1-2 weeks)
- Implement arena-specific level loading
- Custom portal/trigger system for arenas
- Arena potion item behavior

### Phase 3: Polish & Testing (1 week)
- Multiplayer testing
- Balance adjustments
- Documentation

## Code Changes Summary

### Files Removed/Modified
The following arena-specific code has been removed from the core:

#### Enums & Types
- `SL_ARENA_CHURCH`, `SL_ARENA_HELL`, `SL_ARENA_CIRCLE_OF_LIFE` from `_setlevels` enum
- `IMISC_ARENAPOT` from item misc types
- `IDI_ARENAPOT` from item IDs

#### Functions Removed
- `IsArenaLevel(_setlevels)`
- `GetArenaLevelType(_setlevels)`
- `ForceArenaTrig()`
- `Player::isOnArenaLevel()`
- `TextCmdArena()`
- `TextCmdArenaPot()`
- `AppendArenaOverview()`

#### Modified Files
- `Source/levels/gendung.h` - Removed arena enums
- `Source/levels/setmaps.h` - Removed `GetArenaLevelType`
- `Source/levels/setmaps.cpp` - Removed arena map loading cases
- `Source/levels/trigs.cpp` - Removed arena trigger code
- `Source/player.h` - Removed `isOnArenaLevel()` method
- `Source/control.cpp` - Removed `/arena` and `/arenapot` commands
- `Source/itemdat.h` - Removed `IMISC_ARENAPOT` and `IDI_ARENAPOT`
- `Source/items.cpp` - Removed arena potion handling
- `Source/inv.cpp` - Removed arena potion restrictions
- `Source/portals/validation.cpp` - Removed arena level validation
- `Source/interfac.cpp` - Removed arena cutscene logic
- `Source/engine/render/scrollrt.cpp` - Removed arena lighting checks
- And many more...

## Architecture

### Current Structure
```
mods/arena/
├── mod.json              # Mod metadata and configuration
├── init.lua              # Mod entry point (TODO: needs Lua API)
├── README.md             # This file
└── arena/                # Arena map files
    ├── church.dun
    ├── hell.dun
    └── circle_of_death.dun
```

### When API is Ready
The `init.lua` file contains placeholder code that will work once the Lua API is implemented. The mod registers:
- 3 custom arena levels
- Arena potion custom item
- `/arena` and `/arenapot` chat commands

## Testing Plan

Once Lua API is available:
1. Enable the mod in DevilutionX
2. Test level loading for each arena
3. Test arena potion creation and usage
4. Test chat commands
5. Test multiplayer synchronization
6. Test portal/trigger functionality

## Contributing

To complete this mod, contributions are needed in:
1. **C++ Lua API bindings** - Core DevilutionX changes
2. **Lua mod implementation** - Update `init.lua` once API exists
3. **Testing** - Multiplayer and balance testing
4. **Documentation** - Update this README as features are completed

## License

This mod follows the DevilutionX project license (see main project LICENSE).

## Credits

- Original arena implementation: DevilutionX core team
- Mod conversion: Community effort
