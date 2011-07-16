// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CNodes_hpp
#define CF_Mesh_CNodes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Common/CLink.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CDynTable.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// CNodes component class
/// This class stores information about the nodes of the mesh
/// @author Willem Deconinck
class Mesh_API CNodes : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CNodes> Ptr;
  typedef boost::shared_ptr<CNodes const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CNodes ( const std::string& name );

  /// Virtual destructor
  virtual ~CNodes();

  /// Get the class name
  static std::string type_name () { return "CNodes"; }

  CTable<Real>& coordinates() { return *m_coordinates; }
  const CTable<Real>& coordinates() const { return *m_coordinates; }

  CList<Uint>& rank() { return *m_rank; }
  const CList<Uint>& rank() const { return *m_rank; }

  CDynTable<Uint>& glb_elem_connectivity() { return *m_glb_elem_connectivity; }
  const CDynTable<Uint>& glb_elem_connectivity() const { return *m_glb_elem_connectivity; }

  CList<Uint>& glb_idx() { return *m_global_numbering; }
  const CList<Uint>& glb_idx() const { return *m_global_numbering; }

  virtual void resize(const Uint size);

  Uint size() const { return coordinates().size(); }

  Uint dim() const { return coordinates().row_size(); }

  bool is_ghost(const Uint idx) const;

private: // data

  boost::shared_ptr<CTable<Real> > m_coordinates;

  boost::shared_ptr<CDynTable<Uint> > m_glb_elem_connectivity;

  boost::shared_ptr<CList<Uint> > m_rank;

  boost::shared_ptr<CList<Uint> > m_global_numbering;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CNodes_hpp
