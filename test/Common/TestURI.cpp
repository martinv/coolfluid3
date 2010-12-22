// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/URI.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( URI_TestSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructors )
{
  // empty contructor
  URI p0;
  BOOST_CHECK(p0.empty());
  BOOST_CHECK_EQUAL(p0.string().size(), (size_t)0 );

  // string constructor
  URI p1 ( "lolo" );
  BOOST_CHECK(!p1.empty());
  BOOST_CHECK_EQUAL( std::strcmp( p1.string_without_protocol().c_str(), "lolo" ), 0 );
  BOOST_CHECK_EQUAL( std::strcmp( p1.string().c_str(), "cpath:lolo" ), 0 );

  // copy constructor
  URI p2 ( "koko" );
  URI p3 ( p2 );
  BOOST_CHECK(!p2.empty());
  BOOST_CHECK(!p3.empty());
  BOOST_CHECK_EQUAL( std::strcmp( p2.string().c_str(), p3.string().c_str() ), 0 );

  URI uri_absolute ( "cpath://hostname/root/component");
  URI uri_relative ( "../component");

  URI p4(uri_absolute);
  BOOST_CHECK(!p4.empty());
  BOOST_CHECK_EQUAL( p4.string(), "cpath://hostname/root/component");
  BOOST_CHECK(p4.is_absolute());

  URI p5(uri_relative);
  BOOST_CHECK(!p5.empty());
  BOOST_CHECK_EQUAL( p5.string(), "cpath:../component");
  BOOST_CHECK(p5.is_relative());

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( concatenation )
{
  URI p0 ( "/root/dir1" );
  URI p1 ( "dir2/dir3" );

  URI p2 = p0 / p1;
  BOOST_CHECK_EQUAL( std::strcmp( p2.string_without_protocol().c_str(), "/root/dir1/dir2/dir3" ), 0 );

  URI p3;
  p3 /= p0;
  BOOST_CHECK_EQUAL( std::strcmp( p3.string_without_protocol().c_str(), "/root/dir1" ), 0 );

  URI p5 = p0 / "dir5/dir55";
  BOOST_CHECK_EQUAL( std::strcmp( p5.string_without_protocol().c_str(), "/root/dir1/dir5/dir55" ), 0 );

  URI p6 = "/root/dir6";
  BOOST_CHECK_EQUAL( std::strcmp( p6.string_without_protocol().c_str(), "/root/dir6" ), 0 );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( protocol_management )
{
  URI uri("//Root/Component");

  // URI without any protocol
  BOOST_CHECK_EQUAL( uri.protocol(), URI::Protocol::CPATH );
  BOOST_CHECK_EQUAL( uri.string(), std::string("cpath://Root/Component") );
  BOOST_CHECK_EQUAL( uri.string_without_protocol(), std::string("//Root/Component") );

  // URI with a cpath
  URI uri2("cpath://Root/Component");
  BOOST_CHECK_EQUAL( uri2.protocol(), URI::Protocol::CPATH );
  BOOST_CHECK_EQUAL( uri2.string(), std::string("cpath://Root/Component") );
  BOOST_CHECK_EQUAL( uri2.string_without_protocol(), std::string("//Root/Component") );

  // URI with a file
  URI uri3("file:///etc/fstab");
  BOOST_CHECK_EQUAL( uri3.protocol(), URI::Protocol::FILE );
  BOOST_CHECK_EQUAL( uri3.string(), std::string("file:///etc/fstab") );
  BOOST_CHECK_EQUAL( uri3.string_without_protocol(), std::string("///etc/fstab") );

  // URI with an http address
  URI uri4("http://coolfluidsrv.vki.ac.be");
  BOOST_CHECK_EQUAL( uri4.protocol(), URI::Protocol::HTTP );
  BOOST_CHECK_EQUAL( uri4.string(), std::string("http://coolfluidsrv.vki.ac.be") );
  BOOST_CHECK_EQUAL( uri4.string_without_protocol(), std::string("//coolfluidsrv.vki.ac.be") );

  // URI with an https address
  URI uri5("https://coolfluidsrv.vki.ac.be");
  BOOST_CHECK_EQUAL( uri5.protocol(), URI::Protocol::HTTPS );
  BOOST_CHECK_EQUAL( uri5.string(), std::string("https://coolfluidsrv.vki.ac.be") );
  BOOST_CHECK_EQUAL( uri5.string_without_protocol(), std::string("//coolfluidsrv.vki.ac.be") );

  // URI with a very long http address
  URI uri6("http://coolfluidsrv.vki.ac.be/redmine/projects/activity/coolfluid3?"
           "show_issues=1&show_changesets=1&show_news=1&show_documents=1&"
           "show_files=1&show_wiki_edits=1");
  BOOST_CHECK_EQUAL( uri6.protocol(), URI::Protocol::HTTP );
  BOOST_CHECK_EQUAL( uri6.string(),
                     std::string("http://coolfluidsrv.vki.ac.be/redmine/projects/activity/"
                                 "coolfluid3?show_issues=1&show_changesets=1&show_news=1&"
                                 "show_documents=1&show_files=1&show_wiki_edits=1") );
  BOOST_CHECK_EQUAL( uri6.string_without_protocol(),
                     std::string("//coolfluidsrv.vki.ac.be/redmine/projects/activity/"
                                 "coolfluid3?show_issues=1&show_changesets=1&show_news=1&"
                                 "show_documents=1&show_files=1&show_wiki_edits=1") );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( memory_failure )
{
  BOOST_CHECK_EQUAL( URI::Protocol::Convert::instance().to_str(URI::Protocol::CPATH), std::string("cpath") );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
