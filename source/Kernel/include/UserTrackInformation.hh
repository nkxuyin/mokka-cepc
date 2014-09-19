#ifndef UserTrackInformation_h
#define UserTrackInformation_h 1

#include "globals.hh"
#include "G4Allocator.hh"
#include "G4VUserTrackInformation.hh"
#include "TrackSummary.hh"

class UserTrackInformation : public G4VUserTrackInformation 
{
public:
  UserTrackInformation(TrackSummary * aTrackSummary)
  { theTrackSummary = aTrackSummary; }
  
  virtual ~UserTrackInformation(){}
  
  inline void *operator new(size_t);
  inline void operator delete(void *aTrackInfo);
  
  TrackSummary* GetTheTrackSummary() const
  { return theTrackSummary; }

//   void SetTheTrackSummary(TrackSummary * aTrackSummary)
//   { theTrackSummary = aTrackSummary; }
  
  void Print() const {}

private:
  TrackSummary *theTrackSummary;
};

extern G4Allocator<UserTrackInformation> aTrackInformationAllocator;

inline void* UserTrackInformation::operator new(size_t)
{ void* aTrackInfo;
  aTrackInfo = (void*)aTrackInformationAllocator.MallocSingle();
  return aTrackInfo;
}

inline void UserTrackInformation::operator delete(void *aTrackInfo)
{ aTrackInformationAllocator.FreeSingle((UserTrackInformation*)aTrackInfo);}

#endif

