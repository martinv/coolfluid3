// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibMesh_hpp
#define CF_LibMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Mesh_API
/// @note build system defines COOLFLUID_MESH_EXPORTS when compiling MeshTools files
#ifdef COOLFLUID_MESH_EXPORTS
#   define Mesh_API      CF_EXPORT_API
#   define Mesh_TEMPLATE
#else
#   define Mesh_API      CF_IMPORT_API
#   define Mesh_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  /// Basic Classes for Mesh applications used by CF
  namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Mesh
  /// @author Tiago Quintino
  class Mesh_API LibMesh :  public Common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibMesh> Ptr;
    typedef boost::shared_ptr<LibMesh const> ConstPtr;

    /// Constructor
    LibMesh ( const std::string& name) : Common::CLibrary(name) { BuildComponent<none>().build(this); }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.Mesh"; }


    /// Static function that returns the module name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "Mesh"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the mesh manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibMesh"; }

    /// initiate library
    virtual void initiate();

    /// terminate library
    virtual void terminate();

  }; // end LibMesh

////////////////////////////////////////////////////////////////////////////////

  /// Enumeration of the dimensions
  enum Dim                  { DIM_0D, DIM_1D, DIM_2D, DIM_3D };
  /// Enumeration of the coordinates indexes
  enum CoordXYZ             { XX, YY, ZZ };
  /// Enumeration of the reference coordinates indexes
  enum CoordRef             { KSI, ETA, ZTA };

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LibMesh_hpp
