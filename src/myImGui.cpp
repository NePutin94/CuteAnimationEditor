#include "../include/myImGui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/include/imgui_internal.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <cassert>
#include <cmath>    // abs
#include <cstddef>  // offsetof, NULL
#include <cstring>  // memcpy
using namespace ImGui;

ImTextureID convertGLTextureHandleToImTextureID(GLuint glTextureHandle) {
	ImTextureID textureID = (ImTextureID)NULL;
	std::memcpy(&textureID, &glTextureHandle, sizeof(GLuint));
	return textureID;
}

bool SelectableImage(const sf::Texture& texture, bool selected, const ImVec2& size)
{
	int frame_padding = 1;
	//ImGui::Selectable
	ImVec4 bg_col = sf::Color::Black;
	ImVec4 tint_col = sf::Color::Red;

	sf::Vector2f textureSize = static_cast<sf::Vector2f>(texture.getSize());
	sf::FloatRect textureRect = { 0,0, textureSize.x, textureSize.y };

	ImVec2 uv0(textureRect.left / textureSize.x,
		textureRect.top / textureSize.y);
	ImVec2 uv1((textureRect.left + textureRect.width) / textureSize.x,
		(textureRect.top + textureRect.height) / textureSize.y);

	ImTextureID textureID = convertGLTextureHandleToImTextureID(texture.getNativeHandle());
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return false;

	// Default to using texture ID as ID. User can still push string/integer prefixes.
	PushID((void*)(intptr_t)textureID);
	const ImGuiID id = window->GetID("#image");
	PopID();

	const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : g.Style.FramePadding;
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
	ItemSize(bb);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);

	// Render
	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);

	if (selected)
	{
		//RenderNavHighlight(bb, id);
		//RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, g.Style.FrameRounding));
		tint_col = sf::Color::White;
	}
	if (hovered)
		tint_col = sf::Color::Green;

	if (bg_col.w > 0.0f)
		window->DrawList->AddRectFilled(bb.Min + padding, bb.Max - padding, GetColorU32(bg_col));
	window->DrawList->AddImage(textureID, bb.Min + padding, bb.Max - padding, uv0, uv1, GetColorU32(tint_col));
	return pressed;

	//ImGui::ImageButton(textureID, size, uv0, uv1, 1, sf::Color::Black, sf::Color::White);

	//ImGui::ImageButton(texture, { 32,32 });
}
