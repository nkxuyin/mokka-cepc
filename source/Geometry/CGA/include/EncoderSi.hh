#ifndef EncoderSi_h
#define EncoderSi_h 1

#include<cstring>

#include "globals.hh"
#include "G4VTouchable.hh"
#include "VEncoder.hh"


struct BIT_FIELDS
{
	unsigned int subdet : 5;
	         int side   : 2;
	unsigned int layer  : 9;
	unsigned int module : 8;
	unsigned int sensor : 8;
};

class EncoderSi
{
	public:
		EncoderSi();
		~EncoderSi() { ; }

		unsigned int encode(const G4VTouchable * touchable );

		unsigned int getlayer()  { return _id.layer; }
		unsigned int getlayer(const int & code);
		unsigned int getside()  { return _id.side; }
		unsigned int getside(const int & code);
		unsigned int getmodule() { return _id.module; }
		unsigned int getmodule(const int & code);
		unsigned int getsensor() { return _id.sensor; }
		unsigned int getsensor(const int & code);

		BIT_FIELDS getCellID() { return _id; }
		std::string getCellIDStr();

	private:
		void setfields( const unsigned int & codeint );

		BIT_FIELDS _id;


};



		

/* OLD
unsigned int encodeSi( const G4VTouchable * touchable );
G4int decodeSi( const G4int & id );*/

#endif
