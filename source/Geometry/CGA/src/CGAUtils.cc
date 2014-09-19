//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//*    Mokka home page.                                 *
//*                                                     *
//*******************************************************
//
// $Id: CGAUtils.cc,v 1.3 2006/03/01 14:13:30 musat Exp $
// $Name: mokka-07-00 $
//
// History
// first implementation for the 
// Mokka Common Geometry Access (CGA) by 
// Gabriel Musat (musat@poly.in2p3.fr), July 2002
//
// see CGA documentation at 
// http://polype.in2p3.fr/geant4/tesla/www/mokka/
//        software/doc/CGADoc/CGAIndex.html
//-------------------------------------------------------

#include <stdio.h>
#include <string.h>

void endString(char *str, int sLen) {
        str[sLen-1] = '\0';
        for(int i=sLen-2; i>=0; --i) {
                if(str[i] != ' ') break;
                str[i] = '\0';
        }
}

void fillString(char *str, int sLen) {
        char * zero = strchr(str,'\0');
	
	if(zero == NULL) 
		return;

        for(char *sp = zero; sp < str+sLen; sp++)
                *sp = ' ';
}

