/*! \file G4EndcapArray.cc
    \brief An implementation of Silc::G4EndcapArray class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "G4EndcapArray.hh"
#include "XUV_G4Endcap.hh"
#include "XY_G4Endcap.hh"

using namespace Silc;

// Constants.
static const TAngle DE_ROTATION_ANGLE = M_PI;

G4EndcapArray::EndcapTypeMap G4EndcapArray::InitializeEndcapTypeMap()
{
    EndcapTypeMap result;
    result[XUV_G4Endcap::ENDCAP_TYPE] = &XUV_G4Endcap::MakeInstance;
    result[XY_G4Endcap::ENDCAP_TYPE] = &XY_G4Endcap::MakeInstance;
    return result;
}

P<Endcap> G4EndcapArray::MakeEndcap(const Endcap::EndcapType& p_oEndcapType)
{
    static const EndcapTypeMap l_TypeMap = InitializeEndcapTypeMap();
    EndcapTypeMap::const_iterator iter = l_TypeMap.find(p_oEndcapType);
    assert(iter != l_TypeMap.end());
    P<G4Endcap> l_pEndcap = iter->second();
    m_mEndcapOrigins[l_pEndcap] = l_pEndcap;
    return l_pEndcap;
}

G4EndcapArray::G4EndcapArray(G4LogicalVolume& p_oG4World)
    : m_oG4World(p_oG4World)
{
}

void G4EndcapArray::Assemble()
{
    EndcapArray::Assemble();

    MakeDetectionElementAssembly();

    G4AssemblyVolume* m_pArrayAssembly = new G4AssemblyVolume();
    G4Transform3D l_g4Transform;
    m_pArrayAssembly->AddPlacedAssembly(m_pDetectionElementAssembly, l_g4Transform);

    if(HasMirrorImage())
    {
        G4RotationMatrix l_oRotationMatrix(0, DE_ROTATION_ANGLE, 0);
        G4ThreeVector l_oShiftVector;
        G4Transform3D l_oTransform(l_oRotationMatrix, l_oShiftVector);
        m_pArrayAssembly->AddPlacedAssembly(m_pDetectionElementAssembly, l_oTransform);
    }

    //    clog << "G4EndcapArray::Assemble : Placing endcap assembies... ";
    m_pArrayAssembly->MakeImprint( &m_oG4World, l_g4Transform, 0, false);
    // clog << "done." << endl;

}

vector<VSensitiveDetector*> G4EndcapArray::GetSensitiveDetectors() const
{
    vector<VSensitiveDetector*> l_vDetectors;
    for(EndcapSet::const_iterator iter = GetBaseEndcaps().begin(); iter != GetBaseEndcaps().end(); ++iter)
    {
        P<G4Endcap> l_pEndcap = m_mEndcapOrigins.find(iter->first)->second;
        for(unsigned l_uZoneId = 0; l_uZoneId < l_pEndcap->GetNumberOfZones(); ++l_uZoneId)
            l_vDetectors.push_back(l_pEndcap->GetSensitiveDetector(l_uZoneId));
    }
    return l_vDetectors;
}

void G4EndcapArray::SetDriverPrefix(const string & p_sPrefix)
{
    for(EndcapSet::iterator l_pIter = GetBaseEndcaps().begin(); l_pIter != GetBaseEndcaps().end(); ++l_pIter)
    {
        unsigned l_uEndcapId = l_pIter->second[0];
        string l_sEndcapType("num");
        stringstream l_sNewPrefix;
        l_sNewPrefix << p_sPrefix <<"_" << l_sEndcapType << "_"<< l_uEndcapId;
        m_mEndcapOrigins[l_pIter->first]->SetSensitiveVolumePrefix(l_sNewPrefix.str());
    }
}
/*
for(unsigned endcap=0; endcap<GetNumberOfEndcaps(); endcap++)
  {
  G4Endcap *l_oEndcap = m_vEndcaps[layer];
  stringstream l_sNewPrefix;
  l_sNewPrefix << p_sPrefix << "_LAYER_" << layer;
  l_oEndcap->SetLayerPrefix(l_sNewPrefix.str());
  }
  }
*/

void G4EndcapArray::MakeDetectionElementAssembly()
{
    m_pDetectionElementAssembly = new G4AssemblyVolume();

    for(unsigned l_uEndcapId = 0; l_uEndcapId < GetNumberOfEndcaps(); ++l_uEndcapId)
    {
        const EndcapDescriptor& l_oDescriptor = (*this)[l_uEndcapId];
        G4RotationMatrix l_oEndcapRotation(l_oDescriptor.RotationAngle, 0, 0);
        // ???
        G4ThreeVector l_oEndcapShift ( 0, 0, -l_oDescriptor.ZPosition );
        G4Transform3D l_oEndcapTranform(l_oEndcapRotation, l_oEndcapShift);
        m_pDetectionElementAssembly->AddPlacedAssembly(
            m_mEndcapOrigins[l_oDescriptor.EndcapObject]->GetAssemblyVolume(),
            l_oEndcapTranform);
    }
}

//void G4EndcapArray::PlaceDetectionElement(TAngle p_dRotation, TCoordinate p_dShift)
//{
//    G4RotationMatrix l_oRotationMatrix(0, p_dRotation, 0);
//    G4ThreeVector l_oShiftVector(0.0, 0.0, p_dShift);
//    G4Transform3D l_oTransform(l_oRotationMatrix, l_oShiftVector);
//    m_pEndcapAssembly->AddPlacedAssembly(m_pDetectionElementAssembly, l_oTransform);
//}


/// GEAR Patch
/// shouldn't be here :/

vector<TLength> G4EndcapArray::GetEndcapZPositions()
{
    vector<TLength> l_vdZPosition;
    for(unsigned endcap=0; endcap<GetNumberOfEndcaps(); endcap++)
    {
        for(unsigned layer=0; layer<(*this)[endcap].EndcapObject->GetNumberOfJoinedLayers(); layer++)
        {
            l_vdZPosition.push_back((*this)[endcap].ZPosition);
        }
    }
    return l_vdZPosition;
}
