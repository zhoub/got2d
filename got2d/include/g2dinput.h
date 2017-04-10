#pragma once
#include <gml/gmlrect.h>
#include "g2dconfig.h"
#include "g2dmessage.h"

namespace g2d
{
	// Button state of Mouse/Keyboard
	enum class G2DAPI SwitchState : int
	{
		Releasing,		// not pressed
		JustPressed,	// just pressed down, without response
		Pressing,		// pressed for a while, will repeatly triggering
	};

	// Retrieve Keyboard states 
	class G2DAPI Keyboard
	{
	public:
		static Keyboard& Instance();

		virtual SwitchState GetPressState(KeyCode key) const = 0;

		// Repeating count will be increasing each frame
		virtual uint32_t GetRepeatingCount(KeyCode key) const = 0;

		// None of button is pressed/pressing
		virtual bool IsFree() const = 0;

		bool IsReleasing(KeyCode key) { return GetPressState(key) == SwitchState::Releasing; }

		bool IsPressing(KeyCode key) { return GetPressState(key) == SwitchState::Pressing; }

		bool IsJustPressed(KeyCode key) { return GetPressState(key) == SwitchState::JustPressed; }
	};

	// Retrieve Mouse states
	class G2DAPI Mouse
	{
	public:
		static Mouse& Instance();

		// Current screen position of cursor
		virtual const gml::coord& GetCursorPosition() const = 0;

		// Screen position of cursor when button was pressed just
		virtual const gml::coord& GetCursorPressPosition(MouseButton button) const = 0;

		virtual SwitchState GetPressState(MouseButton button) const = 0;

		// Repeating count will be increasing each frame
		virtual uint32_t GetRepeatingCount(MouseButton button) const = 0;

		// None of button is pressed/pressing
		virtual bool IsFree() const = 0;

		bool IsReleasing(MouseButton button) { return GetPressState(button) == SwitchState::Releasing; }

		bool IsPressing(MouseButton button) { return GetPressState(button) == SwitchState::Pressing; }

		bool IsJustPressed(MouseButton button) { return GetPressState(button) == SwitchState::JustPressed; }
	};

	inline Keyboard& GetKeyboard()
	{
		return Keyboard::Instance();
	}

	inline Mouse& GetMouse()
	{
		return Mouse::Instance();
	}
}
