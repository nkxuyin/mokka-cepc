/*! \file EndcapSerializer.cc
    \brief An implementation of Silc::EndcapSerializer class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "EndcapSerializer.hh"

using namespace Silc;

const char* EndcapSerializer::ENDCAP_LAYER_TYPES_ENTRY = "etd_layer_types";
const char* EndcapSerializer::ENDCAP_LAYER_POSITIONS_ENTRY = "etd_layer_positions";
const char* EndcapSerializer::ENDCAP_LAYER_ROTATIONS_ENTRY = "etd_layer_rotations";
const char* EndcapSerializer::ENDCAP_INNER_RADIUS_ENTRY = "etd_inner_radius";
const char* EndcapSerializer::ENDCAP_OUTER_RADIUS_ENTRY = "etd_outer_radius";
const char* EndcapSerializer::ENDCAP_PLANE_BORDER_WIDTH_ENTRY = "etd_plane_border_width";
const char* EndcapSerializer::ENDCAP_RADIAL_BORDER_WIDTH_ENTRY = "etd_radial_border_width";
const char* EndcapSerializer::ENDCAP_MODULE_SUPPORT_THICKNESS_ENTRY = "etd_module_support_thickness";
const char* EndcapSerializer::ENDCAP_MODULE_SUPPORT_WIDTH_ENTRY = "etd_module_support_width";
const char* EndcapSerializer::ENDCAP_MODULE_SUPPORT_STANDOFF_FROM_EDGE_ENTRY =
    "etd_module_support_standoff_from_edge";
const char* EndcapSerializer::ENDCAP_SUPPORT_MOUSSE_THICKNESS_ENTRY = "etd_endcap_support_mousse_thickness";
const char* EndcapSerializer::ENDCAP_SUPPORT_CARBON_THICKNESS_ENTRY = "etd_endcap_support_carbon_thickness";
const char* EndcapSerializer::ENDCAP_SUPPORT_REINFORCEMENT_THICKNESS_ENTRY =
    "etd_endcap_support_reinforcement_thickness";

const char* EndcapSerializer::ENDCAP_ACCURACY_ZONES_ENTRY = "etd_accuracy_zones";
const char* EndcapSerializer::ENDCAP_SENSOR_TECHNOLOGIES_ENTRY = "etd_sensor_technologies";
const char* EndcapSerializer::ENDCAP_SENSOR_X_DIMENSIONS_ENTRY = "etd_sensor_x_dimensions";
const char* EndcapSerializer::ENDCAP_SENSOR_Y_DIMENSIONS_ENTRY = "etd_sensor_y_dimensions";
const char* EndcapSerializer::ENDCAP_SENSOR_Z_DIMENSIONS_ENTRY = "etd_sensor_z_dimensions";
const char* EndcapSerializer::ENDCAP_SENSORS_PER_MODULE_ENTRY = "etd_sensors_per_module";
const char* EndcapSerializer::ENDCAP_CHIPS_PER_ROW_ENTRY = "etd_chips_per_row";
const char* EndcapSerializer::ENDCAP_CHIPS_PER_COLUMN_ENTRY = "etd_chips_per_column";
const char* EndcapSerializer::ENDCAP_CHIP_X_DIMENSIONS_ENTRY = "etd_chip_x_dimensions";
const char* EndcapSerializer::ENDCAP_CHIP_Y_DIMENSIONS_ENTRY = "etd_chip_y_dimensions";
const char* EndcapSerializer::ENDCAP_CHIP_Z_DIMENSIONS_ENTRY = "etd_chip_z_dimensions";
const char* EndcapSerializer::ENDCAP_CHIP_CHANNELS_ENTRY = "etd_chip_channels";

// External parameters.
const char* EndcapSerializer::ECAL_Z_MIN = "Ecal_endcap_zmin";
const char* EndcapSerializer::ECAL_CENTER_RADIUS = "Ecal_endcap_center_box_size";
const char* EndcapSerializer::TPC_OUTER_RADIUS_ENTRY = "TPC_outer_radius";

// Temporary constants -> to put into db.
const char* EndcapSerializer::TMP_SENSOR_MATERIAL_NAME_VALUE = "silicon_2.33gccm";
const char* EndcapSerializer::TMP_MODULE_SUPPORT_MATERIAL_NAME_VALUE = "graphite";
