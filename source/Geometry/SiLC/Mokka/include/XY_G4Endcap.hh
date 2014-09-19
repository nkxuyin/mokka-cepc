/*! \file XY_G4Endcap.hh
    \brief A definition of Silc::XY_G4Endcap class.

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

#include "G4Endcap.hh"
#include "XY_Endcap.hh"

namespace Silc
{
    /// ???
    class XY_G4Endcap : public XY_Endcap, public G4Endcap
    {
    public: // Static methods.
        /// ???
        static P<G4Endcap> MakeInstance();

    public: // Basic methods.
        /// ???
        virtual ~XY_G4Endcap();

    public: // IAssemblable implementations.
        /// ???
        virtual void Assemble();

    public: // G4Endcap implementations.
        /// ???
        virtual void MakeSupportAssembly();

        /// Creates a GEANT4 assembly of a super-module.
        virtual void MakeSuperModuleAssembly();

    private: // Internal methods.
        /// ???
        void SetupParameters();

        /// ???
        void MakeRohacelLayer();

        /// ???
        void MakeCarbonLayers();

        /// ???
        void MakeReinforcementLayer();

        /// ???
        ///// Construct the "L shape"
        G4AssemblyVolume* BuildFlatLayerVolume(const string& p_sMaterial, TLength p_dThickness);

        /// ???
        G4AssemblyVolume* BuildBendedLayerVolume(const string& p_sMaterial, TLength p_dThickness);

        /// ???
        G4VSolid* CreateTrapezoidShape(const string& p_sMaterial, TLength p_dThickness);

        /// ???
        G4VSolid* JoinSupportCuboidShape(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness);

        /// ???
        G4VSolid* JoinBox(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness,
                          TLength p_dXLength, TLength p_dYLength,
                          TCoordinate p_dXShift = 0, TCoordinate p_dYShift = 0, TAngle p_dRotation = 0);


//        /// ???
//        G4VSolid* CreateReinforcementDisc(const string& p_sMaterial, TLength p_dThickness);

//        /// ???
//        G4VSolid* CreateSupportDiscShape(const string& p_sMaterial, TLength p_dThickness);

//        /// ???
//        G4VSolid* ApplyBottomSubtraction(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness);

//        /// ???
//        G4VSolid* ApplyLeftSubtraction(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness);

//        /// ???
//        G4VSolid* JoinReinforcementShortEdge(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness);

//        /// ???
//        G4VSolid* JoinReinforcementLongEdge(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness);

        /// ???
        G4LogicalVolume* CreateLogicalVolume(G4VSolid* p_pShape, const string& p_sMaterialName,
                                             const string& p_sPrefix);
    private: // Data members.
        /// ???
        TLength m_dOctagonInscribedCircleRadius;

        /// ???
        TLength m_dTrapeziumShift;

        /// ???
        TLength m_dOctagonSide;

        /// ???
        TLength m_dTrapeziumHeight;

        /// ???
        TLength m_dTrapeziumLowerSide;

        /// ???
        TLength m_dTrapeziumUpperSide;

        /// ???
        TLength m_dRectangleShift;

        /// ???
        TLength m_dRectangleVerticalSide;

        /// ???
        TLength m_dRectangleHorizontalSide;
    };
}
