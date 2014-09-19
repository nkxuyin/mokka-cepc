/*! \file BarrelSerializer.hh
    \brief A definition of Silc::BarrelSerializer class.

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

#include "BarrelArray.hh"

namespace Silc
{
    /// Provides a serialization functionality to save and load Barrel configuration.
    /** Silc::BarrelSerializer uses generic save/load methods, so it can be used with different Reader and Writer
        implementations. */
    class BarrelSerializer
    {
    private: // Constants.
        static const char* BARREL_TYPE;
        static const char* BARREL_RADIUS;
        static const char* BARREL_LENGTH;
        static const char* BARREL_RADIAL_ADJUST;
        static const char* BARREL_LONGITUDINAL_ADJUST;

        static const char* BARREL_SUPPORT_ENABLE;
        static const char* BARREL_SUPPORT_MATERIAL;
        static const char* BARREL_SUPPORT_THICKNESS;

        static const char* BARREL_SENSOR_NB_NODES_X;
        static const char* BARREL_SENSOR_NB_NODES_Y;
        static const char* BARREL_SENSOR_PITCH_X;
        static const char* BARREL_SENSOR_PITCH_Y;
        static const char* BARREL_SENSOR_NODE_SIZE_X;
        static const char* BARREL_SENSOR_NODE_SIZE_Y;
        static const char* BARREL_SENSOR_DIM_X;
        static const char* BARREL_SENSOR_DIM_Y;
        static const char* BARREL_SENSOR_DIM_Z;

        static const char* BARREL_NB_CHIPS;
        static const char* BARREL_CHIP_NB_CHANNELS;
        static const char* BARREL_CHIPS_PER_ROW;
        static const char* BARREL_CHIP_X_DIMENSIONS;
        static const char* BARREL_CHIP_Y_DIMENSIONS;
        static const char* BARREL_CHIP_Z_DIMENSIONS;

        static const char* BARREL_SENSORS_PER_MODULE_X;
        static const char* BARREL_SENSORS_PER_MODULE_Y;
        static const char* BARREL_MODULE_GAPS_X;
        static const char* BARREL_MODULE_GAPS_Y;

        static const char* BARREL_MODULE_SUPPORT_ENABLE;
        static const char* BARREL_MODULE_SUPPORT_THICKNESS;
        static const char* BARREL_MODULE_SUPPORT_WIDTH;
        static const char* BARREL_MODULE_SUPPORT_EDGE;

        static const char* BARREL_MODULE_DIRECTION_ANGLE;
        static const char* BARREL_MODULE_FACE_ROTATION_ANGLE;

        static const char* SENSOR_MATERIAL_NAME;
        static const char* SENSOR_MATERIAL_RADIATION_LENGTH;
        static const char* MODULE_SUPPORT_MATERIAL_NAME;
        static const char* MODULE_SUPPORT_MATERIAL_RADIATION_LENGTH;

        // External parameters.
        static const char* TPC_OUTER_RADIUS;
        static const char* TPC_ECAL_HCAL_HALF;

    public: // Basic methods.
        /// The Silc::BarrelSerializer constructor.
        BarrelSerializer(const string& p_sSubDetectorName) throw()
            : m_sSubDetectorName(p_sSubDetectorName) {}

        /// Returns a name of the sub-detector.
        const string& GetSubDetectorName() const throw()
        {
            return m_sSubDetectorName;
        }

        /// Saves Silc::BarrelArray configuration using generic data writer.
        template<typename WriterType>
        void Save(const BarrelArray& p_oBarrelArray, WriterType& p_oWriter) const
        throw(Exception, std_ext::out_of_range_exception)
        {
            assert(p_oBarrelArray.GetNumberOfBarrels() > 0);
            const unsigned l_uNumberOfBarrels = p_oBarrelArray.GetNumberOfBarrels();
            vector<string> l_vsBarrelTypes(l_uNumberOfBarrels);
            vector<unsigned> l_vuNbFaces(l_uNumberOfBarrels);
            vector<double> l_vdBarrelRadius(l_uNumberOfBarrels);
            vector<double> l_vdBarrelLength(l_uNumberOfBarrels);
            vector<unsigned> l_vuNbLayersPerBarrel(l_uNumberOfBarrels);
            vector<string> l_vsBarrelRadialAdjust(l_uNumberOfBarrels);
            vector<string> l_vsBarrelLongitudinalAdjust(l_uNumberOfBarrels);
            vector<string> l_vsBarrelSupportEnable(l_uNumberOfBarrels);
            vector<string> l_vsBarrelSupportMaterial(l_uNumberOfBarrels);
            vector<double> l_vdBarrelSupportThickness(l_uNumberOfBarrels);

            for(unsigned n = 0; n < l_uNumberOfBarrels; ++n)
            {
                const Barrel& l_oBarrel         = p_oBarrelArray[n];
                l_vsBarrelTypes[n]              = l_oBarrel.GetBarrelType();
                l_vuNbFaces[n]                  = l_oBarrel.GetNbFace();
                l_vdBarrelRadius[n]             = l_oBarrel.GetInitialInnerRadius()
                                                  * p_oWriter.MetricalUnitsCorrection();
                l_vdBarrelLength[n]             = l_oBarrel.GetLength() * p_oWriter.MetricalUnitsCorrection();
                l_vuNbLayersPerBarrel[n]        = l_oBarrel.GetNbLayers();
                l_vsBarrelRadialAdjust[n]       = l_oBarrel.GetRadialAdjustment();
                l_vsBarrelLongitudinalAdjust[n] = l_oBarrel.GetLongitudinalAdjustment();
                l_vsBarrelSupportEnable[n]      = l_oBarrel.IsSupportEnable() ? "enabled" : "disabled";
                l_vsBarrelSupportMaterial[n]    = l_oBarrel.GetBarrelSupportMaterial();
                l_vdBarrelSupportThickness[n]   = l_oBarrel.GetBarrelSupportThickness();
            }

            p_oWriter.Write( MakeKey(BARREL_TYPE),                   l_vsBarrelTypes);
            p_oWriter.Write( MakeKey(BARREL_RADIUS),                 l_vdBarrelRadius);
            p_oWriter.Write( MakeKey(BARREL_LENGTH),                 l_vdBarrelLength);
            p_oWriter.Write( MakeKey(BARREL_RADIAL_ADJUST),          l_vsBarrelRadialAdjust);
            p_oWriter.Write( MakeKey(BARREL_LONGITUDINAL_ADJUST),    l_vsBarrelLongitudinalAdjust);
            p_oWriter.Write( MakeKey(BARREL_SUPPORT_ENABLE),         l_vsBarrelSupportEnable);
            p_oWriter.Write( MakeKey(BARREL_SUPPORT_MATERIAL),       l_vsBarrelSupportMaterial);
            p_oWriter.Write( MakeKey(BARREL_SUPPORT_THICKNESS),      l_vdBarrelSupportThickness);

            const unsigned l_uNumberOfLayers = p_oBarrelArray.GetTotalNumberOfLayers();
            vector<unsigned> l_vuNbChannelX(l_uNumberOfLayers);
            vector<unsigned> l_vuNbChannelY(l_uNumberOfLayers);
            vector<double> l_vdSensorPitchX(l_uNumberOfLayers);
            vector<double> l_vdSensorPitchY(l_uNumberOfLayers);
            vector<double> l_vdSensorWidthX(l_uNumberOfLayers);
            vector<double> l_vdSensorWidthY(l_uNumberOfLayers);
            vector<double> l_vdSensorDimX(l_uNumberOfLayers);
            vector<double> l_vdSensorDimY(l_uNumberOfLayers);
            vector<double> l_vdSensorDimZ(l_uNumberOfLayers);
            vector<string> l_vsSensorMaterial(l_uNumberOfLayers);
            vector<double> l_vdSensorMaterialRadLen(l_uNumberOfLayers);
            vector<unsigned> l_vuNbChipChannels(l_uNumberOfLayers);
            vector<unsigned> l_vuNbChips(l_uNumberOfLayers);
            vector<unsigned> l_vuChipRows(l_uNumberOfLayers);
            vector<double> l_vdChipDimX(l_uNumberOfLayers);
            vector<double> l_vdChipDimY(l_uNumberOfLayers);
            vector<double> l_vdChipDimZ(l_uNumberOfLayers);
            vector<unsigned> l_vuNbSensorX(l_uNumberOfLayers);
            vector<unsigned> l_vuNbSensorY(l_uNumberOfLayers);
            vector<double> l_vdGapX(l_uNumberOfLayers);
            vector<double> l_vdGapY(l_uNumberOfLayers);
            vector<string> l_vsModuleSupportEnable(l_uNumberOfLayers);
            vector<double> l_vdModuleSupportThickness(l_uNumberOfLayers);
            vector<double> l_vdModuleSupportWidth(l_uNumberOfLayers);
            vector<double> l_vdModuleSupportEdge(l_uNumberOfLayers);
            vector<double> l_vdModuleDirectionAngle(l_uNumberOfLayers);
            vector<double> l_vdModuleDirectionFace(l_uNumberOfLayers);
            vector<string> l_vsModuleSupportMaterial(l_uNumberOfLayers);
            vector<double> l_vdModuleSupportRadLen(l_uNumberOfLayers);


            for(unsigned n = 0; n < l_uNumberOfLayers; ++n)
            {
                const BarrelArray::LayerDescriptor& l_oLayerDescriptor = p_oBarrelArray.GetLayerDescriptor(n);
                const Barrel& l_oBarrel                     = p_oBarrelArray[l_oLayerDescriptor.BarrelId];
                const Module& l_oModule                     = l_oBarrel.GetModulePrototype(l_oLayerDescriptor.LayerId);
                const Module::SensorArray& l_oSensorArray   = l_oModule.GetSensorArray();
                const Sensor& l_oSensor                     = l_oSensorArray.GetSensorPrototype();
                const Module::ChipArray& l_oChipArray       = l_oModule.GetChipArray();
                const Module::Support& l_oSupport           = l_oModule.GetSupport();
                l_vuNbChannelX[n]                           = l_oSensor.GetSensitiveNodeDistribution().x;
                l_vuNbChannelY[n]                           = l_oSensor.GetSensitiveNodeDistribution().y;
                l_vdSensorPitchX[n]                         = l_oSensor.GetPitchSize().x
                        * p_oWriter.MetricalUnitsCorrection();
                l_vdSensorPitchY[n]                         = l_oSensor.GetPitchSize().y
                        * p_oWriter.MetricalUnitsCorrection();
                l_vdSensorWidthX[n]                         = l_oSensor.GetSensitiveNodeSize().x
                        * p_oWriter.MetricalUnitsCorrection();
                l_vdSensorWidthY[n]                         = l_oSensor.GetSensitiveNodeSize().y
                        * p_oWriter.MetricalUnitsCorrection();
                l_vdSensorDimX[n]                           = l_oSensor.GetSensorSize().x
                        * p_oWriter.MetricalUnitsCorrection();
                l_vdSensorDimY[n]                           = l_oSensor.GetSensorSize().y
                        * p_oWriter.MetricalUnitsCorrection();
                l_vdSensorDimZ[n]                           = l_oSensor.GetSensorSize().z
                        * p_oWriter.MetricalUnitsCorrection();
                l_vsSensorMaterial[n]                       = l_oSensor.GetMaterialName();
                if(l_oSensor.HasMaterialProperties())
                    l_vdSensorMaterialRadLen[n]             = l_oSensor.GetRadiationLength();
                l_vuNbChipChannels[n]                       = l_oChipArray.GetNumberOfChannelsPerChip();
                l_vuNbChips[n]                              = l_oChipArray.GetNumberOfChips();
                l_vuChipRows[n]                             = l_oChipArray.GetChipDistribution().y;
                l_vdChipDimX[n]                             = l_oChipArray.GetChipSize().x
                        * p_oWriter.MetricalUnitsCorrection();
                l_vdChipDimY[n]                             = l_oChipArray.GetChipSize().y
                        * p_oWriter.MetricalUnitsCorrection();
                l_vdChipDimZ[n]                             = l_oChipArray.GetChipSize().z
                        * p_oWriter.MetricalUnitsCorrection();
                l_vuNbSensorX[n]                            = l_oSensorArray.GetNumberOfSensors().x;
                l_vuNbSensorY[n]                            = l_oSensorArray.GetNumberOfSensors().y;
                l_vdGapX[n]                                 = l_oSensorArray.GetGapBetweenSensors().x
                        * p_oWriter.MetricalUnitsCorrection();
                l_vdGapY[n]                                 = l_oSensorArray.GetGapBetweenSensors().y
                        * p_oWriter.MetricalUnitsCorrection();
                l_vsModuleSupportEnable[n]                  = l_oSupport.IsEnabled() ? "enabled" : "disabled";
                l_vdModuleSupportThickness[n]               = l_oSupport.GetThickness()
                        * p_oWriter.MetricalUnitsCorrection();
                l_vdModuleSupportWidth[n]                   = l_oSupport.GetWidth()
                        * p_oWriter.MetricalUnitsCorrection();
                l_vdModuleSupportEdge[n]                    = l_oSupport.GetStandoffFromEdge()
                        * p_oWriter.MetricalUnitsCorrection();
                l_vsModuleSupportMaterial[n]                = l_oSupport.GetMaterialName();
                if(l_oSupport.HasMaterialProperties())
                    l_vdModuleSupportRadLen[n]              = l_oSupport.GetRadiationLength();
                l_vdModuleDirectionAngle[n]                 = l_oBarrel.GetModuleDirection(l_oLayerDescriptor.LayerId)
                        * p_oWriter.AngularUnitsCorrection();
                l_vdModuleDirectionFace[n]                  = l_oBarrel.GetModuleFace(l_oLayerDescriptor.LayerId)
                        * p_oWriter.AngularUnitsCorrection();
            }

            p_oWriter.Write( MakeKey(BARREL_SENSOR_NB_NODES_X),      l_vuNbChannelX);
            p_oWriter.Write( MakeKey(BARREL_SENSOR_NB_NODES_Y),      l_vuNbChannelY);
            p_oWriter.Write( MakeKey(BARREL_SENSOR_PITCH_X),            l_vdSensorPitchX);
            p_oWriter.Write( MakeKey(BARREL_SENSOR_PITCH_Y),            l_vdSensorPitchY);
            p_oWriter.Write( MakeKey(BARREL_SENSOR_NODE_SIZE_X),            l_vdSensorWidthX);
            p_oWriter.Write( MakeKey(BARREL_SENSOR_NODE_SIZE_Y),            l_vdSensorWidthY);
            p_oWriter.Write( MakeKey(BARREL_SENSOR_DIM_X),              l_vdSensorDimX);
            p_oWriter.Write( MakeKey(BARREL_SENSOR_DIM_Y),              l_vdSensorDimY);
            p_oWriter.Write( MakeKey(BARREL_SENSOR_DIM_Z),              l_vdSensorDimZ);
            p_oWriter.Write( MakeKey(SENSOR_MATERIAL_NAME),              l_vsSensorMaterial);
            p_oWriter.Write( MakeKey(SENSOR_MATERIAL_RADIATION_LENGTH), l_vdSensorMaterialRadLen);
            p_oWriter.Write( MakeKey(BARREL_CHIP_NB_CHANNELS),          l_vuNbChipChannels);
            p_oWriter.Write( MakeKey(BARREL_NB_CHIPS),                  l_vuNbChips);
            p_oWriter.Write( MakeKey(BARREL_CHIPS_PER_ROW),             l_vuChipRows);
            p_oWriter.Write( MakeKey(BARREL_CHIP_X_DIMENSIONS),         l_vdChipDimX);
            p_oWriter.Write( MakeKey(BARREL_CHIP_Y_DIMENSIONS),         l_vdChipDimY);
            p_oWriter.Write( MakeKey(BARREL_CHIP_Z_DIMENSIONS),         l_vdChipDimZ);
            p_oWriter.Write( MakeKey(BARREL_SENSORS_PER_MODULE_X),       l_vuNbSensorX);
            p_oWriter.Write( MakeKey(BARREL_SENSORS_PER_MODULE_Y),       l_vuNbSensorY);
            p_oWriter.Write( MakeKey(BARREL_MODULE_GAPS_X),             l_vdGapX);
            p_oWriter.Write( MakeKey(BARREL_MODULE_GAPS_Y),             l_vdGapY);
            p_oWriter.Write( MakeKey(BARREL_MODULE_SUPPORT_ENABLE),     l_vsModuleSupportEnable);
            p_oWriter.Write( MakeKey(BARREL_MODULE_SUPPORT_THICKNESS),  l_vdModuleSupportThickness);
            p_oWriter.Write( MakeKey(BARREL_MODULE_SUPPORT_WIDTH),      l_vdModuleSupportWidth);
            p_oWriter.Write( MakeKey(BARREL_MODULE_SUPPORT_EDGE),       l_vdModuleSupportEdge);
            p_oWriter.Write( MakeKey(MODULE_SUPPORT_MATERIAL_NAME),              l_vsModuleSupportMaterial);
            p_oWriter.Write( MakeKey(MODULE_SUPPORT_MATERIAL_RADIATION_LENGTH), l_vdModuleSupportRadLen);
            p_oWriter.Write( MakeKey(BARREL_MODULE_DIRECTION_ANGLE),    l_vdModuleDirectionAngle);
            p_oWriter.Write( MakeKey(BARREL_MODULE_FACE_ROTATION_ANGLE),     l_vdModuleDirectionFace);
        }

        /// Loads Silc::BarrelArray configuration using generic data reader.
        /** p_bUseExternalParameters indicates if a configuration with the external contriants should be loaded,
            or if only propre configuration parameters should be used. */
        template<typename ReaderType>
        void Load(BarrelArray& p_oBarrelArray, ReaderType& p_oReader, bool p_bUseExternalParameters = false)
        {
            vector<string> l_vsBarrelTypes;
            const unsigned l_uNumberOfBarrels = p_oReader.ReadList(MakeKey(BARREL_TYPE), l_vsBarrelTypes);

            vector<double> l_vdBarrelRadius;
            p_oReader.ReadList(MakeKey(BARREL_RADIUS), l_vdBarrelRadius, l_uNumberOfBarrels);

            if(p_bUseExternalParameters)
            {
                double l_dRadiusTPC;
                p_oReader.Read(TPC_OUTER_RADIUS, l_dRadiusTPC);
                l_vdBarrelRadius.at(l_uNumberOfBarrels-1) = l_dRadiusTPC;

            }

            vector<double> l_vdBarrelLength;
            p_oReader.ReadList(MakeKey(BARREL_LENGTH), l_vdBarrelLength, l_uNumberOfBarrels);
            if(p_bUseExternalParameters)
            {
                double l_dTPCLength;
                p_oReader.Read(TPC_ECAL_HCAL_HALF, l_dTPCLength);
                l_vdBarrelLength.at(l_uNumberOfBarrels-1) = l_dTPCLength*2.;
            }

            vector<unsigned> l_vuBarrelLayerRef;
            vector<unsigned> l_vuNbLayersPerBarrel;
            l_vuBarrelLayerRef.assign(l_uNumberOfBarrels, 0);
            l_vuNbLayersPerBarrel.assign(l_uNumberOfBarrels, 2);// TEMPORARY CHANGE

            unsigned l_uNbLayers = 0;
            for(unsigned barrel=0; barrel<l_uNumberOfBarrels; barrel++)
            {
                l_vuBarrelLayerRef[barrel] = l_uNbLayers;
                l_uNbLayers += l_vuNbLayersPerBarrel[barrel];
            }

            vector<string> l_vsBarrelRadialAdjust;
            p_oReader.ReadList(MakeKey(BARREL_RADIAL_ADJUST), l_vsBarrelRadialAdjust, l_uNumberOfBarrels);

            vector<string> l_vsBarrelLongitudinalAdjust;
            p_oReader.ReadList(MakeKey(BARREL_LONGITUDINAL_ADJUST), l_vsBarrelLongitudinalAdjust, l_uNumberOfBarrels);

            vector<string> l_vsBarrelSupportEnable;
            p_oReader.ReadList(MakeKey(BARREL_SUPPORT_ENABLE), l_vsBarrelSupportEnable, l_uNumberOfBarrels);

            vector<string> l_vsBarrelSupportMaterial;
            p_oReader.ReadList(MakeKey(BARREL_SUPPORT_MATERIAL), l_vsBarrelSupportMaterial, l_uNumberOfBarrels);

            vector<double> l_vdBarrelSupportThickness;
            p_oReader.ReadList(MakeKey(BARREL_SUPPORT_THICKNESS), l_vdBarrelSupportThickness, l_uNumberOfBarrels);

            vector<string> l_vsSensorMaterialNames;
            p_oReader.ReadList(MakeKey(SENSOR_MATERIAL_NAME), l_vsSensorMaterialNames, l_uNbLayers);

            vector<unsigned> l_vuNbNodesX;
            p_oReader.ReadList(MakeKey(BARREL_SENSOR_NB_NODES_X), l_vuNbNodesX, l_uNbLayers);

            vector<unsigned> l_vuNbNodesY;
            p_oReader.ReadList(MakeKey(BARREL_SENSOR_NB_NODES_Y), l_vuNbNodesY, l_uNbLayers);

            vector<double>l_vdSensorPitchX;
            p_oReader.ReadList(MakeKey(BARREL_SENSOR_PITCH_X), l_vdSensorPitchX, l_uNbLayers);

            vector<double> l_vdSensorPitchY;
            p_oReader.ReadList(MakeKey(BARREL_SENSOR_PITCH_Y), l_vdSensorPitchY, l_uNbLayers);

            vector<double>l_vdSensorNodeSizeX;
            p_oReader.ReadList(MakeKey(BARREL_SENSOR_NODE_SIZE_X), l_vdSensorNodeSizeX, l_uNbLayers);

            vector<double> l_vdSensorNodeSizeY;
            p_oReader.ReadList(MakeKey(BARREL_SENSOR_NODE_SIZE_Y), l_vdSensorNodeSizeY, l_uNbLayers);

            vector<double> l_vdSensorDimX;
            p_oReader.ReadList(MakeKey(BARREL_SENSOR_DIM_X), l_vdSensorDimX, l_uNbLayers);

            vector<double> l_vdSensorDimY;
            p_oReader.ReadList(MakeKey(BARREL_SENSOR_DIM_Y), l_vdSensorDimY, l_uNbLayers);

            vector<double> l_vdSensorDimZ;
            p_oReader.ReadList(MakeKey(BARREL_SENSOR_DIM_Z), l_vdSensorDimZ, l_uNbLayers);

            vector<unsigned> l_vuNbChipChannels;
            p_oReader.ReadList(MakeKey(BARREL_CHIP_NB_CHANNELS), l_vuNbChipChannels, l_uNbLayers);

            vector<unsigned> l_vuNbChips;
            p_oReader.ReadList(MakeKey(BARREL_NB_CHIPS), l_vuNbChips, l_uNbLayers);

            vector<unsigned> l_vuChipRows;
            p_oReader.ReadList(MakeKey(BARREL_CHIPS_PER_ROW), l_vuChipRows, l_uNbLayers);

            vector<double> l_vdChipDimX;
            p_oReader.ReadList(MakeKey(BARREL_CHIP_X_DIMENSIONS), l_vdChipDimX, l_uNbLayers);

            vector<double> l_vdChipDimY;
            p_oReader.ReadList(MakeKey(BARREL_CHIP_Y_DIMENSIONS), l_vdChipDimY, l_uNbLayers);

            vector<double> l_vdChipDimZ;
            p_oReader.ReadList(MakeKey(BARREL_CHIP_Z_DIMENSIONS), l_vdChipDimZ, l_uNbLayers);

            vector<unsigned> l_vuNbSensorX;
            p_oReader.ReadList(MakeKey(BARREL_SENSORS_PER_MODULE_X), l_vuNbSensorX, l_uNbLayers);

            vector<unsigned> l_vuNbSensorY;
            p_oReader.ReadList(MakeKey(BARREL_SENSORS_PER_MODULE_Y), l_vuNbSensorY, l_uNbLayers);

            vector<double>l_vdGapX;
            p_oReader.ReadList(MakeKey(BARREL_MODULE_GAPS_X), l_vdGapX, l_uNbLayers);

            vector<double> l_vdGapY;
            p_oReader.ReadList(MakeKey(BARREL_MODULE_GAPS_Y), l_vdGapY, l_uNbLayers);

            vector<string> l_vsModuleSupportMaterialNames;
            p_oReader.ReadList(MakeKey(MODULE_SUPPORT_MATERIAL_NAME), l_vsModuleSupportMaterialNames, l_uNbLayers);

            vector<string> l_vsModuleSupportEnable;
            p_oReader.ReadList(MakeKey(BARREL_MODULE_SUPPORT_ENABLE), l_vsModuleSupportEnable, l_uNbLayers);

            vector<double> l_vdModuleSupportThickness;
            p_oReader.ReadList(MakeKey(BARREL_MODULE_SUPPORT_THICKNESS), l_vdModuleSupportThickness, l_uNbLayers);

            vector<double> l_vdModuleSupportWidth;
            p_oReader.ReadList(MakeKey(BARREL_MODULE_SUPPORT_WIDTH), l_vdModuleSupportWidth, l_uNbLayers);

            vector<double> l_vdModuleSupportEdge;
            p_oReader.ReadList(MakeKey(BARREL_MODULE_SUPPORT_EDGE), l_vdModuleSupportEdge, l_uNbLayers);

            vector<double> l_vdModuleDirectionAngle;
            p_oReader.ReadList(MakeKey(BARREL_MODULE_DIRECTION_ANGLE), l_vdModuleDirectionAngle, l_uNbLayers);

            vector<double> l_vdModuleDirectionFace;
            p_oReader.ReadList(MakeKey(BARREL_MODULE_FACE_ROTATION_ANGLE), l_vdModuleDirectionFace, l_uNbLayers);

            unsigned l_uCurrentLayer = 0;
            for(unsigned barrel=0; barrel<l_uNumberOfBarrels; barrel++)
            {
                const Barrel::BarrelType l_sBarrelType = l_vsBarrelTypes[barrel];
                Barrel& l_oBarrel = p_oBarrelArray.AddNewBarrel(l_sBarrelType);
                l_oBarrel.SetAdjustement(l_vsBarrelRadialAdjust[barrel], l_vsBarrelLongitudinalAdjust[barrel]);
                l_oBarrel.SetBarrelShape(l_vdBarrelRadius[barrel] * p_oReader.MetricalUnitsCorrection(),
                                         l_vdBarrelLength[barrel] * p_oReader.MetricalUnitsCorrection());

                l_oBarrel.Initialize();

                l_oBarrel.EnableSupport(l_vsBarrelSupportEnable[barrel]=="enabled");
                l_oBarrel.SetBarrelSupportParameters(l_vsBarrelSupportMaterial[barrel],
                                              l_vdBarrelSupportThickness[barrel] * p_oReader.MetricalUnitsCorrection());

                for(unsigned layer=0; layer<l_vuNbLayersPerBarrel[barrel]; layer++)
                {
                    TPlaneDistribution l_auNbPixels;
                    l_auNbPixels.x = l_vuNbNodesX[l_uCurrentLayer];
                    l_auNbPixels.y = l_vuNbNodesY[l_uCurrentLayer];
                    TPlaneDimension l_adPitch;
                    l_adPitch.x = l_vdSensorPitchX[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection();
                    l_adPitch.y = l_vdSensorPitchY[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection();
                    TCuboidSize l_adWidth;
                    l_adWidth.x = l_vdSensorNodeSizeX[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection();
                    l_adWidth.y = l_vdSensorNodeSizeY[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection();
                    l_adWidth.z = 0;
                    TCuboidSize l_adSensorSize;
                    l_adSensorSize.x = l_vdSensorDimX[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection();
                    l_adSensorSize.y = l_vdSensorDimY[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection();
                    l_adSensorSize.z = l_vdSensorDimZ[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection();

                    TPlaneDistribution l_auNbSensors;
                    l_auNbSensors.x = l_vuNbSensorX[l_uCurrentLayer];
                    l_auNbSensors.y = l_vuNbSensorY[l_uCurrentLayer];
                    TPlaneDimension l_adGap;
                    l_adGap.x = l_vdGapX[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection();
                    l_adGap.y = l_vdGapY[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection();

                    l_adSensorSize.x = l_adSensorSize.x * l_auNbSensors.x + l_adGap.x * (l_auNbSensors.x - 1);
                    l_adSensorSize.y = l_adSensorSize.y * l_auNbSensors.y + l_adGap.y * (l_auNbSensors.y - 1);
                    l_auNbSensors.x = 1;
                    l_auNbSensors.y = 1;
                    P<Sensor> l_pSensor = new Sensor(l_adSensorSize, l_auNbPixels, l_adPitch, l_adWidth,
                                                     l_vsSensorMaterialNames[l_uCurrentLayer]);
                    Module::SensorArray l_oSensorArray(l_pSensor, l_auNbSensors, l_adGap);

                    TCuboidSize l_ChipSize;
                    l_ChipSize.x = l_vdChipDimX[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection();
                    l_ChipSize.y = l_vdChipDimY[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection();
                    l_ChipSize.z = l_vdChipDimZ[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection();
                    TPlaneDistribution l_vChipDistribution;
                    l_vChipDistribution.x = l_vuNbChips[l_uCurrentLayer] / l_vuChipRows[l_uCurrentLayer];
                    l_vChipDistribution.y = l_vuChipRows[l_uCurrentLayer];
                    Module::ChipArray l_oChipArray(l_ChipSize, l_vChipDistribution,
                                                   l_vuNbChipChannels[l_uCurrentLayer]);

                    bool l_bEnable = l_vsModuleSupportEnable[l_uCurrentLayer] == "enabled";
                    Module::Support l_oModuleSupport(
                        l_vdModuleSupportThickness[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection(),
                        l_vdModuleSupportWidth[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection(),
                        l_vdModuleSupportEdge[l_uCurrentLayer] * p_oReader.MetricalUnitsCorrection(),
                        l_vsModuleSupportMaterialNames[l_uCurrentLayer], l_bEnable);

                    l_oBarrel.InitializeModulePrototype(layer, l_oSensorArray, l_oChipArray, l_oModuleSupport);

                    l_oBarrel.SetModuleOrientation(
                        layer,
                        l_vdModuleDirectionAngle[l_uCurrentLayer] * p_oReader.AngularUnitsCorrection(),
                        l_vdModuleDirectionFace[l_uCurrentLayer] * p_oReader.AngularUnitsCorrection() );

                    l_uCurrentLayer++;
                }
            }
            p_oBarrelArray.SetSensitiveDetectorNames(GetSubDetectorName());
        }

    private: // Internal methods.
        /// Combines variable name and detector name to create an unique key.
        /** Generated key can be used to point data entry into an external storage.*/
        string MakeKey(const string& p_sVariableName) const
        {
            return GetSubDetectorName() + p_sVariableName;
        }

    private: // Data members
        /// A name of the detector.
        string m_sSubDetectorName;
    };
}
