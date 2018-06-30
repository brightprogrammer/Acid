#include "Joysticks.hpp"

namespace fl
{
	void CallbackJoystickConnect(WsiShell shell, WsiJoystick port, const char *name, uint32_t buttonCount, uint32_t axesCount, bool connected)
	{
		Joystick joystick = Joysticks::Get()->m_connected[port];
		bool newConnection = joystick.m_connected != connected;
		joystick.m_connected = connected;
		joystick.m_name = name;
		joystick.m_buttons.resize(buttonCount);
		joystick.m_axes.resize(axesCount);

		if (newConnection)
		{
			for (int i = 0; i < buttonCount; i++)
			{
				joystick.m_buttons[i] = WSI_ACTION_RELEASE;
			}

			for (int i = 0; i < axesCount; i++)
			{
				joystick.m_axes[i] = 0.0f;
			}
		}
	}

	void CallbackJoystickButton(WsiShell shell, WsiJoystick port, uint32_t button, WsiAction action)
	{
		Joystick joystick = Joysticks::Get()->m_connected[port];
		joystick.m_buttons[button] = action;
	}

	void CallbackJoystickAxis(WsiShell shell, WsiJoystick port, uint32_t axis, float amount)
	{
		Joystick joystick = Joysticks::Get()->m_connected[port];
		joystick.m_axes[axis] = amount;
	}

	Joysticks::Joysticks() :
		IModule(),
		m_connected(std::array<Joystick, WSI_JOYSTICK_LAST>())
	{
		for (int i = 0; i < WSI_JOYSTICK_LAST; i++)
		{
			Joystick joystick = Joystick();
			joystick.m_port = static_cast<WsiJoystick>(i);
			joystick.m_connected = false;
			m_connected[i] = joystick;
		}

		// Sets the joystick callbacks.
		wsiCmdSetJoystickConnectCallback(Display::Get()->GetWsiShell(), CallbackJoystickConnect);
		wsiCmdSetJoystickButtonCallback(Display::Get()->GetWsiShell(), CallbackJoystickButton);
		wsiCmdSetJoystickAxisCallback(Display::Get()->GetWsiShell(), CallbackJoystickAxis);
	}

	Joysticks::~Joysticks()
	{
	}

	void Joysticks::Update()
	{
	}

	float Joysticks::GetAxis(const WsiJoystick &port, const int &axis) const
	{
		if (axis < 0 || axis > GetCountAxes(port))
		{
			return false;
		}

		return m_connected.at(port).m_axes[axis];
	}

	bool Joysticks::GetButton(const WsiJoystick &port, const int &button) const
	{
		if (button < 0 || button > GetCountButtons(port))
		{
			return false;
		}

		return m_connected.at(port).m_buttons[button];
	}
}
