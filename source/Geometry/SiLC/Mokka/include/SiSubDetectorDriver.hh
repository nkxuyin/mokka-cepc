/*! \file SiSubDetectorDriver.hh
    \brief A definition of Silc::SiSubDetectorDriver class.

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

#include "MokkaStore.hh"

namespace Silc
{
    template<typename SubDetector, typename SubDetectorSerializer>
    class SiSubDetectorDriver : public VSubDetectorDriver
    {
    public: // Type definitions.
        typedef void (*MokkaUpdateCallback)(const SubDetector&);
        typedef void (*GearSetupCallback)(const SubDetector&, const SubDetectorSerializer&);

        static void DefaultUpdate(const SubDetector&) { }
        static void DefaultSetup(const SubDetector&, const SubDetectorSerializer&) { }

    public:
        SiSubDetectorDriver(const string& p_sDriverName, bool p_bUseExternalParameters = true,
                            MokkaUpdateCallback p_pUpdateMokka = &DefaultUpdate,
                            GearSetupCallback p_pSetupGear = &DefaultSetup)
            : VSubDetectorDriver(p_sDriverName, p_sDriverName), m_bUseExternalParameters(p_bUseExternalParameters),
              m_pUpdateMokka(p_pUpdateMokka), m_pSetupGear(p_pSetupGear)
        { }

        virtual G4bool ContextualConstruct( const CGAGeometryEnvironment &aGeometryEnvironment,
                                            G4LogicalVolume *theWorld)
        {
            try
            {
                d_message(GetName() + " construction started.");
                assert(theWorld != nullptr);

                d_message("Loading configuration...");
                m_pSubDetector = P<SubDetector>(new SubDetector(*theWorld));
                MokkaReader l_oReader(aGeometryEnvironment);
                m_pSerializer = P<SubDetectorSerializer>(new SubDetectorSerializer(GetName()));
                m_pSerializer->Load(*m_pSubDetector, l_oReader, m_bUseExternalParameters);
                d_message("Configuration is loaded.");
                d_message(*m_pSubDetector);

                d_message("Starting assemblage...");
                m_pSubDetector->Assemble();
                const vector<VSensitiveDetector*> l_vDetectors = m_pSubDetector->GetSensitiveDetectors();
                typedef vector<VSensitiveDetector*>::const_iterator sd_iterator;
                for(sd_iterator iter=l_vDetectors.begin(); iter!=l_vDetectors.end(); ++iter)
                    RegisterSensitiveDetector(*iter);

                m_pUpdateMokka(*m_pSubDetector);

                d_message(GetName() +" construction done.");
                return true;
            }
            catch(Exception& e)
            {
                clog << e.GetMessage() << endl;
            }
            catch(std::exception& e)
            {
                clog << GetName() << " : Unexpected exception!" << endl << e.what() << endl;
            }

            clog << GetName() << " : Construction failed." << endl;
            return false;
        }

        virtual void GearSetup()
        {
            m_pSetupGear(*m_pSubDetector, *m_pSerializer);
        }

    private: // Data members.
        bool m_bUseExternalParameters;
        P<SubDetector> m_pSubDetector;
        P<SubDetectorSerializer> m_pSerializer;
        MokkaUpdateCallback m_pUpdateMokka;
        GearSetupCallback m_pSetupGear;
    };
}
