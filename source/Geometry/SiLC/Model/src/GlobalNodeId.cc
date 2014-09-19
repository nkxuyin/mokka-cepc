/*! \file GlobalNodeId.cc
    \brief An implementation of Silc::GlobalNodeId class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "GlobalNodeId.hh"

using namespace Silc;

GlobalNodeId::GlobalNodeId(unsigned p_uDetectorId) throw()
    : id(0)
{
    SetValue(p_uDetectorId, DETECTOR_ID_SHIFT, DETECTOR_ID_MASK);
}

GlobalNodeId::GlobalNodeId(unsigned p_uDetectorId, SideId p_uSideId, unsigned p_uLayerId) throw()
    : id(0)
{
    SetValue(p_uDetectorId, DETECTOR_ID_SHIFT, DETECTOR_ID_MASK);
    SetValue(p_uSideId, SIDE_ID_SHIFT, SIDE_ID_MASK);
    SetValue(p_uLayerId, LAYER_ID_SHIFT, LAYER_ID_MASK);
}

GlobalNodeId::GlobalNodeId(unsigned p_uDetectorId, SideId p_uSideId, unsigned p_uLayerId, unsigned p_uSuperModuleId)
throw() : id(0)
{
    SetValue(p_uDetectorId, DETECTOR_ID_SHIFT, DETECTOR_ID_MASK);
    SetValue(p_uSideId, SIDE_ID_SHIFT, SIDE_ID_MASK);
    SetValue(p_uLayerId, LAYER_ID_SHIFT, LAYER_ID_MASK);
    SetValue(p_uSuperModuleId, SUPER_MODULE_ID_SHIFT, SUPER_MODULE_ID_MASK);
}

GlobalNodeId::GlobalNodeId(unsigned p_uDetectorId, SideId p_uSideId, unsigned p_uLayerId, unsigned p_uSuperModuleId,
                           unsigned p_uModuleId) throw() : id(0)
{
    SetValue(p_uDetectorId, DETECTOR_ID_SHIFT, DETECTOR_ID_MASK);
    SetValue(p_uSideId, SIDE_ID_SHIFT, SIDE_ID_MASK);
    SetValue(p_uLayerId, LAYER_ID_SHIFT, LAYER_ID_MASK);
    SetValue(p_uSuperModuleId, SUPER_MODULE_ID_SHIFT, SUPER_MODULE_ID_MASK);
    SetValue(p_uModuleId, MODULE_ID_SHIFT, MODULE_ID_MASK);
}

GlobalNodeId::GlobalNodeId(unsigned p_uDetectorId, SideId p_uSideId, unsigned p_uLayerId, unsigned p_uSuperModuleId,
                           unsigned p_uModuleId, unsigned p_uChannelId, Orientation p_uChannelOrientation) throw()
    : id(0)
{
    SetValue(p_uDetectorId, DETECTOR_ID_SHIFT, DETECTOR_ID_MASK);
    SetValue(p_uSideId, SIDE_ID_SHIFT, SIDE_ID_MASK);
    SetValue(p_uLayerId, LAYER_ID_SHIFT, LAYER_ID_MASK);
    SetValue(p_uSuperModuleId, SUPER_MODULE_ID_SHIFT, SUPER_MODULE_ID_MASK);
    SetValue(p_uModuleId, MODULE_ID_SHIFT, MODULE_ID_MASK);
    SetValue(p_uChannelId, CHANNEL_ID_SHIFT, CHANNEL_ID_MASK);
    SetValue(p_uChannelOrientation, ORIENTATION_ID_SHIFT, ORIENTATION_ID_MASK);
}

GlobalNodeId::GlobalNodeId(unsigned p_uCellId0, unsigned p_uCellId1) throw()
    : id(0)
{
    SetValue(p_uCellId0, CELL_ID_0_SHIFT, CELL_ID_0_MASK);
    SetValue(p_uCellId1, CELL_ID_1_SHIFT, CELL_ID_1_MASK);
}

unsigned GlobalNodeId::GetDetectorId() const throw()
{
    return GetValue(DETECTOR_ID_SHIFT, DETECTOR_ID_MASK);
}

GlobalNodeId::SideId GlobalNodeId::GetSideId() const throw()
{
    return GetValue(SIDE_ID_SHIFT, SIDE_ID_MASK);
}

unsigned GlobalNodeId::GetLayerId() const throw()
{
    return GetValue(LAYER_ID_SHIFT, LAYER_ID_MASK);
}

unsigned GlobalNodeId::GetSuperModuleId() const throw()
{
    return GetValue(SUPER_MODULE_ID_SHIFT, SUPER_MODULE_ID_MASK);
}

unsigned GlobalNodeId::GetModuleId() const throw()
{
    return GetValue(MODULE_ID_SHIFT, MODULE_ID_MASK);
}

unsigned GlobalNodeId::GetChannelId() const throw()
{
    return GetValue(CHANNEL_ID_SHIFT, CHANNEL_ID_MASK);
}

GlobalNodeId::Orientation GlobalNodeId::GetChannelOrientation() const throw()
{
    return GetValue(ORIENTATION_ID_SHIFT, ORIENTATION_ID_MASK);
}

unsigned GlobalNodeId::GetCellId0() const throw()
{
    return GetValue(CELL_ID_0_SHIFT, CELL_ID_0_MASK);
}

unsigned GlobalNodeId::GetCellId1() const throw()
{
    return GetValue(CELL_ID_1_SHIFT, CELL_ID_1_MASK);
}

unsigned GlobalNodeId::GetValue(unsigned p_uShift, UInt64 p_uMask) const throw()
{
    return (unsigned)((id >> p_uShift) & p_uMask);
}

void GlobalNodeId::SetValue(unsigned p_uValue, unsigned p_uShift, UInt64 p_uMask) throw()
{
    id |= (((UInt64)p_uValue) & p_uMask) << p_uShift;
}
