/*! \file Silc_Globals.hh
    \brief Defines all common stuff for the classes in Silc namespace.

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

#define VERBOSE_DEBUG
#define SAFE_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Frequently used headers.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Exception.hh"
#include "stl_extensions.hh"
#include "phys_primitives.hh"

#include <iostream>
#include <cmath>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <sstream>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Consolidates all classes designed for Silicon numerical simulations, digitization and event reconstruction.
namespace Silc
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Namespace-wide usings.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    using std::string;                                      ///< A default string container.
    using std::ostream;                                     ///< A default output stream.
    using std::stringstream;                                ///< A default string stream.
    using std_ext::P;                                       ///< A default smart pointer.
    using std::vector;                                      ///< A default vector container.
    using std::list;                                        ///< A default list container.
    using std::map;                                         ///< A default map container.
    using std::set;                                         ///< A default set container.

    using std::cout;                                        ///< A default output stream.
    using std::cerr;                                        ///< A default error stream.
    using std::clog;                                        ///< A default log stream.
    using std::endl;                                        ///< A default end line determinant.

    using std::abs;                                         ///< Calculates an absolute value.
    using std::max;                                         ///< Selects a maximal value.
    using std::min;                                         ///< Selects a minimal value.
    using std_ext::sqr;                                     ///< Calculates a value square.
    using std_ext::fcmp;                                    ///< Compares floating point numbers.

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Namespace-wide constants.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    static const unsigned SPATIAL_DIMENSION = 3;                     ///< Number of dimention in space.
    static const unsigned PLANE_DIMENSION = SPATIAL_DIMENSION - 1;   ///< Number of dimentions in plane.

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Namespace-wide types.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    typedef unsigned long long UInt64;                      ///< 64-bit unsigned integer.

    typedef double TLength;                                 ///< Type for store length in millimeters [mm].
    typedef double TCoordinate;                             ///< Type for store system coordinates in millimeters [mm].
    typedef double TAngle;                                  ///< Type for store angles in radians [rad].
    typedef double TEnergy;                                 ///< Type for store energy in mega electronvolts [MeV].
    typedef double TEnergyLinearDensity;                    ///< Type for store energy linear density [MeV/mm].

    typedef phys::euclidean_vector<TCoordinate, SPATIAL_DIMENSION> TVector;   ///< Type for store free vector in a space.
    typedef phys::euclidean_vector<TCoordinate, SPATIAL_DIMENSION> TPosition; ///< Type for store position in a space.
    typedef phys::vector<TLength, SPATIAL_DIMENSION> TCuboidSize;         ///< Type for store a size of cuboid object.
    typedef phys::vector<TCoordinate, PLANE_DIMENSION> TPlanePosition;    ///< Type for store position in a plane.
    typedef phys::vector<TLength, PLANE_DIMENSION> TPlaneDimension;       ///< Type for store position in a plane.
    typedef phys::solid_angle<TAngle, SPATIAL_DIMENSION> TSolidAngle;     ///< Type for store spheric angle.
    typedef phys::vector<unsigned, PLANE_DIMENSION> TPlaneDistribution;   ///< Type for store object plane distribution.

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Namespace-wide interfaces.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /// An interface for the components which could be assembled.
    class IAssemblable
    {
    public:
        /// A virtual destructor.
        virtual ~IAssemblable() {}

        /// Assemble component.
        virtual void Assemble() = 0;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
