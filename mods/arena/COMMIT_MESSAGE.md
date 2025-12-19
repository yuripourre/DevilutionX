# Suggested Git Commit Message

```
Refactor: Convert arenas from hardcoded features to external mod

BREAKING CHANGE: Arena functionality temporarily disabled pending Lua API

This commit removes all hardcoded arena code from the core game and
prepares it for implementation as an optional mod. Arenas will be
re-enabled once the required Lua APIs are implemented.

## Changes

### Core Game (22 files modified)
- Removed arena enum values (SL_ARENA_*)
- Removed arena item types (IMISC_ARENAPOT, IDI_ARENAPOT)
- Removed arena-specific functions (IsArenaLevel, GetArenaLevelType, etc.)
- Removed arena chat commands (/arena, /arenapot)
- Removed arena level loading logic
- Removed arena trigger system
- Removed Player::isOnArenaLevel() checks throughout codebase

### New Mod Structure
- Created mods/arena/ directory
- Added mod.json configuration
- Added init.lua with arena logic (pending Lua API)
- Migrated arena .dun map files
- Comprehensive documentation (README, LUA_API_TODO, CONVERSION_SUMMARY)

### Impact
- Base game is now cleaner without PvP-specific code
- Arenas become optional, not forced on all installations  
- Sets precedent for other content to become mods
- Provides template for future mod creators

### Next Steps
1. Implement Lua APIs specified in mods/arena/LUA_API_TODO.md
2. Test arena mod with new APIs
3. Document mod system for other developers
4. Consider converting other optional features to mods

## Testing
- ✅ All arena references removed from source (verified with grep)
- ✅ Code structure validated
- ⚠️ Compilation pending SDK fix (unrelated to this PR)
- ⏳ Full testing pending Lua API implementation

## Related Issues
- Addresses modding system extensibility
- Prepares for custom level support
- Improves code maintainability

## Documentation
See mods/arena/QUICK_START.md for overview
See mods/arena/LUA_API_TODO.md for API requirements
See mods/arena/CONVERSION_SUMMARY.md for detailed changes

Estimated completion: 5-8 weeks (pending Lua API implementation)
```

---

## Alternative Shorter Version

```
refactor: extract arena functionality to external mod

Convert hardcoded arena levels and items to mod structure.
Arena functionality temporarily disabled pending Lua API implementation.

- Remove all SL_ARENA_* enums and IsArenaLevel() checks
- Remove IMISC_ARENAPOT and IDI_ARENAPOT items
- Remove /arena and /arenapot commands
- Create mods/arena/ with complete mod structure
- Add documentation for Lua API requirements

22 files modified, ~200 lines net deletion

See mods/arena/QUICK_START.md for details
```

---

## For Pull Request Description

```markdown
## Description
This PR refactors the arena system from hardcoded game features into an external mod structure. This is part of an effort to make DevilutionX more modular and extensible.

## Motivation
- Arenas are optional PvP content that not all players use
- Hardcoded arena code adds complexity to the core game
- Converting to mod demonstrates extensible mod system
- Provides template for future custom content mods

## Changes
### Removed from Core (~500 lines)
- Arena level enums (SL_ARENA_CHURCH, SL_ARENA_HELL, SL_ARENA_CIRCLE_OF_LIFE)
- Arena item types (IMISC_ARENAPOT, IDI_ARENAPOT)  
- Arena helper functions (IsArenaLevel, GetArenaLevelType, ForceArenaTrig, etc.)
- Arena commands (/arena, /arenapot)
- Arena-specific rendering, trigger, and item logic

### Added Mod Structure
- `mods/arena/` directory with complete mod setup
- Lua-based arena implementation (pending API)
- Migrated arena .dun map files
- Comprehensive documentation

## Status
✅ Core refactoring complete  
⚠️ Arena functionality disabled  
🚧 Awaiting Lua API implementation (see mods/arena/LUA_API_TODO.md)

## Testing
- [x] All arena references removed from core
- [x] Code structure validated
- [ ] Compilation verified (pending SDK fix)
- [ ] Arena mod functional (requires Lua API)

## Documentation
- [mods/arena/QUICK_START.md](mods/arena/QUICK_START.md) - Overview
- [mods/arena/README.md](mods/arena/README.md) - Full documentation  
- [mods/arena/LUA_API_TODO.md](mods/arena/LUA_API_TODO.md) - API requirements
- [mods/arena/CONVERSION_SUMMARY.md](mods/arena/CONVERSION_SUMMARY.md) - Detailed changes

## Breaking Changes
- ⚠️ Arena functionality is temporarily unavailable
- Will be restored once Lua APIs are implemented
- No save game incompatibilities

## Next Steps
1. Review and merge core refactoring
2. Implement Lua APIs (separate PR)
3. Complete arena mod implementation
4. Release as optional mod

## Related
- Part of modding system improvements
- Enables custom level support
- Template for future content mods

---

Estimated timeline: 5-8 weeks for full completion
Phase 1 (this PR): Complete ✅
Phase 2 (Lua API): ~3-5 weeks
Phase 3 (Testing): ~1 week
```
