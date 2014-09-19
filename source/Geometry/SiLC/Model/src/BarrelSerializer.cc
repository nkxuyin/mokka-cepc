/*! \file BarrelSerializer.cc
    \brief An implementation of Silc::BarrelSerializer class.

This code is a part of the Geometry Builder for silicon tracking.
Created and Developed by the SiLC Collaboration for full simulation optimisation.

The design pattern of this code is originally defined to be used in Mokka/GEANT4 simulation and IlcRoot.
Special thanks for Paulo Mora de Freitas and Gabriel Musat for their help.

Created in 2010-2011 by A. Charpy <professional@charpy.net> and K. Androsov <konstantin.androsov@gmail.com>.

If you develop and bring new feature to this code, please, add your name in the following list.

Contribution of:
...
*/

#include "BarrelSerializer.hh"

using namespace Silc;

const char* BarrelSerializer::BARREL_TYPE = "_barrel_types";
const char* BarrelSerializer::BARREL_RADIUS = "_barrel_radiuses";

const char* BarrelSerializer::BARREL_LENGTH = "_barrel_lengths";
const char* BarrelSerializer::BARREL_RADIAL_ADJUST = "_barrel_radial_adjustments";
const char* BarrelSerializer::BARREL_LONGITUDINAL_ADJUST =  "_barrel_longitudinal_adjustments";
const char* BarrelSerializer::BARREL_SUPPORT_ENABLE= "_barrel_support_states";
const char* BarrelSerializer::BARREL_SUPPORT_MATERIAL = "_barrel_support_materials";
const char* BarrelSerializer::BARREL_SUPPORT_THICKNESS = "_barrel_support_thicknesses";

const char* BarrelSerializer::BARREL_SENSOR_NB_NODES_X = "_sensor_nb_nodes_x";
const char* BarrelSerializer::BARREL_SENSOR_NB_NODES_Y = "_sensor_nb_nodes_y";
const char* BarrelSerializer::BARREL_SENSOR_PITCH_X = "_sensor_pitches_x";
const char* BarrelSerializer::BARREL_SENSOR_PITCH_Y = "_sensor_pitches_y";
const char* BarrelSerializer::BARREL_SENSOR_NODE_SIZE_X = "_sensor_node_sizes_x";
const char* BarrelSerializer::BARREL_SENSOR_NODE_SIZE_Y = "_sensor_node_sizes_y";
const char* BarrelSerializer::BARREL_SENSOR_DIM_X = "_sensor_dim_x";
const char* BarrelSerializer::BARREL_SENSOR_DIM_Y = "_sensor_dim_y";
const char* BarrelSerializer::BARREL_SENSOR_DIM_Z = "_sensor_dim_z";

const char* BarrelSerializer::BARREL_NB_CHIPS = "_module_nb_chips";
const char* BarrelSerializer::BARREL_CHIP_NB_CHANNELS = "_chip_nb_channels";
const char* BarrelSerializer::BARREL_CHIPS_PER_ROW = "_module_nb_chips_per_row";
const char* BarrelSerializer::BARREL_CHIP_X_DIMENSIONS = "_chip_dim_x";
const char* BarrelSerializer::BARREL_CHIP_Y_DIMENSIONS = "_chip_dim_y";
const char* BarrelSerializer::BARREL_CHIP_Z_DIMENSIONS = "_chip_dim_z";

const char* BarrelSerializer::BARREL_SENSORS_PER_MODULE_X = "_module_nb_sensors_x";
const char* BarrelSerializer::BARREL_SENSORS_PER_MODULE_Y = "_module_nb_sensors_y";
const char* BarrelSerializer::BARREL_MODULE_GAPS_X = "_module_gaps_x";
const char* BarrelSerializer::BARREL_MODULE_GAPS_Y= "_module_gaps_y";

const char* BarrelSerializer::BARREL_MODULE_SUPPORT_ENABLE= "_module_support_states";
const char* BarrelSerializer::BARREL_MODULE_SUPPORT_THICKNESS = "_module_support_thicknesses";
const char* BarrelSerializer::BARREL_MODULE_SUPPORT_WIDTH = "_module_support_widths";
const char* BarrelSerializer::BARREL_MODULE_SUPPORT_EDGE = "_module_support_edges";

const char* BarrelSerializer::BARREL_MODULE_DIRECTION_ANGLE = "_module_direction_angles";
const char* BarrelSerializer::BARREL_MODULE_FACE_ROTATION_ANGLE = "_module_face_rotation_angles";

const char* BarrelSerializer::SENSOR_MATERIAL_NAME = "_sensor_materials";
const char* BarrelSerializer::SENSOR_MATERIAL_RADIATION_LENGTH = "_sensor_rad_lengths";
const char* BarrelSerializer::MODULE_SUPPORT_MATERIAL_NAME = "_module_support_materials";
const char* BarrelSerializer::MODULE_SUPPORT_MATERIAL_RADIATION_LENGTH = "_module_support_rad_lengths";

// External parameters.
const char* BarrelSerializer::TPC_OUTER_RADIUS = "TPC_outer_radius";
const char* BarrelSerializer::TPC_ECAL_HCAL_HALF = "TPC_Ecal_Hcal_barrel_halfZ";
