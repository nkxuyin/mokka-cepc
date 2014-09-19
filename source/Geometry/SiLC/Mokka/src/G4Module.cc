/*! \file G4Module.cc
    \brief An implementation of Silc::G4Module class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/


// TODDO LIST:
// - Add DETECTOR_SENSITIVITY to the configuration file.
// - Make sure that the SILICON_MIP value is correct.
// - Add SILICON_MIP value to the configuration file.
// - Add DETECTOR_MATERIAL_NAME to the configuration file.
// - Add SUPPORT_MATERIAL_NAME to the configuration file.
// - Add NUMBER_OF_CHPS_PER_COLUMN to the configuration file.
// - Add SUPPORT_THICKNESS to the configuration file.
// - Add SUPPORT_CROSS_BEAM_WIDTH to the configuration file.
// - Add SUPPORT_BACK_FROM_MODULE_EDGE to the configuration file.
// - Make clean a memory allocation/liberation procedure.

#include "G4Module.hh"

using namespace Silc;

// Constants
static const TEnergyLinearDensity SILICON_MIP = 340 * keV / mm;     ///< Minimum ionizing particle (MIP) energy density
///< for a silicon material.
static const double DETECTOR_SENSITIVITY = 0.2;                     ///< The ETD Sensitive detector threshold is 20 % of
///< a MIP.
static const char* DETECTOR_MATERIAL_NAME = "silicon_2.33gccm";     ///< A name of the material chosen for the silicon
///< detector.
static const char* SUPPORT_MATERIAL_NAME = "graphite";              ///< A name of the material chosen for the
///< detector's support.

//static const char* SILICON_TRACKER_NAME                = "SiliconTracker";
static const char* SILICON_MODULE_SHAPE_NAME           = "SiliconModuleShape";
//static const char* SILICON_MODULE_VOLUME_NAME          = "SiliconModuleVolume";
static const char* CHIP_VOLUME_NAME                    = "ChipVolume";
static const char* CHIP_SHAPE_NAME                     = "ChipShape";
static const char* SUPPORT_SHAPE_NAME                  = "ModuleSupportShape";
static const char* SUPPORT_HOLE_SHAPE_NAME             = "ModuleSupportHoleShape";
static const char* SUPPORT_WITH_HOLE_SHAPE_NAME        = "ModuleSupportWithHoleShape";
static const char* SUPPORT_VOLUME_NAME                 = "ModuleSupportVolume";

static const G4Colour SILICON_VISUALISATION_COLOR(0.75, 0.0, 0.75);
static const G4Colour CHIP_VISUALISATION_COLOR(0.0, 0.0, 0.99);

G4Module::G4Module(const SensorArray& p_oSensorArray, const ChipArray& p_oChipArray,
                   const Support& p_oModuleSupport) throw()
    : Module(p_oSensorArray, p_oChipArray, p_oModuleSupport)
{ }

void G4Module::Assemble()
{
    MakeSensitiveVolume();
    G4LogicalVolume* l_pSiliconModuleVolume = CreateSiliconModuleVolume();
    G4LogicalVolume* l_pChipVolume = CreateChipVolume();
    G4AssemblyVolume* l_pChipAssembly = CreateChipAssembly(l_pChipVolume);
    G4LogicalVolume* l_pSupportVolume = CreateSupportVolume();
    MakeModuleAssembly(l_pSiliconModuleVolume, l_pChipAssembly, l_pSupportVolume);
}

G4AssemblyVolume* G4Module::GetAssemblyVolume() const
{
    return m_pModuleAssembly;
}

VSensitiveDetector* G4Module::GetSensitiveDetector() const
{
    return m_pSensitiveDetector;
}

void G4Module::MakeSensitiveVolume()
{
    TLength l_dSensorThickness = GetSensorArray().GetSensorPrototype().GetSensorSize().z;
    TEnergy l_dParticlePassThreshold = l_dSensorThickness * SILICON_MIP * DETECTOR_SENSITIVITY;
    m_pSensitiveDetector = new SiLCSD(GetSensitiveDetectorName(), l_dParticlePassThreshold);
}

G4LogicalVolume* G4Module::CreateSiliconModuleVolume()
{
    TCuboidSize l_oModuleSize = GetModuleSize();
    TCuboidSize l_oSiliconModuleSize(l_oModuleSize.x, l_oModuleSize.y,
                                     GetSensorArray().GetSensorPrototype().GetSensorSize().z);
    G4VSolid* l_pSiliconModuleShape = G4Extensions::CreateG4Box(SILICON_MODULE_SHAPE_NAME, l_oSiliconModuleSize);
    G4Material* l_pMaterial = CGAGeometryManager::GetMaterial(DETECTOR_MATERIAL_NAME);
    GetSensorArray().GetSensorPrototype().SetMaterialProperties(l_pMaterial->GetRadlen());
    G4LogicalVolume* l_pSiliconModuleVolume = new G4LogicalVolume(l_pSiliconModuleShape, l_pMaterial,
            GetSensitiveDetectorName());
    m_pSensitiveVolume = l_pSiliconModuleVolume;
    m_pSensitiveVolume->SetSensitiveDetector(m_pSensitiveDetector);

    G4Extensions::ApplyVisualisationColour(l_pSiliconModuleVolume, SILICON_VISUALISATION_COLOR);
    return l_pSiliconModuleVolume;
}

G4LogicalVolume* G4Module::CreateChipVolume()
{
    G4VSolid* l_pChipShape = G4Extensions::CreateG4Box(CHIP_SHAPE_NAME, GetChipArray().GetChipSize());
    G4Material* l_pMaterial = CGAGeometryManager::GetMaterial(DETECTOR_MATERIAL_NAME);
    G4LogicalVolume* l_pChipVolume = new G4LogicalVolume(l_pChipShape, l_pMaterial, CHIP_VOLUME_NAME);
    G4Extensions::ApplyVisualisationColour(l_pChipVolume, CHIP_VISUALISATION_COLOR);
    return l_pChipVolume;
}

G4AssemblyVolume* G4Module::CreateChipAssembly(G4LogicalVolume* p_pChipVolume)
{
    G4AssemblyVolume* l_pChipAssembly = new G4AssemblyVolume();
    const unsigned l_uChipsPerColumn = GetChipArray().GetChipDistribution().y;// ?? GetNumberOfChipRows();
    const unsigned l_uChipsPerRow = GetChipArray().GetChipDistribution().x; // ??GetNumberOfChips() / l_uChipsPerColumn;
    const TCuboidSize& l_vSensorSize = GetSensorArray().GetSensorPrototype().GetSensorSize();
    const TCuboidSize& l_vChipSize = GetChipArray().GetChipSize();
    const TCuboidSize& l_vModuleSize = GetModuleSize();
    const unsigned l_vChipsCount[] = { l_uChipsPerRow, l_uChipsPerColumn };
    TPosition l_vStepSize;
    TPosition l_vInitialShift;
    for(unsigned n = 0; n < PLANE_DIMENSION ; ++n)
    {
        l_vStepSize[n] = ( l_vSensorSize[n] - l_vChipsCount[n] * l_vChipSize[n] ) / ( l_vChipsCount[n] + 1 );
        if(l_vStepSize[n] < 0)
            throw Exception("G4Module::CreateChipAssembly : Too many chips to place them into the sensor surface.");
        l_vInitialShift[n] = - ( l_vModuleSize[n] - l_vChipSize[n] ) / 2 + l_vStepSize[n];
    }
    l_vInitialShift.z = l_vChipSize.z / 2;

    for(unsigned l_uColumnId = 0; l_uColumnId < l_uChipsPerColumn; ++l_uColumnId)
    {
        for(unsigned l_uLineId = 0; l_uLineId < l_uChipsPerRow; ++l_uLineId)
        {
            G4ThreeVector l_vChipPosition( l_vInitialShift.x + l_uColumnId * ( l_vStepSize.x + l_vChipSize.x ),
                                           l_vInitialShift.y + l_uLineId   * ( l_vStepSize.y + l_vChipSize.y ),
                                           l_vInitialShift.z);
            l_pChipAssembly->AddPlacedVolume(p_pChipVolume, l_vChipPosition, nullptr);
        }
    }

    return l_pChipAssembly;
}

G4LogicalVolume* G4Module::CreateSupportVolume()
{
    const TCuboidSize l_vModuleSize = GetModuleSize();
    const TCuboidSize l_vSupportSize( l_vModuleSize.x - GetSupport().GetStandoffFromEdge(),
                                      l_vModuleSize.y - GetSupport().GetStandoffFromEdge(),
                                      GetSupport().GetThickness() );
    G4VSolid* l_pSupportShape = G4Extensions::CreateG4Box(SUPPORT_SHAPE_NAME, l_vSupportSize);

    const TCuboidSize l_vHoleSize( l_vSupportSize.x - GetSupport().GetWidth(),
                                   l_vSupportSize.y - GetSupport().GetWidth(),
                                   GetSupport().GetThickness() );
    G4VSolid* l_pSupportHoleShape = G4Extensions::CreateG4Box(SUPPORT_HOLE_SHAPE_NAME, l_vHoleSize);

    G4VSolid* l_pSupportWithHoleShape = new G4SubtractionSolid( SUPPORT_WITH_HOLE_SHAPE_NAME,
            l_pSupportShape,
            l_pSupportHoleShape);

    G4Material* l_oMaterial = CGAGeometryManager::GetMaterial(SUPPORT_MATERIAL_NAME);
    GetSupport().SetMaterialProperties(l_oMaterial->GetRadlen());
    return new G4LogicalVolume(l_pSupportWithHoleShape, l_oMaterial, SUPPORT_VOLUME_NAME);
}

void G4Module::MakeModuleAssembly(G4LogicalVolume* p_pSiliconModuleVolume, G4AssemblyVolume* p_pChipAssembly,
                                  G4LogicalVolume* p_pSupportVolume)
{
    const TLength l_dSiliconThickness = GetSensorArray().GetSensorPrototype().GetSensorSize().z;
    const TLength l_dChipThickness = GetChipArray().GetChipSize().z;
    const TLength l_dSupportThickness = GetSupport().GetThickness();
    m_pModuleAssembly = new G4AssemblyVolume();

    G4ThreeVector l_vSupportOffset(0, 0, l_dSupportThickness / 2);
    m_pModuleAssembly->AddPlacedVolume(p_pSupportVolume, l_vSupportOffset, nullptr);

    G4ThreeVector l_vSiliconModuleOffset(0, 0, l_dSupportThickness + l_dSiliconThickness / 2);
    m_pModuleAssembly->AddPlacedVolume(p_pSiliconModuleVolume, l_vSiliconModuleOffset, nullptr);

    G4ThreeVector l_vChipsOffset(0, 0, l_dSupportThickness + l_dSiliconThickness + l_dChipThickness / 2);
    m_pModuleAssembly->AddPlacedAssembly(p_pChipAssembly, l_vChipsOffset, nullptr);
}

G4LogicalVolume* G4Module::GetSensitiveVolume() const
{
    return m_pSensitiveVolume;
}
