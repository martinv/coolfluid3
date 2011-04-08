// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <iomanip>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Solver/Actions/Proto/ElementLooper.hpp"
#include "Solver/Actions/Proto/Functions.hpp"
#include "Solver/Actions/Proto/NodeLooper.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

#include "Common/Core.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"
#include "Common/LibLoader.hpp"
#include "Common/OSystem.hpp"
#include "Common/Timer.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/SF/Types.hpp"

#include "Solver/CEigenLSS.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "UFEM/src/NavierStokesOps.hpp"

using namespace CF;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;
using namespace CF::Common;
using namespace CF::Math::MathConsts;
using namespace CF::Mesh;
using namespace CF::UFEM;

using namespace boost;

typedef std::vector<std::string> StringsT;
typedef std::vector<Uint> SizesT;

/// Probe based on a coordinate value
void probe(const Real coord, const Real val, Real& result)
{
  if(coord > -0.1 && coord < 0.1)
    result = val;
}

static boost::proto::terminal< void(*)(Real, Real, Real&) >::type const _probe = {&probe};

BOOST_AUTO_TEST_SUITE( ProtoSystemSuite )

// Solve the Navier-Stokes equations with SUPG and the bulk viscosity term
BOOST_AUTO_TEST_CASE( ProtoNavierStokesBULK )
{
  int    argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;

  const Real length = 5.;
  const Real height = 5.;
  const Uint x_segments = 25;
  const Uint y_segments = 25;
  
  const Real start_time = 0.;
  const Real end_time = 50.;
  const Real dt = 5.;
  Real t = start_time;
  const Uint write_interval = 200;
  const Real invdt = 1. / dt;
  
  const Real mu = 0.1;
  const Real rho = 100.;
  
  const RealVector2 u_lid(1., 0.);
  const RealVector2 u_wall(0., 0.);
  
  SUPGState state;
  state.u_ref = u_lid[XX];
  state.nu = mu / rho;
  state.rho = rho;
  
  // Load the required libraries (we assume the working dir is the binary path)
  LibLoader& loader = *OSystem::instance().lib_loader();
  
  const std::vector< boost::filesystem::path > lib_paths = boost::assign::list_of("../../../dso")("../../../src/Mesh/VTKLegacy");
  loader.set_search_paths(lib_paths);
  
  loader.load_library("coolfluid_mesh_vtklegacy");
  
  // Setup document structure and mesh
  CRoot::Ptr root = Core::instance().root();
  
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, length, height, x_segments, y_segments);
  
  // Linear system
  CEigenLSS& lss = *root->create_component<CEigenLSS>("LSS");
  lss.set_config_file(argv[1]);
  
  // Create output fields
  CField& u_fld = mesh->create_field2( "Velocity", CField::Basis::POINT_BASED, std::vector<std::string>(1, "u"), std::vector<CField::VarType>(1, CField::VECTOR_2D) );
  CField& p_fld = mesh->create_scalar_field("Pressure", "p", CF::Mesh::CField::Basis::POINT_BASED);
  
  // Used in increment step
  const StringsT fields = boost::assign::list_of("Velocity")("Pressure");
  const StringsT vars = boost::assign::list_of("u")("p");
  const SizesT dims = boost::assign::list_of(2)(1);
  
  lss.resize(u_fld.data().size() * 2 + p_fld.size());
  
  // Setup a mesh writer
  CMeshWriter::Ptr writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.VTKLegacy.CWriter","meshwriter");
  root->add_component(writer);
  const std::vector<URI> out_fields = boost::assign::list_of(u_fld.full_path())(p_fld.full_path());
  writer->configure_property( "Fields", out_fields );
  
  // Regions
  CRegion& left = find_component_recursively_with_name<CRegion>(*mesh, "left");
  CRegion& right = find_component_recursively_with_name<CRegion>(*mesh, "right");
  CRegion& bottom = find_component_recursively_with_name<CRegion>(*mesh, "bottom");
  CRegion& top = find_component_recursively_with_name<CRegion>(*mesh, "top");

  // Expression variables
  MeshTerm<0, VectorField> u("Velocity", "u");
  MeshTerm<1, ScalarField> p("Pressure", "p");
  
  // Set up a physical model (normally handled automatically if using the Component wrappers)
  PhysicalModel physical_model;
  physical_model.nb_dofs = 3;
  physical_model.variable_offsets["u"] = 0;
  physical_model.variable_offsets["p"] = 2;
  
  // Set initial conditions
  for_each_node(mesh->topology(), p = 0.);
  for_each_node(mesh->topology(), u = u_wall);
  
  // Timings
  Real assemblytime, bctime, increment_time, advect_time, update_advect_time;
  
  // Set up fields for velocity extrapolation
  const std::vector<std::string> advection_vars = boost::assign::list_of("u_adv")("u1")("u2")("u3");
  CField& u_adv_fld = mesh->create_field2( "AdvectionVelocity", CField::Basis::POINT_BASED, advection_vars, std::vector<CField::VarType>(4, CField::VECTOR_2D) );
  
  // Variables associated with the advection velocity
  MeshTerm<2, VectorField> u_adv("AdvectionVelocity", "u_adv"); // The extrapolated advection velocity (n+1/2)
  MeshTerm<3, VectorField> u1("AdvectionVelocity", "u1");  // Two timesteps ago (n-1)
  MeshTerm<4, VectorField> u2("AdvectionVelocity", "u2"); // n-2
  MeshTerm<5, VectorField> u3("AdvectionVelocity", "u3"); // n-3
  
  // initialize
  for_each_node(mesh->topology(), u1 = u);
  for_each_node(mesh->topology(), u2 = u);
  for_each_node(mesh->topology(), u3 = u);
  
  while(t < end_time)
  {
    Timer timer;
    
    const Uint tstep = static_cast<Uint>(t / dt);
    
    // Extrapolate velocity
    for_each_node(mesh->topology(), u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3);
    
    advect_time = timer.elapsed(); timer.restart();
    
    // Fill the system matrix
    lss.set_zero();
    
    for_each_element< boost::mpl::vector1<SF::Quad2DLagrangeP1> >
    (
      mesh->topology(),
      group(state) <<                             // Note we pass the state here, to calculate and share tau_...
      (
        set_tau(u_adv),                               // Calculate the stabilization coefficients
        _A(p, p) = continuity_p_a(u_adv),             // Continuity equation, p terms (PSPG)
        _A(p, u) = continuity_u_a(u_adv),             // Continuity equation, u terms (Standard + PSPG)
        _A(u, p) = momentum_p_a(u_adv),               // Momentum equation, p terms (Standard + SUPG)
        _A(u, u) = momentum_u_a(u_adv),               // Momentum equation, u terms (Standard + SUPG + bulk viscosity)
        //_cout << "momentum, no skew:\n" << momentum_u_a(u_adv) << "\nmomentum skew:\n" << momentum_u_a_skew(u_adv) << "\ndiff:\n" << momentum_u_a(u_adv) - momentum_u_a_skew(u_adv) << "\n",
        _T(p, u) = continuity_t(u_adv),               // Time, PSPG
        _T(u, u) = momentum_t(u_adv),                 // Time, standard and SUPG
        system_matrix(lss) += invdt * _T + 1.0 * _A,
        system_rhs(lss) -= _A * _b
      )
    );
    
    assemblytime = timer.elapsed(); timer.restart();
    
    // Set boundary conditions
    for_each_node(left,   dirichlet(lss, u) = u_wall, physical_model);
    for_each_node(right,  dirichlet(lss, u) = u_wall, physical_model);
    for_each_node(top,    dirichlet(lss, u) = u_lid,  physical_model);
    for_each_node(bottom, dirichlet(lss, u) = u_wall, physical_model);
    
    bctime = timer.elapsed(); 
    
    std::cout << "Solving for time " << t << std::endl;
    
    // Solve the system!
    lss.solve();
    
    timer.restart();
    
    // Save previous velocities for exrapolation
    for_each_node(mesh->topology(), u3 = u2);
    for_each_node(mesh->topology(), u2 = u1);
    for_each_node(mesh->topology(), u1 = u);
    update_advect_time = timer.elapsed(); timer.restart();
    
    increment_solution(lss.solution(), fields, vars, dims, *mesh);
    increment_time = timer.elapsed();
    
    const Real total_time = assemblytime + bctime + increment_time + lss.time_matrix_construction + lss.time_matrix_fill + lss.time_residual + lss.time_solve + lss.time_solver_setup + advect_time + update_advect_time;
    std::cout << "  assembly     : " << assemblytime << " (" << assemblytime/total_time*100. << "%)\n"
              << "  bc           : " << bctime << " (" << bctime/total_time*100. << "%)\n"
              << "  matrix build : " << lss.time_matrix_construction << " (" << lss.time_matrix_construction/total_time*100. << "%)\n"
              << "  matrix fill  : " << lss.time_matrix_fill << " (" << lss.time_matrix_fill/total_time*100. << "%)\n"
              << "  solver setup : " << lss.time_solver_setup << " (" << lss.time_solver_setup/total_time*100. << "%)\n"
              << "  solve        : " << lss.time_solve << " (" << lss.time_solve/total_time*100. << "%)\n"
              << "  residual     : " << lss.time_residual << " (" << lss.time_residual/total_time*100. << "%)\n"
              << "  write field  : " << increment_time << " (" << increment_time/total_time*100. << "%)\n"
              << "  extrapolate  : " << advect_time << " (" << advect_time/total_time*100. << "%)\n"
              << "  save tsteps  : " << update_advect_time << " (" << update_advect_time/total_time*100. << "%)\n"
              << "  total        : " << total_time << std::endl;
    
    t += dt;
        
    // Output solution
    if(t > 0. && (tstep % write_interval == 0 || t >= end_time))
    {
      std::stringstream outname;
      outname << "navier-stokes-lid-";
      outname << std::setfill('0') << std::setw(5) << static_cast<Uint>(t / dt);
      boost::filesystem::path output_file(outname.str() + ".vtk");
      writer->write_from_to(mesh, output_file);
    }
  }
  
  
}

BOOST_AUTO_TEST_SUITE_END()