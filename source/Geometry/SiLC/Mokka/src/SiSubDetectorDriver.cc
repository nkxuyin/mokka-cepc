/*! \file SiSubDetectorDriver.cc
    \brief An implementation of Silc::SiSubDetectorDriver class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "SiSubDetectorDriver.hh"
#include "G4BarrelArray.hh"
#include "G4EndcapArray.hh"
#include "BarrelSerializer.hh"
#include "EndcapSerializer.hh"

using namespace Silc;

static void UpdateMokkaForSIT(const G4BarrelArray& p_oBarrelArray)
{
    if(Control::DETECTOR_MODEL.find("ILD") != 0) return;

    MokkaWriter l_oWriter;
    unsigned l_uNbBarrels = p_oBarrelArray.GetNumberOfBarrels();
    assert_ex(l_uNbBarrels >= 2, std_ext::out_of_range_exception);
    l_oWriter.Write("SIT1_Half_Length_Z", p_oBarrelArray[0].GetLength()/2.);
    l_oWriter.Write("SIT2_Half_Length_Z", p_oBarrelArray[l_uNbBarrels-1].GetLength()/2.);
    l_oWriter.Write("SIT1_Radius", p_oBarrelArray[0].GetInnerRadiusMin());
    l_oWriter.Write("SIT2_Radius", p_oBarrelArray[l_uNbBarrels-1].GetInnerRadiusMin());
}

static void UpdateMokkaForSET(const G4BarrelArray&)
{
}

static void UpdateMokkaForETD(const G4EndcapArray&)
{
}

#ifdef MOKKA_GEAR

#include <MokkaGear.h>
#include <gearimpl/ZPlanarParametersImpl.h>
#include "GearStore.hh"

static void SetupGearForBarrel(const G4BarrelArray& p_oBarrelArray, const BarrelSerializer& p_oSerializer)
{
    GearWriter l_oWriter;
    p_oSerializer.Save(p_oBarrelArray, l_oWriter);
    gear::GearMgr* l_pGearMgr = MokkaGear::getMgr();
    l_oWriter.Finalize(p_oSerializer.GetSubDetectorName() + "_full", *l_pGearMgr);

    gear::ZPlanarParametersImpl* l_pParameters = new gear::ZPlanarParametersImpl(
        gear::ZPlanarParametersImpl::CMOS, 0, 0, 0, 0, 0);

    for(unsigned l_uBarrelId = 0; l_uBarrelId < p_oBarrelArray.GetNumberOfBarrels(); ++l_uBarrelId)
    {
        const G4Barrel& l_oBarrel = p_oBarrelArray.GetG4Barrel(l_uBarrelId);
        for(unsigned l_uLayerId = 0; l_uLayerId < l_oBarrel.GetNbLayers(); ++l_uLayerId)
        {
            l_pParameters->addLayer(l_oBarrel.GetNbFace(),
                                    l_oBarrel.GetInitialFaceAngle(),
                                    l_oBarrel.GetLayerRadius(l_uLayerId),
                                    0,
                                    l_oBarrel.GetLayerThickness(l_uLayerId),
                                    l_oBarrel.GetLayerLength(l_uLayerId),
                                    l_oBarrel.GetLayerWidth(l_uLayerId),
                                    l_oBarrel.GetMeanSupportRadiationLength(l_uLayerId),
                                    l_oBarrel.GetSensitiveVolumeRadius(l_uLayerId),
                                    0,
                                    l_oBarrel.GetSensitiveVolumeThickness(l_uLayerId),
                                    l_oBarrel.GetLayerLength(l_uLayerId),
                                    l_oBarrel.GetSensitiveVolumeWidth(l_uLayerId),
                                    l_oBarrel.GetSensitiveVolumeRadiationLength(l_uLayerId));
        }
    }
    l_pGearMgr->setSITParameters(l_pParameters);
}

static void SetupGearForEndcap(const G4EndcapArray& p_oEndcapArray, const EndcapSerializer& p_oSerializer)
{
    GearWriter l_oWriter;
    p_oSerializer.Save(p_oEndcapArray, l_oWriter);
    gear::GearMgr* l_pGearMgr = MokkaGear::getMgr();
    l_oWriter.Finalize(p_oSerializer.GetSubDetectorName(), *l_pGearMgr);
}

#else

static void SetupGearForBarrel(const G4BarrelArray&, const BarrelSerializer&)
{
}

static void SetupGearForEndcap(const G4EndcapArray&, const EndcapSerializer&)
{
}

#endif

static SiSubDetectorDriver<G4BarrelArray, BarrelSerializer> theSIT("sit", false,
        &UpdateMokkaForSIT, &SetupGearForBarrel);
static SiSubDetectorDriver<G4BarrelArray, BarrelSerializer> theSET("set", true,
        &UpdateMokkaForSET, &SetupGearForBarrel);
static SiSubDetectorDriver<G4EndcapArray, EndcapSerializer> theETD("etd", true,
        &UpdateMokkaForETD, &SetupGearForEndcap);
