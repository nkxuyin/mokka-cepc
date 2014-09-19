/*! \file XUV_Endcap.hh
    \brief A definition of Silc::XUV_Endcap class.

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

#include "Endcap.hh"

namespace Silc
{
    /// Represents an endcap generated using XUV concept.
    class XUV_Endcap : virtual public Endcap
    {
    private: // Type definitions.
        /// A container for the build state parameters.
        struct BuildState;

        /// A comparation function.
        typedef const TLength& (*compare_function)(const TLength&, const TLength&);

        /// A rounding function.
        typedef double (*round_function)(double);

    public: // Static members.
        /// An endcap type for this class.
        static const EndcapType ENDCAP_TYPE;

        /// ???
        static P<Endcap> MakeInstance();

    public: // Endcap implementations.
        /// ???
        virtual unsigned GetNumberOfJoinedLayers() const;

        /// \copydoc Silc::Endcap::GetType()
        virtual const EndcapType& GetType() const;

    public: // IAssemblable implementations.
        /// Places modules into the endcap using XUV concept.
        virtual void Assemble();

    private: // Internal methods.

        /// Returns module size for a zone specified by 'p_uZoneId' taking into account it rotation.
        TCuboidSize GetRotatedModuleSize(unsigned p_uZoneId = 0);

        /// Calculates a nuber of the vertical lines of modules into the super-module.
        unsigned CalculateNumberOfLadderSteps();

        /// Fills a super-module with modules using XUV concept.
        void FillSupermodule(unsigned p_uNumberOfLadderSteps);

        /// Fill a one zone of super-module with modules using XUV concept.
        void FillZone(BuildState& p_oState, compare_function p_fCompare, round_function p_fRound);
    };

    /// A container for the build state parameters.
    struct XUV_Endcap::BuildState
    {
        /// A current step ID.
        unsigned StepId;

        /// A current zone ID.
        unsigned ZoneId;

        /// A X-coordinate of the left modules bound for the current step.
        TLength LeftStepX;

        /// A X-coordinate of the right modules bound for the current step.
        TLength RightStepX;

        /// A first avaible Y-coordinate for the current step.
        TLength AvailableY;
    };
}
