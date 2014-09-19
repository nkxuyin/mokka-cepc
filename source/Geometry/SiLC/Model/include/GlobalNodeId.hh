/*! \file GlobalNodeId.hh
    \brief A definition of Silc::GlobalNodeId class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#pragma once

#include "Silc_Globals.hh"

namespace Silc
{
    /// ???
    class GlobalNodeId
    {
    private:
        static const unsigned DETECTOR_ID_SHIFT     = 59;           ///< ???
        static const unsigned SIDE_ID_SHIFT         = 57;           ///< ???
        static const unsigned LAYER_ID_SHIFT        = 48;           ///< ???
        static const unsigned SUPER_MODULE_ID_SHIFT = 40;           ///< ???
        static const unsigned MODULE_ID_SHIFT       = 32;           ///< ???
        static const unsigned CHANNEL_ID_SHIFT      = 1;            ///< ???
        static const unsigned ORIENTATION_ID_SHIFT  = 0;            ///< ???
        static const unsigned CELL_ID_0_SHIFT       = 32;           ///< ???
        static const unsigned CELL_ID_1_SHIFT       = 0;            ///< ???

        static const UInt64 DETECTOR_ID_MASK        = 0x1f;         ///< ???
        static const UInt64 SIDE_ID_MASK            = 0x03;         ///< ???
        static const UInt64 LAYER_ID_MASK           = 0x1ff;        ///< ???
        static const UInt64 SUPER_MODULE_ID_MASK    = 0xff;         ///< ???
        static const UInt64 MODULE_ID_MASK          = 0xff;         ///< ???
        static const UInt64 CHANNEL_ID_MASK         = 0x7fffffff;   ///< ???
        static const UInt64 ORIENTATION_ID_MASK     = 0x1;          ///< ???
        static const UInt64 CELL_ID_0_MASK          = 0xffffffff;   ///< ???
        static const UInt64 CELL_ID_1_MASK          = 0xffffffff;   ///< ???

    public:
        typedef unsigned SideId;
        static const SideId FORWARD_SIDE = 1;
        static const SideId VERTICAL_SIDE = 0;
        static const SideId BACKWARD_SIDE = 3;

        typedef unsigned Orientation;
        static const Orientation RPhi = 0;
        static const Orientation Z = 1;

    public:
        /// Creates a global identifier for the detector.
        GlobalNodeId(unsigned p_uDetectorId) throw();

        /// Creates a global identifier for the layer.
        GlobalNodeId(unsigned p_uDetectorId, SideId p_uSideId, unsigned p_uLayerId) throw();

        /// Creates a global identifier for the super-module.
        GlobalNodeId(unsigned p_uDetectorId, SideId p_uSideId, unsigned p_uLayerId, unsigned p_uSuperModuleId) throw();

        /// Creates a global identifier for the module.
        GlobalNodeId(unsigned p_uDetectorId, SideId p_uSideId, unsigned p_uLayerId, unsigned p_uSuperModuleId,
                     unsigned p_uModuleId) throw();

        /// Creates a global identifier for the channel.
        GlobalNodeId(unsigned p_uDetectorId, SideId p_uSideId, unsigned p_uLayerId, unsigned p_uSuperModuleId,
                     unsigned p_uModuleId, unsigned p_uChannelId, Orientation p_uChannelOrientation) throw();

        /// Creates a global identifier from the cell identifiers.
        GlobalNodeId(unsigned p_uCellId0, unsigned p_uCellId1) throw();

        unsigned GetDetectorId() const throw();
        SideId GetSideId() const throw();
        unsigned GetLayerId() const throw();
        unsigned GetSuperModuleId() const throw();
        unsigned GetModuleId() const throw();
        Orientation GetChannelOrientation() const throw();
        unsigned GetChannelId() const throw();

        unsigned GetCellId0() const throw();
        unsigned GetCellId1() const throw();

    private:
        unsigned GetValue(unsigned p_uShift, UInt64 p_uMask) const throw();
        void SetValue(unsigned p_uValue, unsigned p_uShift, UInt64 p_uMask) throw();

    private:
        /// ???
        UInt64 id;
    };
}
