#pragma once

#include <string>

#include <expected.hpp>

#include "engine/clx_sprite.hpp"

namespace devilution {

extern OptionalOwnedClxSpriteList PanelButtonDown;
extern OptionalOwnedClxSpriteList TalkButton;
/** @brief Scroll arrows for chat mute button list (4 frames: up active, up, down active, down). From ui_art/scrlarrw or sb_arrow. */
extern OptionalOwnedClxSpriteList ChatScrollArrows;

tl::expected<void, std::string> LoadMainPanel();
void FreeMainPanel();

} // namespace devilution
