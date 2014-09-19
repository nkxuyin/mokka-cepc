#include "EncoderSi.hh"

// LCIO Classes
#ifdef LCIO_MODE
#include <UTIL/ILDConf.h>
#endif

#include <sstream>
#include <cstdlib>

#include "G4NavigationHistory.hh"

/* EncoderSi

  J. Duarte Campderros, IFCA, 12:55 10/05/2010
*/

////////////////////////////////////////////////////////////////////
// 
// Using the new scheme: available bits for 
// DISKS (Layer) : 9 bits   ---> LSB
// PETALS(ladder): 8 bits
// SENSOR        : 8 bits
// SIDE          : 2 bits
//
//        ____________________________________________________________________
//       |         |SIDE||      SENSOR   ||     PETALS    ||        DISKS      |    WORD (32-b)
//  MSB  |_|_|_|_|_||_|_||_|_|_|_|_|_|_|_||_|_|_|_|_|_|_|_||_|_|_|_|_|_|_|_|_|_| LSB
//  

#define SHIFT_LAYER  0   // DISKS
#define SHIFT_LADDER 9   // LADDER
#define SHIFT_SENSOR 17  // SENSOR

#define NBITS_LAYER  (unsigned int)0x000001FF   // Number of bits
#define NBITS_LADDER (unsigned int)0x000000FF   // Number of bits
#define NBITS_SENSOR (unsigned int)0x000000FF   // Number of bits

#define MASK_LAYER   (unsigned int)0x000001FF   // DISKS
#define MASK_LADDER  (unsigned int)0x0001FE00   // LADDER
#define MASK_SENSOR  (unsigned int)0x01FE0000   // SENSOR


EncoderSi::EncoderSi()
{
	_id.subdet = ILDDetID::FTD;
	_id.side   = 0;
	_id.layer  = 0;
	_id.module = 0;
	_id.sensor = 0;
}

unsigned int EncoderSi::encode( const G4VTouchable * touchable )
{
	// The bit shift
	std::vector<int> shifts(3,0);
	shifts[2]=SHIFT_LAYER;   // layer 9 bits
	shifts[1]=SHIFT_LADDER;  // ladder 8 bits;
	shifts[0]=SHIFT_SENSOR;  // sensor 8 bits; 
	// The masks to simulate int of NBITS
	std::vector<unsigned int> masks(3,0);
	masks[2]=NBITS_LAYER;
	masks[1]=NBITS_LADDER;
	masks[0]=NBITS_SENSOR;
	// Never have more than 3 deep levels...

	unsigned int auxCopyNo = 0;
	const G4int depth = touchable->GetHistory()->GetDepth();

	// Order is from deeper volume to world volume
	int indx = 0;
	for(int i = 0; i < depth ; ++i )
	{
		G4int idcp = touchable->GetVolume(i)->GetCopyNo();
		// FIXME: This controls only if the first volume
		//        is zero!!
		if( idcp == 0 )
		{
			continue;
		}
		// Masking in order to deal with the negative values
		unsigned int code = idcp & masks.at(indx);
		auxCopyNo |= code << shifts.at(indx);
		indx++;
	}
	setfields(auxCopyNo);

	return auxCopyNo;
}

std::string EncoderSi::getCellIDStr()
{
	std::stringstream ss;
	ss << "subdet:" << _id.subdet 
		<< ",side:" << _id.side
		<< ",layer:" << _id.layer
		<< ",module:" << _id.module
		<< ",sensor:" << _id.sensor;
	
	std::string cellidstr = ss.str();

	return cellidstr;
}

void EncoderSi::setfields(const unsigned int & codeint)
{
	unsigned int layerid = (codeint & MASK_LAYER) >> SHIFT_LAYER;
	unsigned int ladderid= (codeint & MASK_LADDER) >> SHIFT_LADDER;
	unsigned int sensorid= (codeint & MASK_SENSOR) >> SHIFT_SENSOR;
	int side             = 1;

	// Checking the negative Z
	if( layerid > 7 )
	{
		// 
		layerid = (~layerid & NBITS_LAYER)+1;
		side = -1;
	}
	
	_id.side = side;
	_id.layer = layerid;
	_id.module= ladderid;
	_id.sensor= sensorid;
}


unsigned int EncoderSi::getlayer( const int & code )
{
	setfields(code);

	return getlayer();
}

unsigned int EncoderSi::getside( const int & code )
{
	setfields(code);

	return getside();
}

unsigned int EncoderSi::getmodule( const int & code )
{
	setfields(code);

	return getmodule();
}

unsigned int EncoderSi::getsensor( const int & code )
{
	setfields(code);

	return getsensor();
}
