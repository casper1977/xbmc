#pragma once
/*
 *      Copyright (C) 2012-2013 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "input/IJoystick.h"

class CJoystickXInput : public IJoystick
{
public:
  static void Initialize(JoystickArray &joysticks);
  static void DeInitialize(JoystickArray &joysticks);

  virtual ~CJoystickXInput() { }
  virtual void Update();
  virtual const SJoystick &GetState() const { return m_state; }

private:
  CJoystickXInput(unsigned int controllerID, unsigned int id);

  SJoystick    m_state;
  unsigned int m_controllerID; // XInput port, in the range (0, 3)
  DWORD        m_dwPacketNumber; // If unchanged, controller state hasn't changed
};