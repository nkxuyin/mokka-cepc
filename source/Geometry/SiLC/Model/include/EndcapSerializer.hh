/*! \file EndcapSerializer.hh
    \brief A definition of Silc::EndcapSerializer class.

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

#include "EndcapArray.hh"

namespace Silc
{
    /// Provides a serialization functionality to save and load Endcap configuration.
    /** Silc::EndcapSerializer uses generic save/load methods, so it can be used with different Reader and Writer
        implementations. */
    class EndcapSerializer
    {
    private: // Constants.
        static const char* ENDCAP_LAYER_TYPES_ENTRY;
        static const char* ENDCAP_LAYER_POSITIONS_ENTRY;
        static const char* ENDCAP_LAYER_ROTATIONS_ENTRY;
        static const char* ENDCAP_INNER_RADIUS_ENTRY;
        static const char* ENDCAP_OUTER_RADIUS_ENTRY;
        static const char* ENDCAP_PLANE_BORDER_WIDTH_ENTRY;
        static const char* ENDCAP_RADIAL_BORDER_WIDTH_ENTRY;
        static const char* ENDCAP_MODULE_SUPPORT_THICKNESS_ENTRY;
        static const char* ENDCAP_MODULE_SUPPORT_WIDTH_ENTRY;
        static const char* ENDCAP_MODULE_SUPPORT_STANDOFF_FROM_EDGE_ENTRY;
        static const char* ENDCAP_SUPPORT_MOUSSE_THICKNESS_ENTRY;
        static const char* ENDCAP_SUPPORT_CARBON_THICKNESS_ENTRY;
        static const char* ENDCAP_SUPPORT_REINFORCEMENT_THICKNESS_ENTRY;

        static const char* ENDCAP_ACCURACY_ZONES_ENTRY;
        static const char* ENDCAP_SENSOR_TECHNOLOGIES_ENTRY;
        static const char* ENDCAP_SENSOR_X_DIMENSIONS_ENTRY;
        static const char* ENDCAP_SENSOR_Y_DIMENSIONS_ENTRY;
        static const char* ENDCAP_SENSOR_Z_DIMENSIONS_ENTRY;
        static const char* ENDCAP_SENSORS_PER_MODULE_ENTRY;
        static const char* ENDCAP_CHIPS_PER_ROW_ENTRY;
        static const char* ENDCAP_CHIPS_PER_COLUMN_ENTRY;
        static const char* ENDCAP_CHIP_X_DIMENSIONS_ENTRY;
        static const char* ENDCAP_CHIP_Y_DIMENSIONS_ENTRY;
        static const char* ENDCAP_CHIP_Z_DIMENSIONS_ENTRY;
        static const char* ENDCAP_CHIP_CHANNELS_ENTRY;

        // External parameters.
        static const char* ECAL_Z_MIN;
        static const char* ECAL_CENTER_RADIUS;
        static const char* TPC_OUTER_RADIUS_ENTRY;

        // Temporary constants -> to put into db.
        static const char* TMP_SENSOR_MATERIAL_NAME_VALUE;
        static const char* TMP_MODULE_SUPPORT_MATERIAL_NAME_VALUE;

    public: // Basic methods.
        /// The Silc::EndcapSerializer constructor.
        EndcapSerializer(const string& p_sSubDetectorName) throw()
            : m_sSubDetectorName(p_sSubDetectorName)
        {}

        /// Returns a name of the sub-detector.
        const string& GetSubDetectorName() const throw()
        {
            return m_sSubDetectorName;
        }

        /// Saves Silc::EndcapArray configuration using generic data writer.
        template<typename WriterType>
        void Save(const EndcapArray& p_oEndcapArray, WriterType& p_oWriter) const
        {
            assert(p_oEndcapArray.GetNumberOfEndcaps() > 0);
            const unsigned l_uNumberOfEndcaps = p_oEndcapArray.GetNumberOfEndcaps();
            vector<string> l_vEndcapTypes(l_uNumberOfEndcaps);
            vector<double> l_vEndcapPositions(l_uNumberOfEndcaps);
            vector<double> l_vEndcapRotations(l_uNumberOfEndcaps);

            for(unsigned n = 0; n < l_uNumberOfEndcaps; ++n)
            {
                l_vEndcapTypes[n] = p_oEndcapArray[n].EndcapObject->GetType();
                l_vEndcapPositions[n] = p_oEndcapArray[n].ZPosition * p_oWriter.MetricalUnitsCorrection();
                l_vEndcapRotations[n] = p_oEndcapArray[n].RotationAngle * p_oWriter.AngularUnitsCorrection();
            }

            p_oWriter.Write(ENDCAP_LAYER_TYPES_ENTRY, l_vEndcapTypes);
            p_oWriter.Write(ENDCAP_LAYER_POSITIONS_ENTRY, l_vEndcapPositions);
            p_oWriter.Write(ENDCAP_LAYER_ROTATIONS_ENTRY, l_vEndcapRotations);

            const unsigned l_uNumberOfZones = p_oEndcapArray.GetNumberOfZones();
            const Endcap& l_oEndcap = *p_oEndcapArray[0].EndcapObject;
            vector<double> l_vAccuracyZones(l_uNumberOfZones);
            vector<double> l_vSensorXDimensions(l_uNumberOfZones);
            vector<double> l_vSensorYDimensions(l_uNumberOfZones);
            vector<double> l_vSensorZDimensions(l_uNumberOfZones);
            vector<unsigned> l_vSensorsPerModule(l_uNumberOfZones);
            vector<unsigned> l_vChipsPerRow(l_uNumberOfZones);
            vector<unsigned> l_vChipsPerColumn(l_uNumberOfZones);
            vector<double> l_vChipXDimensions(l_uNumberOfZones);
            vector<double> l_vChipYDimensions(l_uNumberOfZones);
            vector<double> l_vChipZDimensions(l_uNumberOfZones);
            vector<unsigned> l_vChipChannels(l_uNumberOfZones);

            for(unsigned n = 0; n < l_uNumberOfZones; ++n)
            {
                const Module& l_oModule = l_oEndcap.GetModulePrototype(n);
                const Module::SensorArray& l_oSensorArray = l_oModule.GetSensorArray();
                const Module::ChipArray& l_oChipArray = l_oModule.GetChipArray();
                const Sensor& l_oSensor = l_oSensorArray.GetSensorPrototype();
                l_vAccuracyZones[n] = p_oEndcapArray.GetZoneAngle(n) * p_oWriter.AngularUnitsCorrection();
                l_vSensorXDimensions[n] = l_oSensor.GetSensorSize().x * p_oWriter.MetricalUnitsCorrection();
                l_vSensorYDimensions[n] = l_oSensor.GetSensorSize().y * p_oWriter.MetricalUnitsCorrection();
                l_vSensorZDimensions[n] = l_oSensor.GetSensorSize().z * p_oWriter.MetricalUnitsCorrection();
                l_vSensorsPerModule[n] = l_oSensorArray.GetNumberOfSensors().x;
                l_vChipsPerRow[n] = l_oChipArray.GetChipDistribution().x;
                l_vChipsPerColumn[n] = l_oChipArray.GetChipDistribution().y;
                l_vChipXDimensions[n] = l_oChipArray.GetChipSize().x * p_oWriter.MetricalUnitsCorrection();
                l_vChipYDimensions[n] = l_oChipArray.GetChipSize().y * p_oWriter.MetricalUnitsCorrection();
                l_vChipZDimensions[n] = l_oChipArray.GetChipSize().z * p_oWriter.MetricalUnitsCorrection();
                l_vChipChannels[n] = l_oChipArray.GetNumberOfChannelsPerChip();
            }

            p_oWriter.Write(ENDCAP_ACCURACY_ZONES_ENTRY, l_vAccuracyZones);
            p_oWriter.Write(ENDCAP_SENSOR_X_DIMENSIONS_ENTRY, l_vSensorXDimensions);
            p_oWriter.Write(ENDCAP_SENSOR_Y_DIMENSIONS_ENTRY, l_vSensorYDimensions);
            p_oWriter.Write(ENDCAP_SENSOR_Z_DIMENSIONS_ENTRY, l_vSensorZDimensions);
            p_oWriter.Write(ENDCAP_SENSORS_PER_MODULE_ENTRY, l_vSensorsPerModule);
            p_oWriter.Write(ENDCAP_CHIPS_PER_ROW_ENTRY, l_vChipsPerRow);
            p_oWriter.Write(ENDCAP_CHIPS_PER_COLUMN_ENTRY, l_vChipsPerColumn);
            p_oWriter.Write(ENDCAP_CHIP_X_DIMENSIONS_ENTRY, l_vChipXDimensions);
            p_oWriter.Write(ENDCAP_CHIP_Y_DIMENSIONS_ENTRY, l_vChipYDimensions);
            p_oWriter.Write(ENDCAP_CHIP_Y_DIMENSIONS_ENTRY, l_vChipZDimensions);
            p_oWriter.Write(ENDCAP_CHIP_CHANNELS_ENTRY, l_vChipChannels);

            const Module::Support& l_oModuleSupport = l_oEndcap.GetModulePrototype(0).GetSupport();
            const TLength l_dInnerRadius = l_oEndcap.GetInnerRadius() * p_oWriter.MetricalUnitsCorrection();
            const TLength l_dOuterRadius = l_oEndcap.GetOuterRadius() * p_oWriter.MetricalUnitsCorrection();
            const TLength l_dPlaneBorderWidth = l_oEndcap.GetPlaneSupportWidth() * p_oWriter.MetricalUnitsCorrection();
            const TLength l_dRadialBorderWidth = l_oEndcap.GetCircularSupportWidth()
                                                 * p_oWriter.MetricalUnitsCorrection();
            const TLength l_dMousseThickness = l_oEndcap.GetMousseThickness() * p_oWriter.MetricalUnitsCorrection();
            const TLength l_dCarbonThickness = l_oEndcap.GetCarbonThickness() * p_oWriter.MetricalUnitsCorrection();
            const TLength l_dReinforcementThickness = l_oEndcap.GetReinforcementThickness()
                    * p_oWriter.MetricalUnitsCorrection();
            const TLength l_dModuleSupportThickness = l_oModuleSupport.GetThickness()
                    * p_oWriter.MetricalUnitsCorrection();
            const TLength l_dModuleSupportWidth = l_oModuleSupport.GetWidth() * p_oWriter.MetricalUnitsCorrection();
            const TLength l_dModuleSupportStandoffFromEdge = l_oModuleSupport.GetStandoffFromEdge()
                    * p_oWriter.MetricalUnitsCorrection();

            p_oWriter.Write(ENDCAP_INNER_RADIUS_ENTRY, l_dInnerRadius);
            p_oWriter.Write(ENDCAP_OUTER_RADIUS_ENTRY, l_dOuterRadius);
            p_oWriter.Write(ENDCAP_PLANE_BORDER_WIDTH_ENTRY, l_dPlaneBorderWidth);
            p_oWriter.Write(ENDCAP_RADIAL_BORDER_WIDTH_ENTRY, l_dRadialBorderWidth);
            p_oWriter.Write(ENDCAP_SUPPORT_MOUSSE_THICKNESS_ENTRY, l_dMousseThickness);
            p_oWriter.Write(ENDCAP_SUPPORT_CARBON_THICKNESS_ENTRY, l_dCarbonThickness);
            p_oWriter.Write(ENDCAP_SUPPORT_REINFORCEMENT_THICKNESS_ENTRY, l_dReinforcementThickness);
            p_oWriter.Write(ENDCAP_MODULE_SUPPORT_THICKNESS_ENTRY, l_dModuleSupportThickness);
            p_oWriter.Write(ENDCAP_MODULE_SUPPORT_WIDTH_ENTRY, l_dModuleSupportWidth);
            p_oWriter.Write(ENDCAP_MODULE_SUPPORT_STANDOFF_FROM_EDGE_ENTRY, l_dModuleSupportStandoffFromEdge);
        }

        /// Loads Silc::EndcapArray configuration using generic data reader.
        /** p_bUseExternalParameters indicates if a configuration with the external contriants should be loaded,
            or if only propre configuration parameters should be used. */
        template<typename ReaderType>
        void Load(EndcapArray& p_oEndcapArray, ReaderType& p_oReader, bool p_bUseExternalParameters = false) const
        throw(Exception)
        {
            vector<string> l_vEndcapTypes;
            const unsigned l_uNumberOfEndcaps = p_oReader.ReadList(ENDCAP_LAYER_TYPES_ENTRY, l_vEndcapTypes);

            vector<double> l_vEndcapPositions;
            p_oReader.ReadList(ENDCAP_LAYER_POSITIONS_ENTRY, l_vEndcapPositions, l_uNumberOfEndcaps);
            if(p_bUseExternalParameters)
            {
                double l_dECalZPosition;
                p_oReader.Read(ECAL_Z_MIN, l_dECalZPosition);
                for(unsigned layer=0; layer<l_uNumberOfEndcaps; layer++)
                    l_vEndcapPositions.at(layer) = l_dECalZPosition - l_vEndcapPositions.at(layer);
            }

            vector<double> l_vEndcapRotations;
            p_oReader.ReadList(ENDCAP_LAYER_ROTATIONS_ENTRY, l_vEndcapRotations, l_uNumberOfEndcaps);

            map<Endcap::EndcapType, unsigned> l_mBaseEndcaps;
            for(unsigned n = 0; n < l_uNumberOfEndcaps; ++n)
            {
                const Endcap::EndcapType l_sEndcapType = l_vEndcapTypes[n];
                const TCoordinate l_dEndcapPosition = l_vEndcapPositions[n] * p_oReader.MetricalUnitsCorrection();
                const TAngle l_dEndcapRotation = l_vEndcapRotations[n] * p_oReader.AngularUnitsCorrection();
                if(l_mBaseEndcaps.count(l_sEndcapType))
                {
                    const unsigned l_uEndcapId = l_mBaseEndcaps[l_sEndcapType];
                    p_oEndcapArray.AddDuplicateEndcap(l_uEndcapId, l_dEndcapPosition, l_dEndcapRotation);
                }
                else
                {
                    const unsigned l_uEndcapId = p_oEndcapArray.AddNewEndcap(l_sEndcapType, l_dEndcapPosition,
                                                 l_dEndcapRotation);
                    l_mBaseEndcaps[l_sEndcapType] = l_uEndcapId;
                }
            }

            vector<double> l_vAccuracyZones;
            const unsigned l_uNumberOfZones = p_oReader.ReadList(ENDCAP_ACCURACY_ZONES_ENTRY, l_vAccuracyZones);
            p_oEndcapArray.SetNumberOfZones(l_uNumberOfZones);
            for(unsigned l_uZoneId = 0; l_uZoneId < l_uNumberOfZones; ++l_uZoneId)
            {
                const TAngle l_dZoneAngle = l_vAccuracyZones[l_uZoneId] * p_oReader.AngularUnitsCorrection();
                p_oEndcapArray.SetZoneAngle(l_uZoneId, l_dZoneAngle);
            }

            for(map<Endcap::EndcapType, unsigned>::const_iterator l_pIter = l_mBaseEndcaps.begin();
                    l_pIter != l_mBaseEndcaps.end(); ++l_pIter)
            {
                const unsigned l_uEndcapId = l_pIter->second;
                Endcap& l_oEndcap = *p_oEndcapArray[l_uEndcapId].EndcapObject;
                LoadEndcap(l_oEndcap, p_oReader, p_bUseExternalParameters);
            }
        }

    private: // Internal methods.
        /// Loads Silc::Endcap configuration using generic data reader.
        template<typename ReaderType>
        void LoadEndcap(Endcap& p_oEndcap, ReaderType& p_oReader, bool p_bUseExternalParameters) const
        {
            TLength l_dInnerRadius, l_dOuterRadius;
            if(p_bUseExternalParameters)
            {
                p_oReader.Read(ECAL_CENTER_RADIUS, l_dInnerRadius);
                l_dInnerRadius = l_dInnerRadius / 2 * p_oReader.MetricalUnitsCorrection();
                p_oReader.Read(TPC_OUTER_RADIUS_ENTRY, l_dOuterRadius);
                l_dOuterRadius = l_dOuterRadius * p_oReader.MetricalUnitsCorrection();
            }
            else
            {
                p_oReader.Read(ENDCAP_INNER_RADIUS_ENTRY, l_dInnerRadius);
                l_dInnerRadius *= p_oReader.MetricalUnitsCorrection();
                p_oReader.Read(ENDCAP_OUTER_RADIUS_ENTRY, l_dOuterRadius);
                l_dOuterRadius *= p_oReader.MetricalUnitsCorrection();
            }
            p_oEndcap.SetLimits(l_dInnerRadius, l_dOuterRadius);

            TLength l_dPlaneBorderWidth;
            p_oReader.Read(ENDCAP_PLANE_BORDER_WIDTH_ENTRY, l_dPlaneBorderWidth);
            l_dPlaneBorderWidth *= p_oReader.MetricalUnitsCorrection();
            TLength l_dRadialBorderWidth;
            p_oReader.Read(ENDCAP_RADIAL_BORDER_WIDTH_ENTRY, l_dRadialBorderWidth);
            l_dRadialBorderWidth *= p_oReader.MetricalUnitsCorrection();
            p_oEndcap.SetSupermoduleSupportInformation(l_dPlaneBorderWidth, l_dRadialBorderWidth);

            TLength l_dMousseThickness;
            p_oReader.Read(ENDCAP_SUPPORT_MOUSSE_THICKNESS_ENTRY, l_dMousseThickness);
            l_dMousseThickness *= p_oReader.MetricalUnitsCorrection();
            TLength l_dCarbonThickness;
            p_oReader.Read(ENDCAP_SUPPORT_CARBON_THICKNESS_ENTRY, l_dCarbonThickness);
            l_dCarbonThickness *= p_oReader.MetricalUnitsCorrection();
            TLength l_dReinforcementThickness;
            p_oReader.Read(ENDCAP_SUPPORT_REINFORCEMENT_THICKNESS_ENTRY, l_dReinforcementThickness);
            l_dReinforcementThickness *= p_oReader.MetricalUnitsCorrection();
            p_oEndcap.SetEndcapSupportInformation(l_dCarbonThickness, l_dMousseThickness, l_dReinforcementThickness);

            TLength l_dModuleSupportThickness;
            p_oReader.Read(ENDCAP_MODULE_SUPPORT_THICKNESS_ENTRY, l_dModuleSupportThickness);
            l_dModuleSupportThickness *= p_oReader.MetricalUnitsCorrection();
            TLength l_dModuleSupportWidth;
            p_oReader.Read(ENDCAP_MODULE_SUPPORT_WIDTH_ENTRY, l_dModuleSupportWidth);
            l_dModuleSupportWidth *= p_oReader.MetricalUnitsCorrection();
            TLength l_dModuleSupportStandoffFromEdge;
            p_oReader.Read(ENDCAP_MODULE_SUPPORT_STANDOFF_FROM_EDGE_ENTRY, l_dModuleSupportStandoffFromEdge);
            l_dModuleSupportStandoffFromEdge *= p_oReader.MetricalUnitsCorrection();

            const unsigned l_uNumberOfZones = p_oEndcap.GetNumberOfZones();

            vector<double> l_vSensorXDimensions;
            p_oReader.ReadList(ENDCAP_SENSOR_X_DIMENSIONS_ENTRY, l_vSensorXDimensions, l_uNumberOfZones);

            vector<double> l_vSensorYDimensions;
            p_oReader.ReadList(ENDCAP_SENSOR_Y_DIMENSIONS_ENTRY, l_vSensorYDimensions, l_uNumberOfZones);

            vector<double> l_vSensorZDimensions;
            p_oReader.ReadList(ENDCAP_SENSOR_Z_DIMENSIONS_ENTRY, l_vSensorZDimensions, l_uNumberOfZones);

            vector<unsigned> l_vSensorsPerModule;
            p_oReader.ReadList(ENDCAP_SENSORS_PER_MODULE_ENTRY, l_vSensorsPerModule, l_uNumberOfZones);

            vector<unsigned> l_vChipsPerRow;
            p_oReader.ReadList(ENDCAP_CHIPS_PER_ROW_ENTRY, l_vChipsPerRow, l_uNumberOfZones);

            vector<unsigned> l_vChipsPerColumn;
            p_oReader.ReadList(ENDCAP_CHIPS_PER_COLUMN_ENTRY, l_vChipsPerColumn, l_uNumberOfZones);

            vector<double> l_vChipXDimensions;
            p_oReader.ReadList(ENDCAP_CHIP_X_DIMENSIONS_ENTRY, l_vChipXDimensions, l_uNumberOfZones);

            vector<double> l_vChipYDimensions;
            p_oReader.ReadList(ENDCAP_CHIP_Y_DIMENSIONS_ENTRY, l_vChipYDimensions, l_uNumberOfZones);

            vector<double> l_vChipZDimensions;
            p_oReader.ReadList(ENDCAP_CHIP_Z_DIMENSIONS_ENTRY, l_vChipZDimensions, l_uNumberOfZones);

            vector<unsigned> l_vChipChannels;
            p_oReader.ReadList(ENDCAP_CHIP_CHANNELS_ENTRY, l_vChipChannels, l_uNumberOfZones);

            for(unsigned l_uZoneId = 0; l_uZoneId < l_uNumberOfZones; ++l_uZoneId)
            {
                TCuboidSize l_vSensorSize;
                l_vSensorSize.x = l_vSensorXDimensions[l_uZoneId] * p_oReader.MetricalUnitsCorrection();
                l_vSensorSize.y = l_vSensorYDimensions[l_uZoneId] * p_oReader.MetricalUnitsCorrection();
                l_vSensorSize.z = l_vSensorZDimensions[l_uZoneId] * p_oReader.MetricalUnitsCorrection();
                TPlaneDistribution l_vSensitiveNodeDistribution;
                l_vSensitiveNodeDistribution.x = 10;
                l_vSensitiveNodeDistribution.y = 1;
                TPlaneDimension l_vPitchSize;
                l_vPitchSize.x = l_vSensorSize.x / 12;
                l_vPitchSize.y = l_vSensorSize.y;
                TCuboidSize l_vSensitiveNodeSize;
                l_vSensitiveNodeSize.x = l_vSensorSize.x / 50;
                l_vSensitiveNodeSize.y = l_vSensorSize.y;
                l_vSensitiveNodeSize.z = l_vSensorSize.z;
                P<Sensor> l_pSensor = new Sensor(l_vSensorSize, l_vSensitiveNodeDistribution, l_vPitchSize,
                                      l_vSensitiveNodeSize, TMP_SENSOR_MATERIAL_NAME_VALUE);

                TPlaneDistribution l_vSensorDistribution;
                l_vSensorDistribution.x = l_vSensorsPerModule[l_uZoneId];
                l_vSensorDistribution.y = 1;
                TPlaneDimension l_vGapBetweenSensors;
                l_vGapBetweenSensors.x = 0;
                l_vGapBetweenSensors.y = 0;
                Module::SensorArray l_oSensorArray(l_pSensor, l_vSensorDistribution, l_vGapBetweenSensors);

                TCuboidSize l_vChipSize;
                l_vChipSize.x = l_vChipXDimensions[l_uZoneId] * p_oReader.MetricalUnitsCorrection();
                l_vChipSize.y = l_vChipYDimensions[l_uZoneId] * p_oReader.MetricalUnitsCorrection();
                l_vChipSize.z = l_vChipZDimensions[l_uZoneId] * p_oReader.MetricalUnitsCorrection();
                const unsigned l_uChannelsPerChip = l_vChipChannels[l_uZoneId];
                TPlaneDistribution l_vChipDistribution;
                l_vChipDistribution.x = l_vChipsPerRow[l_uZoneId];
                l_vChipDistribution.y = l_vChipsPerColumn[l_uZoneId];
                Module::ChipArray l_oChipArray(l_vChipSize, l_vChipDistribution, l_uChannelsPerChip);

                Module::Support l_oModuleSupport(l_dModuleSupportThickness, l_dModuleSupportWidth,
                                                 l_dModuleSupportStandoffFromEdge,
                                                 TMP_MODULE_SUPPORT_MATERIAL_NAME_VALUE);

                p_oEndcap.InitializeModulePrototype(l_uZoneId, l_oSensorArray, l_oChipArray, l_oModuleSupport);
            }
        }
    private: // Data members.
        /// A name of the detector.
        string m_sSubDetectorName;
    };
}
