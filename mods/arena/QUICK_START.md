# Arena Mod - Quick Start

## What Is This?

The arena functionality (PvP combat levels) has been converted from hardcoded game features into an optional mod. This makes the base game cleaner and arenas fully optional.

## Current Status: ⚠️ Waiting for Lua API

The mod structure is complete, but needs Lua API support in DevilutionX to function.

## Files in This Directory

- **README.md** - Full documentation of the mod and what it will do
- **LUA_API_TODO.md** - Technical specification for needed Lua APIs
- **CONVERSION_SUMMARY.md** - Detailed summary of all code changes
- **mod.json** - Mod metadata and configuration
- **init.lua** - Mod entry point (contains arena logic)
- **arena/** - Arena dungeon map files (church.dun, hell.dun, circle_of_death.dun)

## For Users

**Can I use this mod now?**  
Not yet. The mod requires Lua API extensions that haven't been implemented in DevilutionX.

**When will it be ready?**  
Estimated 5-8 weeks for Lua API implementation.

**Will my saves still work?**  
Yes, the core game changes are backward compatible.

## For Developers

**What was changed in the core?**  
All arena-specific code (~500+ lines) was removed from 22 files. See [CONVERSION_SUMMARY.md](CONVERSION_SUMMARY.md).

**What needs to be implemented?**  
Lua APIs for custom levels, items, and commands. See [LUA_API_TODO.md](LUA_API_TODO.md).

**Can I help?**  
Yes! The Lua API implementation is the main blocker. Check [LUA_API_TODO.md](LUA_API_TODO.md) for details.

## For Mod Creators

**Can I use this as a template?**  
Absolutely! Once the Lua APIs are implemented, this mod demonstrates:
- How to create custom levels
- How to add custom items
- How to register chat commands  
- How to structure a mod with Lua

**Is the API documented?**  
Yes, see [LUA_API_TODO.md](LUA_API_TODO.md) for the complete API specification.

## Quick Reference

### Arena Levels
1. **Church Arena** - Cathedral theme, town portal spawn
2. **Hell Arena** - Hell theme, town portal spawn  
3. **Circle of Life** - Hell theme, ultimate challenge

### Commands (Once API is ready)
- `/arena <1-3>` - Enter an arena
- `/arenapot <count>` - Get arena potions (multiplayer only)

### Arena Potion
Restores full life and mana, but only works inside arena levels.

## Git Diff Summary

**Files modified**: 22  
**Lines removed**: ~9,600 (mostly reformatting)  
**Lines added**: ~9,400 (mostly reformatting)  
**Net code deletion**: ~200 lines of arena-specific code  

## Architecture

```
Core Game (C++)
    ↓ (needs Lua bindings)
Lua API Layer (TO BE IMPLEMENTED)
    ↓
Mod System (init.lua)
    ↓
Arena Functionality
```

## Timeline

- ✅ **Week 1-2**: Core refactoring (COMPLETE)
- 🚧 **Week 3-5**: Lua API implementation (PENDING)
- 🚧 **Week 6-7**: Mod integration & testing (PENDING)
- 🚧 **Week 8**: Polish & release (PENDING)

## Contact / Discussion

For questions about:
- **The mod structure**: See README.md in this directory
- **Core code changes**: See CONVERSION_SUMMARY.md
- **Lua API needs**: See LUA_API_TODO.md
- **DevilutionX project**: Visit the main repository

---

**Last Updated**: 2025-12-19  
**Status**: Phase 1 Complete (Core Refactoring)  
**Next Phase**: Lua API Implementation
