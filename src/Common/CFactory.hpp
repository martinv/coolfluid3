// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Common_CFactory_hpp
#define CF_Common_CFactory_hpp

/////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Common/CommonAPI.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component class for a factory which builds other components
/// @author Tiago Quintino
class Common_API CFactory : public Component
{
public:

  typedef boost::shared_ptr<CFactory> Ptr;
  typedef boost::shared_ptr<CFactory const> ConstPtr;

  /// @brief Contructor
  /// @param name of component
  CFactory(const std::string& name);

  /// @brief Virtual destructor.
  virtual ~CFactory();

  /// @returns the class name
  static std::string type_name() { return "CFactory"; }

  /// Configuration properties
  static void define_config_properties ( CF::Common::PropertyList& props );

  /// @return the name of the type of this factory
  virtual std::string factory_type_name() const = 0;

}; // CFactory

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component class for a factory which builds other components
/// @author Tiago Quintino
template < typename TYPE >
class CFactoryT : public CFactory
{
public:

  typedef boost::shared_ptr< CFactoryT<TYPE> > Ptr;
  typedef boost::shared_ptr< CFactoryT<TYPE> const> ConstPtr;

  /// @brief Contructor
  /// @param name of component
  CFactoryT(const std::string& name) : CFactory(name)
  {
    BuildComponent<none>().build(this);
  }

  /// @brief Virtual destructor.
  virtual ~CFactoryT() {};

  /// @returns the class name
  static std::string type_name() { return "CFactoryT<" + TYPE::type_name() + ">"; }

  /// @return the name of the type of this factory
  virtual std::string factory_type_name() const { return TYPE::type_name(); }

}; // CFactoryT

/////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CFactory_hpp
