// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: data.hh,v 0.0 2012/08/14 S.Lu Exp $
// Used by Mokka plugin to save the detail G4Track information

#ifndef IDG4TrackMapping_hh
#define IDG4TrackMapping_hh 1

class IDG4TrackMapping
{
public:
  IDG4TrackMapping() {}
  ~IDG4TrackMapping(void) {}


  inline G4int GetTrackID() const
  { return _dataTrackID; }

  inline G4int GetParentID() const
  { return _dataParentID; }//track ID of parent track

  inline G4int GetPDGcode() const
  { return _dataPDGcode; }

  inline G4double GetTimeStamp() const
  { return _dataTimeStamp; }

  inline G4double SetKineticEnergy() const
  { return _dataKineticEnergy; }
  
  inline G4double SetTotalEnergy() const
  { return _dataTotalEnergy; }
  
  inline G4String GetParticleName() const
  { return _dataParticleName; }//name of the particle track

  inline G4String GetProcessName() const
  { return _dataProcessName; }//name of the process creating the track
    
  inline G4String GetLogicalVolumeName() const
  { return _dataLogicalVolumeName; }

  inline bool IsHcalWholeSensLayerLogical() const  
  { return _dataIsHcalWholeSensLayerLogical;}

  void SetTrackID(G4int TrackID )
  { _dataTrackID = TrackID; }

  void SetParentID(G4int ParentID )
  { _dataParentID = ParentID; }

  void SetPDGcode(G4int PDGcode )
  { _dataPDGcode = PDGcode; }
  
  void SetTimeStamp(G4double TimeStamp )
  { _dataTimeStamp = TimeStamp; }
  
  void SetKineticEnergy(G4double KineticEnergy )
  { _dataKineticEnergy = KineticEnergy; }
  
  void SetTotalEnergy(G4double TotalEnergy )
  { _dataTotalEnergy = TotalEnergy; }
  
  void SetParticleName(const G4String& ParticleName )
  { _dataParticleName = ParticleName; }

  void SetProcessName(const G4String& ProcessName )
  { _dataProcessName = ProcessName; }
  
  void SetLogicalVolumeName(const G4String& LogicalVolumeName )
  { _dataLogicalVolumeName = LogicalVolumeName; }

  void SetHcalWholeSensLayerLogical(bool IsSensLayerLogical)
  { _dataIsHcalWholeSensLayerLogical = IsSensLayerLogical; }

  void PrintParameters()
  {

    G4cout <<std::setw ( 10 )<<_dataTrackID
	   <<std::setw ( 10 )<<_dataParentID
	   <<std::setw ( 15 )<<_dataPDGcode
	   <<std::setw ( 15 )<<_dataParticleName
	   <<std::setw ( 15 )<<_dataTimeStamp
	   <<std::setw ( 15 )<<_dataKineticEnergy
	   <<std::setw ( 15 )<<_dataTotalEnergy
	   <<std::setw ( 35 )<<_dataProcessName
	   <<std::setw ( 35 )<<_dataLogicalVolumeName 
	   <<std::setw ( 5 )<<_dataIsHcalWholeSensLayerLogical
	   <<G4endl;
  }

  void PrintParametersinSensitive()
  {

    if(_dataIsHcalWholeSensLayerLogical == true)
    G4cout <<std::setw ( 10 )<<_dataTrackID
	   <<std::setw ( 10 )<<_dataParentID
	   <<std::setw ( 15 )<<_dataPDGcode
	   <<std::setw ( 15 )<<_dataParticleName
	   <<std::setw ( 15 )<<_dataTimeStamp
	   <<std::setw ( 15 )<<_dataKineticEnergy
	   <<std::setw ( 15 )<<_dataTotalEnergy
	   <<std::setw ( 35 )<<_dataProcessName
	   <<std::setw ( 35 )<<_dataLogicalVolumeName 
	   <<std::setw ( 5 )<<_dataIsHcalWholeSensLayerLogical
	   <<G4endl;
  }

  void PrintParametersLabel()
  {

    G4cout <<std::setw ( 10 )<<" #TrackID"
	   <<std::setw ( 10 )<<" #id"
	   <<std::setw ( 15 )<<" #PDGEncoding"
	   <<std::setw ( 15 )<<" #ParticleName"
	   <<std::setw ( 15 )<<" #TimeStamp"
	   <<std::setw ( 15 )<<" #KineticEnergy"
	   <<std::setw ( 15 )<<" #TotalEnergy"
	   <<std::setw ( 35 )<<" #ProcessName"
	   <<std::setw ( 35 )<<" #LogicalVolumeName"
	   <<std::setw ( 5 )<<" #IsHcalWholeSensLayerLogical"
	   <<G4endl;

    G4cout <<std::setw ( 10 )<<" "
	   <<std::setw ( 10 )<<" "
	   <<std::setw ( 15 )<<" "
	   <<std::setw ( 15 )<<" "
	   <<std::setw ( 15 )<<" # (ns)"
	   <<std::setw ( 15 )<<" # (GeV)"
	   <<std::setw ( 15 )<<" # (GeV)"
	   <<std::setw ( 35 )<<" #(to create the track)"
	   <<std::setw ( 35 )<<" "
	   <<std::setw ( 5 )<<" # (1 = true)"
	   <<G4endl;
   
  }

private:

  G4int    _dataTrackID;
  G4int    _dataParentID;
  G4int    _dataPDGcode; 
  G4double _dataTimeStamp; //Global Time
  G4double _dataKineticEnergy;
  G4double _dataTotalEnergy;
  G4String _dataParticleName;
  G4String _dataProcessName;
  G4String _dataLogicalVolumeName;
  bool     _dataIsHcalWholeSensLayerLogical;

};

#endif
