/*
  -------------------------------------------------------------------

  Copyright (C) 2015-2016, Andrew W. Steiner

  This file is part of btmanip.
  
  btmanip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.
  
  btmanip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with btmanip. If not, see <http://www.gnu.org/licenses/>.

  -------------------------------------------------------------------
*/ 
#ifndef BTMANIP_HDF_BIBTEX_H
#define BTMANIP_HDF_BIBTEX_H
/** \file hdf_bibtex.h
    \brief File defining HDF I/O for selected \o2 objects
*/
#include <sstream>

#include <boost/numeric/ublas/vector.hpp>

#include "bibtexentry.hpp"
#include "bib_file.h"

#include <o2scl/hdf_file.h>
#include <o2scl/hdf_io.h>

namespace btmanip {

  /** \brief Desc
   */
  void hdf_output(o2scl_hdf::hdf_file &hf, bibtex::BibTeXEntry &ent, 
		  std::string name) {
  
    // Start group
    hid_t top=hf.get_current_id();
    hid_t group=hf.open_group(name);
    hf.set_current_id(group);

    // Add typename
    hf.sets_fixed("o2scl_type","vector<BibTeXEntry>");
    
    std::ostringstream strout;
    bib_file bf;
    bf.bib_output_one(strout,ent);
    
    hf.sets(name,strout.str());

    // Close group
    hf.close_group(group);

    // Return location to previous value
    hf.set_current_id(top);

    return;
  }

  /** \brief Desc
   */
  void hdf_output(o2scl_hdf::hdf_file &hf,
		  std::vector<bibtex::BibTeXEntry> &ents, 
		  std::string name) {
  
    // Start group
    hid_t top=hf.get_current_id();
    hid_t group=hf.open_group(name);
    hf.set_current_id(group);

    // Add typename
    hf.sets_fixed("o2scl_type","vector<BibTeXEntry>");

    std::ostringstream strout;
    bib_file bf;
    for(size_t i=0;i<ents.size();i++) {
      bf.bib_output_one(strout,ents[i]);
      strout << std::endl;
    }
    
    hf.sets(name,strout.str());

    // Close group
    hf.close_group(group);

    // Return location to previous value
    hf.set_current_id(top);

    return;
  }

  /** \brief Desc
   */
  void hdf_input(o2scl_hdf::hdf_file &hf,
		 std::vector<bibtex::BibTeXEntry> &ents, 
		 std::string name="") {

    // If no name specified, find name of first group of specified type
    if (name.length()==0) {
      hf.find_group_by_type("vector<BibTeXEntry>",name);
      if (name.length()==0) {
	O2SCL_ERR2("No object of type vector<BibTeXEntry> found in ",
		   "hdf_input().",o2scl::exc_efailed);
      }
    }
  
    // Open main group
    hid_t top=hf.get_current_id();
    hid_t group=hf.open_group(name);
    hf.set_current_id(group);

    std::string s;
    hf.gets(name,s);
    std::istringstream iss(s);
    bibtex::read(iss,ents); 

    // Close group
    hf.close_group(group);

    // Return location to previous value
    hf.set_current_id(top);

    return;
  }

}

#endif
