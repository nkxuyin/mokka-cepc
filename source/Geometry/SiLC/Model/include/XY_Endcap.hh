/*! \file XY_Endcap.hh
    \brief A definition of Silc::XY_Endcap class.

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
    /// Represents an endcap generated using XY concept.
    class XY_Endcap : virtual public Endcap
    {
    private: // Type definitions.
        /// ???
        typedef TCoordinate (*limit_calculator)(TLength p_dRadius, TCoordinate p_dShift);

    public: // Static members.
        /// An endcap type for this class.
        static const EndcapType ENDCAP_TYPE;

        /// ???
        static P<Endcap> MakeInstance();

    public: // Basic methods.
        /// ???
        static TLength CalculateOctagonSideLength(TLength p_dInscribedCircleRadius);

    public: // Endcap implementations.
        /// ???
        virtual unsigned GetNumberOfJoinedLayers() const;

        /// \copydoc Silc::Endcap::GetType()
        virtual const EndcapType& GetType() const;

    public: // IAssemblable implementations.
        /// Places modules into the endcap using XY concept.
        virtual void Assemble();

    private: // Internal methods.
        /// ??? Calculates a nuber of the vertical lines of modules into the super-module.
        unsigned CalculateNumberOfLadderSteps(unsigned p_uLayerId);

        /// Fills a super-module with modules using XY concept.
        void FillSupermodule(unsigned p_uLayerId, unsigned p_uNumberOfLadderSteps);

        /// ???
        void CollectStepParameters(unsigned p_uLayerId, Filler& p_oFiller, TLength p_dStepLeftShift, TLength p_dStepRightShift);

        /// ???
        void PlaceStepModules(unsigned p_uLayerId, const vector<unsigned>& p_vStepFilling, TLength p_dInitialPosition, TLength p_dStepLeftShift);

        /// ???
        TPlanePosition ApplyRotation(const TPlanePosition& p_vPosition, ModuleDescriptor::RotationIndicator p_dRotationIndicator);

        /// ??? Returns module size for a zone specified by 'p_uZoneId' taking into account it rotation.
        TPlaneDimension GetRotatedModuleProjection(unsigned p_uLayerId, unsigned p_uZoneId = 0);

        /// ???
        TPlanePosition GetInitialPosition(unsigned p_uLayerId);

        /// ???
        limit_calculator GetLimitCalculator(unsigned p_uZoneId);

        /// ???
        static TCoordinate CalculateLimitForCircularBorder(TLength p_dRadius, TCoordinate p_dShift);

        /// ???
        static TCoordinate CalculateLimitForOctogonalBorder(TLength p_dRadius, TCoordinate p_dShift);
    };
}
