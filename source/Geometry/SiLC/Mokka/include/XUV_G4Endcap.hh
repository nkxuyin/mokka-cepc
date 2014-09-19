/*! \file XUV_G4Endcap.hh
    \brief A definition of Silc::XUV_G4Endcap class.

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
#include "XUV_Endcap.hh"

namespace Silc
{
    /// ???
    class XUV_G4Endcap : public XUV_Endcap, public G4Endcap
    {
    public: // Static methods.
        /// ???
        static P<G4Endcap> MakeInstance();

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
        void MakeRohacelLayer();

        /// ???
        void MakeCarbonLayers();

        /// ???
        void MakeReinforcementLayer();

        /// ???
        ///// Construct the "L shape"
        G4LogicalVolume* BuildFlatLayerVolume(const string& p_sMaterial, TLength p_dThickness);

        /// ???
        G4LogicalVolume* BuildBendedLayerVolume(string p_gsMaterial, TLength p_dThickness);

        /// ???
        G4VSolid* CreateReinforcementDisc(const string& p_sMaterial, TLength p_dThickness);

        /// ???
        G4VSolid* CreateSupportDiscShape(const string& p_sMaterial, TLength p_dThickness);

        /// ???
        G4VSolid* ApplyBottomSubtraction(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness);

        /// ???
        G4VSolid* ApplyLeftSubtraction(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness);

        /// ???
        G4VSolid* JoinReinforcementShortEdge(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness);

        /// ???
        G4VSolid* JoinReinforcementLongEdge(G4VSolid* p_pShape, const string& p_sMaterial, TLength p_dThickness);

        /// ???
        G4LogicalVolume* CreateLogicalVolume(G4VSolid* p_pShape, const string& p_sMaterialName,
                                             const string& p_sPrefix);
    };
}
