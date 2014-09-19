// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: Pi0Tracker.cc,v 0.0 2010/10/12 15:49:00 A.Kaplan Exp $
// $Name: mokka-07-00 $

#include <G4VSensitiveDetector.hh>
#include <G4VProcess.hh>
#include <G4ProcessType.hh>

#include <IMPL/LCGenericObjectImpl.h>

#include "Pi0Tracker.hh"

INITPLUGIN(Pi0Tracker, "Pi0Tracker")


//#define Pi0Tracker_DEBUG 1
//#define Pi0Tracker_STEPPING 1
//#define Pi0Tracker_HADTRACK 1


//----------------------------------------------------------------------------//
void Pi0Tracker::Init(void)
//----------------------------------------------------------------------------//
{
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void Pi0Tracker::Exit(void)
//----------------------------------------------------------------------------//
{
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void Pi0Tracker::BeginOfRunAction(const G4Run *run)
//----------------------------------------------------------------------------//
{
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void Pi0Tracker::EndOfRunAction(const G4Run *run)
//----------------------------------------------------------------------------//
{
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void Pi0Tracker::BeginOfEventAction(const G4Event *evt)
//----------------------------------------------------------------------------//
{
    _eEta       = 0.0;
    _nEta       = 0;
    _ePi0       = 0.0;
    _nPi0       = 0;
    _eEtaPhoton = 0.0; //photons from eta decay
    _nEtaPhoton = 0;

    _hcal_eEta       = 0.0;
    _hcal_nEta       = 0;
    _hcal_ePi0       = 0.0;
    _hcal_nPi0       = 0;
    _hcal_eEtaPhoton = 0.0; //photons from eta decay
    _hcal_nEtaPhoton = 0;

    _ecal_eEta       = 0.0;
    _ecal_nEta       = 0;
    _ecal_ePi0       = 0.0;
    _ecal_nPi0       = 0;
    _ecal_eEtaPhoton = 0.0; //photons from eta decay
    _ecal_nEtaPhoton = 0;

    _tcmt_eEta = 0.0;
    _tcmt_nEta = 0;
    _tcmt_ePi0 = 0.0;
    _tcmt_nPi0 = 0;
    _tcmt_eEtaPhoton = 0.0; //photons from eta decay
    _tcmt_nEtaPhoton = 0;
    
    _fhio=false; //first Hadron Interaction Occured?
    _stepno=0;
    _lastindex=0;

    _fhi_z=-123456789.0;
    _fhi_inEcal=false;
    _fhi_inHcal=false;
    _fhi_inTcmt=false;
    _fhi_pSubType=-666;
    _fhi_nSec=0;
    _fhi_secPdgCode.clear();
    _fhi_secEnergy.clear();
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void Pi0Tracker::EndOfEventAction(const G4Event *evt)
//----------------------------------------------------------------------------//
{
    Control* c = Control::GetControl();

    //set event parameters
#ifdef LCIO_MODE
    if(c->lcWrt && c->lcEvt) 
    {
        c->lcEvt->parameters().setValue( "pt_hcal_nPi0",(int)  _hcal_nPi0 );
        c->lcEvt->parameters().setValue( "pt_hcal_ePi0",(float)_hcal_ePi0 );
        c->lcEvt->parameters().setValue( "pt_hcal_nEta",(int)  _hcal_nEta );
        c->lcEvt->parameters().setValue( "pt_hcal_eEta",(float)_hcal_eEta );
        c->lcEvt->parameters().setValue( "pt_hcal_nEtaPhoton",
                                         (int)  _hcal_nEtaPhoton );
        c->lcEvt->parameters().setValue( "pt_hcal_eEtaPhoton",
                                         (float)_hcal_eEtaPhoton );

        c->lcEvt->parameters().setValue( "pt_ecal_nPi0",(int)  _ecal_nPi0 );
        c->lcEvt->parameters().setValue( "pt_ecal_ePi0",(float)_ecal_ePi0 );
        c->lcEvt->parameters().setValue( "pt_ecal_nEta",(int)  _ecal_nEta );
        c->lcEvt->parameters().setValue( "pt_ecal_eEta",(float)_ecal_eEta );
        c->lcEvt->parameters().setValue( "pt_ecal_nEtaPhoton",
                                         (int)  _ecal_nEtaPhoton );
        c->lcEvt->parameters().setValue( "pt_ecal_eEtaPhoton",
                                         (float)_ecal_eEtaPhoton );

        c->lcEvt->parameters().setValue( "pt_tcmt_nPi0",(int)  _tcmt_nPi0 );
        c->lcEvt->parameters().setValue( "pt_tcmt_ePi0",(float)_tcmt_ePi0 );
        c->lcEvt->parameters().setValue( "pt_tcmt_nEta",(int)  _tcmt_nEta );
        c->lcEvt->parameters().setValue( "pt_tcmt_eEta",(float)_tcmt_eEta );
        c->lcEvt->parameters().setValue( "pt_tcmt_nEtaPhoton",
                                         (int)  _tcmt_nEtaPhoton );
        c->lcEvt->parameters().setValue( "pt_tcmt_eEtaPhoton",
                                         (float)_tcmt_eEtaPhoton );

        c->lcEvt->parameters().setValue( "pt_nPi0",     (int)  _nPi0      );
        c->lcEvt->parameters().setValue( "pt_ePi0",     (float)_ePi0      );
        c->lcEvt->parameters().setValue( "pt_nEta",     (int)  _nEta      );
        c->lcEvt->parameters().setValue( "pt_eEta",     (float)_eEta      );
        c->lcEvt->parameters().setValue( "pt_nEtaPhoton",
                                         (int)  _nEtaPhoton      );
        c->lcEvt->parameters().setValue( "pt_eEtaPhoton",
                                         (float)_eEtaPhoton      );
        c->lcEvt->parameters().setValue( "pt_tPrimary", (int)  _tPrimary  );
        c->lcEvt->parameters().setValue( "pt_ePrimary", (float)_ePrimary  );
        
        c->lcEvt->parameters().setValue( "pt_fhi_z",        (float)_fhi_z     );
        c->lcEvt->parameters().setValue( "pt_fhi_inEcal",   (int)_fhi_inEcal  );
        c->lcEvt->parameters().setValue( "pt_fhi_inHcal",   (int)_fhi_inHcal  );
        c->lcEvt->parameters().setValue( "pt_fhi_inTcmt",   (int)_fhi_inTcmt  );
        c->lcEvt->parameters().setValue( "pt_fhi_pSubType", (int)_fhi_pSubType);
        c->lcEvt->parameters().setValue( "pt_fhi_nSec",     (int)_fhi_nSec    );

        LCCollection *col = new LCCollectionVec( LCIO::LCGENERICOBJECT );
        for( int i=0; i<_fhi_nSec; i++) {
//            G4cout
//                <<"adding generic object #"<<i
//                <<", pdgCode="<<_fhi_secPdgCode[i]
//                <<", energy="<<_fhi_secEnergy[i]
//                <<G4endl;
            LCGenericObjectImpl *o = new IMPL::LCGenericObjectImpl;
            o->setIntVal( 0,    (int)   _fhi_secPdgCode[i] );
            o->setDoubleVal( 0, (double)_fhi_secEnergy[i]  );
            col->addElement( o );
        }

        c->lcEvt->addCollection( col, "Pi0TrackerSec" );
    }
#endif

}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void Pi0Tracker::PreUserTrackingAction(const G4Track *trk)
//----------------------------------------------------------------------------//
{
    if( trk->GetParentID() == 0 ) {
        const G4ParticleDefinition* def = trk->GetDefinition();
        G4int pdgCode=def->GetPDGEncoding();

        _ePrimary = trk->GetTotalEnergy() / GeV;
        _tPrimary = pdgCode;
    }

#ifdef Pi0Tracker_HADTRACK
    const G4VProcess *p= trk->GetCreatorProcess();
    if(p) {
        if(p->GetProcessType()==fHadronic) {
            G4cout
                <<"trk "<<trk->GetTrackID()
                <<", pdgc=="<<trk->GetDefinition()->GetPDGEncoding()
                <<", parent=="<<trk->GetParentID()
                <<" from had int. @z == "<<trk->GetPosition().z()/mm
                <<G4endl;
        }
    }
#endif
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void Pi0Tracker::PostUserTrackingAction(const G4Track *trk)
//----------------------------------------------------------------------------//
{
    if(trk) {
        const G4ParticleDefinition* def = trk->GetDefinition();
        G4int pdgCode=def->GetPDGEncoding();

        if( pdgCode == ETA ) {
#ifdef Pi0Tracker_DEBUG
            G4cout<<"track "<<trk->GetTrackID()<<" (parent "
                  <<trk->GetParentID()
                  <<"), pdgcode=="<<trk->GetDefinition()->GetPDGEncoding()
                  <<", E_tot=="<<trk->GetTotalEnergy()/GeV
                  <<", step @ 0x"<<trk->GetStep()
                  <<", secondararies: "<<trk->GetStep()->GetSecondary()->size() 
                  <<" in physical volume "
                  <<trk->GetVolume()->GetName()
                   <<", in logical volume "
                  <<trk->GetVolume()->GetLogicalVolume()->GetName()
                  <<G4endl;

            G4TrackVector v = *trk->GetStep()->GetSecondary();
            for(unsigned i=0; i<v.size(); i++) {
                G4Track *chld = v[i];
                G4cout
                    <<"  "
                    <<"track "<<chld->GetTrackID()<<" (parent "
                    <<chld->GetParentID()
                    <<"), pdgcode=="<<chld->GetDefinition()->GetPDGEncoding()
                    <<", E_tot=="<<chld->GetTotalEnergy()/GeV
                    <<", step @ 0x"<<chld->GetStep()
                    <<" in physical volume "
                    <<chld->GetVolume()->GetName()
                    <<", in logical volume "
                    <<chld->GetVolume()->GetLogicalVolume()->GetName()
                    <<G4endl;
            }
#endif
            
            _eEta += trk->GetTotalEnergy() / GeV;
            _nEta ++;

            if( inHcal( trk ) ) {
                _hcal_eEta += trk->GetTotalEnergy() / GeV;
                _hcal_nEta ++;
            }

            if( inEcal( trk ) ) {
                _ecal_eEta += trk->GetTotalEnergy() / GeV;
                _ecal_nEta ++;
            }

            if( inTcmt( trk ) ) {
                _tcmt_eEta += trk->GetTotalEnergy() / GeV;
                _tcmt_nEta ++;
            }

            G4TrackVector v2 = *trk->GetStep()->GetSecondary();
            for(unsigned i=0; i<v2.size(); i++) {
                const G4Track *chld = v2[i];
                
                if( chld->GetDefinition()->GetPDGEncoding() == PHOTON ) {
                    _eEtaPhoton += chld->GetTotalEnergy() / GeV;
                    _nEtaPhoton ++;

                    if( inHcal( chld ) ) {
                        _hcal_eEtaPhoton += chld->GetTotalEnergy() / GeV;
                        _hcal_nEtaPhoton ++;
                    }
                    if( inEcal( chld ) ) {
                        _ecal_eEtaPhoton += chld->GetTotalEnergy() / GeV;
                        _ecal_nEtaPhoton ++;
                    }
                    if( inTcmt( chld ) ) {
                        _tcmt_eEtaPhoton += chld->GetTotalEnergy() / GeV;
                        _tcmt_nEtaPhoton ++;
                    }

                }
            }

        }

        if( pdgCode == PI0 ) {
#ifdef Pi0Tracker_DEBUG
            G4cout
                <<"track "<<trk->GetTrackID()<<" (parent "
                <<trk->GetParentID()
                <<"), pdgcode=="<<trk->GetDefinition()->GetPDGEncoding()
                <<", E_tot=="<<trk->GetTotalEnergy()/GeV
                <<", step @ 0x"<<trk->GetStep()
                <<", secondararies: "<<trk->GetStep()->GetSecondary()->size() 
                <<" in physical volume "
                <<trk->GetVolume()->GetName()
                <<", in logical volume "
                <<trk->GetVolume()->GetLogicalVolume()->GetName()
                <<G4endl;

            G4TrackVector v = *trk->GetStep()->GetSecondary();
            for(unsigned i=0; i<v.size(); i++) {
                G4Track *chld = v[i];
                G4cout
                    <<"  "
                    <<"track "<<chld->GetTrackID()<<" (parent "
                    <<chld->GetParentID()
                    <<"), pdgcode=="<<chld->GetDefinition()->GetPDGEncoding()
                    <<", E_tot=="<<chld->GetTotalEnergy()/GeV
                    <<", step @ 0x"<<chld->GetStep()
                    <<" in physical volume "
                    <<chld->GetVolume()->GetName()
                    <<", in logical volume "
                    <<chld->GetVolume()->GetLogicalVolume()->GetName()
                    <<G4endl;
            }
#endif

            _ePi0 += trk->GetTotalEnergy() / GeV;
            _nPi0 ++;

            if( inHcal( trk ) ) {
                _hcal_ePi0 += trk->GetTotalEnergy() / GeV;
                _hcal_nPi0 ++;
            }

            if( inEcal( trk ) ) {
                _ecal_ePi0 += trk->GetTotalEnergy() / GeV;
                _ecal_nPi0 ++;
            }

            if( inTcmt( trk ) ) {
                _tcmt_ePi0 += trk->GetTotalEnergy() / GeV;
                _tcmt_nPi0 ++;
            }
           
        }

        if( pdgCode == PHOTON ) {
#ifdef Pi0Tracker_DEBUG
            G4cout<<"track "<<trk->GetTrackID()<<" (parent "
                  <<trk->GetParentID()
                  <<"), pdgcode=="<<trk->GetDefinition()->GetPDGEncoding()
                  <<" in physical volume "
                  <<trk->GetVolume()->GetName()
                  <<", in logical volume "
                  <<trk->GetVolume()->GetLogicalVolume()->GetName()
                  <<", creatorProcess=="
                  <<trk->GetCreatorProcess()->GetProcessName()
                  <<", processType=="
                  <<trk->GetCreatorProcess()->GetProcessType()
                  <<", processSubType=="
                  <<trk->GetCreatorProcess()->GetProcessSubType()
                  <<G4endl;
#endif
        }
    }
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
bool Pi0Tracker::inEcal( const G4Track *&trk )
//----------------------------------------------------------------------------//
{
    G4VPhysicalVolume  *pv = trk->GetVolume();
    G4LogicalVolume   *mlv = pv->GetMotherLogical();
    
    if( mlv ) {
        G4String mlvname = mlv->GetName();
        if( (mlvname.index("CarbonFiber") == 0) ) {
#ifdef Pi0Tracker_DEBUG
            G4cout<<" is in ECAL"<<G4endl;
#endif
            return true;
        }
        if( (mlvname.index("StructureLogical") == 0) ) {
#ifdef Pi0Tracker_DEBUG
            G4cout<<" is in ECAL"<<G4endl;
#endif
            return true;
        }
        if( (mlvname.index("Air") == 0) && mlvname.contains("") ) {
#ifdef Pi0Tracker_DEBUG
            G4cout<<" is in ECAL"<<G4endl;
#endif
            return true;
        }
    }

#ifdef Pi0Tracker_DEBUG
    G4cout<<" is not in ECAL"<<G4endl;
#endif
    return false;
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
bool Pi0Tracker::inTcmt( const G4Track *&trk )
//----------------------------------------------------------------------------//
{
    G4VPhysicalVolume  *pv = trk->GetVolume();
    G4String pvname = pv->GetName();

    if( pvname.index("pv_")==0 ) {
#ifdef Pi0Tracker_DEBUG
        G4cout<<" is in TCMT"<<G4endl;
#endif
        return true;
    }
    if( pvname.index("Catcher")==0 ) {
#ifdef Pi0Tracker_DEBUG
        G4cout<<" is in TCMT"<<G4endl;
#endif
        return true;
    }
        
#ifdef Pi0Tracker_DEBUG
    G4cout<<" is not in TCMT"<<G4endl;
#endif

    return false;
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
bool Pi0Tracker::inHcal( const G4Track *&trk )
//----------------------------------------------------------------------------//
{
    G4VPhysicalVolume  *pv = trk->GetVolume();
    G4LogicalVolume    *lv = pv->GetLogicalVolume();
    G4LogicalVolume   *mlv = pv->GetMotherLogical();
    
    G4String vname = lv->GetName();
    if( (vname.index("HcalAbsLayerLogical") == 0) && (vname(19)!='_') ) {
#ifdef Pi0Tracker_DEBUG
        G4cout<<" is in HCAL"<<G4endl;
#endif
        return true;
    }

    if( mlv ) {
        G4String mlvname = mlv->GetName();
        if( (mlvname.index("WholeScinCass") == 0) ) {
#ifdef Pi0Tracker_DEBUG
            G4cout<<" is in HCAL"<<G4endl;
#endif
            return true;
        }
    }

    //G4cout<<"pi in physical volume: "
    //      <<pv->GetName()
    //      <<" in logical volume: "
    //      <<lv->GetName();
    //if(lv->GetSensitiveDetector()) {
    //    G4cout<<" in sensitive detector: "
    //          <<lv->GetSensitiveDetector()->GetName();
    //}
    //G4cout<<G4endl;

    //if(mlv) {
    //    G4cout<<"    in mother logical volume: "
    //          <<mlv->GetName();

    //    if(mlv->GetSensitiveDetector()) {
    //        G4cout<<" in sensitive detector: "
    //              <<mlv->GetSensitiveDetector()->GetName();
    //    }
    //    G4cout<<G4endl;
    //}
    //G4cout<<G4endl;

#ifdef Pi0Tracker_DEBUG
    G4cout<<" is not in HCAL"<<G4endl;
#endif

    return false;
}
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
void Pi0Tracker::UserSteppingAction(const G4Step *step)
//----------------------------------------------------------------------------//
{
    if( !_fhio ) {

        const G4Track *trk = step->GetTrack();
        if( trk->GetParentID() == 0 ) {
            _stepno++;

            G4TrackVector children = *step->GetSecondary();

            unsigned i=0;
            for( i=_lastindex; i<children.size(); ++i ) {
                G4Track* c = children[i];
                const G4VProcess* p = c->GetCreatorProcess(); 

                if(p->GetProcessType() == fHadronic) {
                    const G4ParticleDefinition* cDef = c->GetDefinition();
                    G4int cPdgCode=cDef->GetPDGEncoding();

                     ++_fhi_nSec; //increase number of secondaries
                    _fhi_secPdgCode.push_back( cPdgCode );
                     _fhi_secEnergy.push_back( c->GetTotalEnergy() );
                    _fhi_pSubType = p->GetProcessSubType();

                    if(!_fhio) {
#ifdef Pi0Tracker_STEPPING
                        G4cout
                            <<G4endl
                            <<"**** first hadron interaction after primary step"
                            <<_stepno
                            <<" ****"
                            <<G4endl;
#endif
                       _fhio=true;
                       _fhi_z=trk->GetPosition().z() / mm;
                       _fhi_inEcal = inEcal(trk);
                       _fhi_inHcal = inHcal(trk);
                       _fhi_inTcmt = inTcmt(trk);
                    }

#ifdef Pi0Tracker_STEPPING
                    G4cout
                        <<"  trk "<<c->GetTrackID()
                        <<", pdgcode=="<<c->GetDefinition()->GetPDGEncoding()
                        <<", E_tot=="<<c->GetTotalEnergy()/GeV
                        <<", cProc=="<<p->GetProcessName()
                        <<", type=="<<p->GetProcessType()
                        <<", sub=="<<p->GetProcessSubType()
                        <<", z=="<<c->GetPosition().z() / mm<<" mm"
                        <<G4endl;
#endif
               }
            }
            _lastindex=i;
        }

    }
}
//----------------------------------------------------------------------------//

