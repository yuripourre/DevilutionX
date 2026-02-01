#include "control_chat.hpp"
#include "control.hpp"
#include "control_panel.hpp"

#include "control/control_chat_commands.hpp"
#include "engine/backbuffer_state.hpp"
#include "engine/render/clx_render.hpp"
#include "options.h"
#include "panels/console.hpp"
#include "panels/mainpanel.hpp"
#include "quick_messages.hpp"
#include "utils/display.h"
#include "utils/sdl_compat.h"
#include "utils/str_cat.hpp"

namespace devilution {

std::optional<TextInputState> ChatInputState;
char TalkMessage[MAX_SEND_STR_LEN];
bool TalkButtonsDown[3];
int sgbPlrTalkTbl;
bool WhisperList[MAX_PLRS];
OptionalOwnedClxSpriteList talkButtons;

namespace {

char TalkSave[8][MAX_SEND_STR_LEN];
uint8_t TalkSaveIndex;
uint8_t NextTalkSave;
TextInputCursorState ChatCursor;

int MuteButtons = 3;
int MuteButtonPadding = 2;
Rectangle MuteButtonRect { { 172, 69 }, { 61, 16 } };

// Scrolling for mute buttons when there are more than 3 other players
int MuteButtonScrollOffset = 0;
constexpr int MaxVisibleMuteButtons = 3;
Rectangle ScrollUpButtonRect { { 158, 69 }, { 12, 16 } };
Rectangle ScrollDownButtonRect { { 158, 88 }, { 12, 16 } };
bool ScrollUpButtonDown = false;
bool ScrollDownButtonDown = false;

void ResetChatMessage()
{
	if (CheckChatCommand(TalkMessage))
		return;

	uint32_t pmask = 0;

	for (size_t i = 0; i < Players.size(); i++) {
		if (WhisperList[i])
			pmask |= 1 << i;
	}

	NetSendCmdString(pmask, TalkMessage);
}

void ControlPressEnter()
{
	if (TalkMessage[0] != 0) {
		ResetChatMessage();
		uint8_t i = 0;
		for (; i < 8; i++) {
			if (strcmp(TalkSave[i], TalkMessage) == 0)
				break;
		}
		if (i >= 8) {
			strcpy(TalkSave[NextTalkSave], TalkMessage);
			NextTalkSave++;
			NextTalkSave &= 7;
		} else {
			uint8_t talkSave = NextTalkSave - 1;
			talkSave &= 7;
			if (i != talkSave) {
				strcpy(TalkSave[i], TalkSave[talkSave]);
				*BufCopy(TalkSave[talkSave], ChatInputState->value()) = '\0';
			}
		}
		TalkMessage[0] = '\0';
		TalkSaveIndex = NextTalkSave;
	}
	ResetChat();
}

void ControlUpDown(int v)
{
	for (int i = 0; i < 8; i++) {
		TalkSaveIndex = (v + TalkSaveIndex) & 7;
		if (TalkSave[TalkSaveIndex][0] != 0) {
			ChatInputState->assign(TalkSave[TalkSaveIndex]);
			return;
		}
	}
}

} // namespace

void DrawChatBox(const Surface &out)
{
	if (!ChatFlag)
		return;

	const Point mainPanelPosition = GetMainPanel().position;

	DrawPanelBox(out, MakeSdlRect(175, sgbPlrTalkTbl + 20, 294, 5), mainPanelPosition + Displacement { 175, 4 });
	int off = 0;
	for (int i = 293; i > 283; off++, i--) {
		DrawPanelBox(out, MakeSdlRect((off / 2) + 175, sgbPlrTalkTbl + off + 25, i, 1), mainPanelPosition + Displacement { (off / 2) + 175, off + 9 });
	}
	DrawPanelBox(out, MakeSdlRect(185, sgbPlrTalkTbl + 35, 274, 30), mainPanelPosition + Displacement { 185, 19 });
	DrawPanelBox(out, MakeSdlRect(180, sgbPlrTalkTbl + 65, 284, 5), mainPanelPosition + Displacement { 180, 49 });
	for (int i = 0; i < 10; i++) {
		DrawPanelBox(out, MakeSdlRect(180, sgbPlrTalkTbl + i + 70, i + 284, 1), mainPanelPosition + Displacement { 180, i + 54 });
	}
	DrawPanelBox(out, MakeSdlRect(170, sgbPlrTalkTbl + 80, 310, 55), mainPanelPosition + Displacement { 170, 64 });

	int x = mainPanelPosition.x + 200;
	const int y = mainPanelPosition.y + 10;

	const uint32_t len = DrawString(out, TalkMessage, { { x, y }, { 250, 39 } },
	    {
	        .flags = UiFlags::ColorWhite | UiFlags::PentaCursor,
	        .lineHeight = 13,
	        .cursorPosition = static_cast<int>(ChatCursor.position),
	        .highlightRange = { static_cast<int>(ChatCursor.selection.begin), static_cast<int>(ChatCursor.selection.end) },
	    });
	ChatInputState->truncate(len);

	x += 46;
	
	// Count only active (joined) other players to avoid empty slots
	int totalOtherPlayers = 0;
	for (size_t i = 0; i < Players.size(); i++) {
		if (i != MyPlayerId && Players[i].plractive)
			totalOtherPlayers++;
	}
	
	// Clamp scroll offset
	const int maxScrollOffset = std::max(0, totalOtherPlayers - MaxVisibleMuteButtons);
	MuteButtonScrollOffset = std::clamp(MuteButtonScrollOffset, 0, maxScrollOffset);
	
	// Draw scroll up button if needed
	if (totalOtherPlayers > MaxVisibleMuteButtons && MuteButtonScrollOffset > 0) {
		const Point scrollUpPos = mainPanelPosition + Displacement { ScrollUpButtonRect.position.x, ScrollUpButtonRect.position.y + 15 };
		const UiFlags arrowColor = ScrollUpButtonDown ? UiFlags::ColorButtonpushed : UiFlags::ColorButtonface;
		DrawString(out, "^", { scrollUpPos, { ScrollUpButtonRect.size.width, ScrollUpButtonRect.size.height } }, 
		    { .flags = arrowColor | UiFlags::AlignCenter | UiFlags::VerticalCenter });
	}
	
	// Draw scroll down button if needed
	if (totalOtherPlayers > MaxVisibleMuteButtons && MuteButtonScrollOffset < maxScrollOffset) {
		const Point scrollDownPos = mainPanelPosition + Displacement { ScrollDownButtonRect.position.x, ScrollDownButtonRect.position.y + 15 };
		const UiFlags arrowColor = ScrollDownButtonDown ? UiFlags::ColorButtonpushed : UiFlags::ColorButtonface;
		DrawString(out, "v", { scrollDownPos, { ScrollDownButtonRect.size.width, ScrollDownButtonRect.size.height } }, 
		    { .flags = arrowColor | UiFlags::AlignCenter | UiFlags::VerticalCenter });
	}
	
	int activeOtherIndex = 0;
	int visibleBtnIndex = 0;
	for (size_t i = 0; i < Players.size(); i++) {
		Player &player = Players[i];
		if (&player == MyPlayer || !player.plractive)
			continue;
		
		// Skip active players before scroll offset
		if (activeOtherIndex < MuteButtonScrollOffset) {
			activeOtherIndex++;
			continue;
		}
		
		// Only show MaxVisibleMuteButtons
		if (visibleBtnIndex >= MaxVisibleMuteButtons)
			break;

		const UiFlags color = player.friendlyMode ? UiFlags::ColorWhitegold : UiFlags::ColorRed;
		const Point talkPanPosition = mainPanelPosition + Displacement { 172, 84 + 18 * visibleBtnIndex };
		if (WhisperList[i]) {
			// the normal (unpressed) voice button is pre-rendered on the panel, only need to draw over it when the button is held
			if (TalkButtonsDown[visibleBtnIndex]) {
				const unsigned spriteIndex = visibleBtnIndex == 0 ? 2 : 3; // the first button sprite includes a tip from the devils wing so is different to the rest.
				ClxDraw(out, talkPanPosition, (*talkButtons)[spriteIndex]);

				// Draw the translated string over the top of the default (english) button. This graphic is inset to avoid overlapping the wingtip, letting
				// the first button be treated the same as the other two further down the panel.
				RenderClxSprite(out, (*TalkButton)[2], talkPanPosition + Displacement { 4, -15 });
			}
		} else {
			unsigned spriteIndex = visibleBtnIndex == 0 ? 0 : 1; // the first button sprite includes a tip from the devils wing so is different to the rest.
			if (TalkButtonsDown[visibleBtnIndex])
				spriteIndex += 4; // held button sprites are at index 4 and 5 (with and without wingtip respectively)
			ClxDraw(out, talkPanPosition, (*talkButtons)[spriteIndex]);

			// Draw the translated string over the top of the default (english) button. This graphic is inset to avoid overlapping the wingtip, letting
			// the first button be treated the same as the other two further down the panel.
			RenderClxSprite(out, (*TalkButton)[TalkButtonsDown[visibleBtnIndex] ? 1 : 0], talkPanPosition + Displacement { 4, -15 });
		}
		DrawString(out, player._pName, { { x, y + 60 + visibleBtnIndex * 18 }, { 204, 0 } }, { .flags = color });

		activeOtherIndex++;
		visibleBtnIndex++;
	}
}

bool CheckMuteButton()
{
	if (!ChatFlag)
		return false;

	// Check scroll up button
	Rectangle scrollUpBtn = ScrollUpButtonRect;
	SetPanelObjectPosition(UiPanels::Main, scrollUpBtn);
	if (scrollUpBtn.contains(MousePosition)) {
		ScrollUpButtonDown = true;
		ScrollDownButtonDown = false;
		return true;
	}
	
	// Check scroll down button
	Rectangle scrollDownBtn = ScrollDownButtonRect;
	SetPanelObjectPosition(UiPanels::Main, scrollDownBtn);
	if (scrollDownBtn.contains(MousePosition)) {
		ScrollDownButtonDown = true;
		ScrollUpButtonDown = false;
		return true;
	}

	Rectangle buttons = MuteButtonRect;

	SetPanelObjectPosition(UiPanels::Main, buttons);

	buttons.size.height = (MuteButtons * buttons.size.height) + ((MuteButtons - 1) * MuteButtonPadding);

	if (!buttons.contains(MousePosition))
		return false;

	for (bool &talkButtonDown : TalkButtonsDown) {
		talkButtonDown = false;
	}

	const Point mainPanelPosition = GetMainPanel().position;

	TalkButtonsDown[(MousePosition.y - (69 + mainPanelPosition.y)) / 18] = true;

	return true;
}

void CheckMuteButtonUp()
{
	if (!ChatFlag)
		return;

	// Handle scroll up button
	if (ScrollUpButtonDown) {
		Rectangle scrollUpBtn = ScrollUpButtonRect;
		SetPanelObjectPosition(UiPanels::Main, scrollUpBtn);
		if (scrollUpBtn.contains(MousePosition)) {
			MuteButtonScrollOffset = std::max(0, MuteButtonScrollOffset - 1);
		}
		ScrollUpButtonDown = false;
		return;
	}
	
	// Handle scroll down button
	if (ScrollDownButtonDown) {
		Rectangle scrollDownBtn = ScrollDownButtonRect;
		SetPanelObjectPosition(UiPanels::Main, scrollDownBtn);
		if (scrollDownBtn.contains(MousePosition)) {
			int totalOtherPlayers = 0;
			for (size_t i = 0; i < Players.size(); i++) {
				if (i != MyPlayerId && Players[i].plractive)
					totalOtherPlayers++;
			}
			const int maxScrollOffset = std::max(0, totalOtherPlayers - MaxVisibleMuteButtons);
			MuteButtonScrollOffset = std::min(maxScrollOffset, MuteButtonScrollOffset + 1);
		}
		ScrollDownButtonDown = false;
		return;
	}

	for (bool &talkButtonDown : TalkButtonsDown)
		talkButtonDown = false;

	Rectangle buttons = MuteButtonRect;

	SetPanelObjectPosition(UiPanels::Main, buttons);

	buttons.size.height = (MuteButtons * buttons.size.height) + ((MuteButtons - 1) * MuteButtonPadding);

	if (!buttons.contains(MousePosition))
		return;

	int off = (MousePosition.y - buttons.position.y) / (MuteButtonRect.size.height + MuteButtonPadding);

	// Map visible button index to actual player ID (only active players, no empty slots)
	int activeOtherIndex = 0;
	size_t playerId = 0;
	for (; playerId < Players.size(); ++playerId) {
		if (playerId == MyPlayerId || !Players[playerId].plractive)
			continue;
		if (activeOtherIndex == MuteButtonScrollOffset + off)
			break;
		activeOtherIndex++;
	}
	
	if (playerId < Players.size())
		WhisperList[playerId] = !WhisperList[playerId];
}

void TypeChatMessage()
{
	if (!IsChatAvailable())
		return;

	ChatFlag = true;
	TalkMessage[0] = '\0';
	ChatInputState.emplace(TextInputState::Options {
	    .value = TalkMessage,
	    .cursor = &ChatCursor,
	    .maxLength = sizeof(TalkMessage) - 1 });
	for (bool &talkButtonDown : TalkButtonsDown) {
		talkButtonDown = false;
	}
	MuteButtonScrollOffset = 0;
	ScrollUpButtonDown = false;
	ScrollDownButtonDown = false;
	sgbPlrTalkTbl = GetMainPanel().size.height + PanelPaddingHeight;
	RedrawEverything();
	TalkSaveIndex = NextTalkSave;

	SDL_Rect rect = MakeSdlRect(GetMainPanel().position.x + 200, GetMainPanel().position.y + 22, 0, 27);
	SDL_SetTextInputArea(ghMainWnd, &rect, /*cursor=*/0);
	SDLC_StartTextInput(ghMainWnd);
}

void ResetChat()
{
	ChatFlag = false;
	SDLC_StopTextInput(ghMainWnd);
	ChatCursor = {};
	ChatInputState = std::nullopt;
	sgbPlrTalkTbl = 0;
	RedrawEverything();
}

bool IsChatActive()
{
	if (!IsChatAvailable())
		return false;

	if (!ChatFlag)
		return false;

	return true;
}

bool CheckKeypress(SDL_Keycode vkey)
{
	if (!IsChatAvailable())
		return false;
	if (!ChatFlag)
		return false;

	switch (vkey) {
	case SDLK_ESCAPE:
		ResetChat();
		return true;
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		ControlPressEnter();
		return true;
	case SDLK_DOWN:
		ControlUpDown(1);
		return true;
	case SDLK_UP:
		ControlUpDown(-1);
		return true;
	default:
		return vkey >= SDLK_SPACE && vkey <= SDLK_Z;
	}
}

void DiabloHotkeyMsg(uint32_t dwMsg)
{
	assert(dwMsg < QuickMessages.size());

#ifdef _DEBUG
	constexpr std::string_view LuaPrefix = "/lua ";
	for (const std::string &msg : GetOptions().Chat.szHotKeyMsgs[dwMsg]) {
		if (!msg.starts_with(LuaPrefix)) continue;
		InitConsole();
		RunInConsole(std::string_view(msg).substr(LuaPrefix.size()));
	}
#endif

	if (!IsChatAvailable()) {
		return;
	}

	for (const std::string &msg : GetOptions().Chat.szHotKeyMsgs[dwMsg]) {
#ifdef _DEBUG
		if (msg.starts_with(LuaPrefix)) continue;
#endif
		char charMsg[MAX_SEND_STR_LEN];
		CopyUtf8(charMsg, msg, sizeof(charMsg));
		NetSendCmdString(0xFFFFFF, charMsg);
	}
}

bool IsChatAvailable()
{
	return gbIsMultiplayer;
}

bool HandleTalkTextInputEvent(const SDL_Event &event)
{
	return HandleInputEvent(event, ChatInputState);
}

} // namespace devilution
