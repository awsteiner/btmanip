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
#include <boost/numeric/ublas/vector.hpp>
#include "bibtexentry.hpp"
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
    hf.sets_fixed("o2scl_type","BibTeXEntry");

    // Add entry information
    hf.sets("tag",ent.tag);
    if (ent.key) hf.sets("key",*ent.key);
    std::vector<std::string> fields;
    for(size_t i=0;i<ent.fields.size();i++) {
      fields.push_back(ent.fields[i].first);
    }
    hf.sets_vec("fields",fields);

    for(size_t i=0;i<fields.size();i++) {
      hid_t group2=hf.open_group(fields[i]);
      hf.set_current_id(group2);

      hf.sets_vec("value",ent.fields[i].second);
    
      hf.close_group(group2);
      hf.set_current_id(group);
    }

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

    // Add size
    hf.seti("n_entries",ents.size());
  
    // Not all entries have keys, so we can't organize by key. 
    // We just file under e0, e1, e2, ...

    for(size_t i=0;i<ents.size();i++) {
      std::string gname=((std::string)"e")+std::to_string(i);
      hid_t group2=hf.open_group(gname);
      hf.set_current_id(group2);
    
      hf.sets("tag",ents[i].tag);
      if (ents[i].key) hf.sets("key",*ents[i].key);
      std::vector<std::string> fields;
      for(size_t j=0;j<ents[i].fields.size();j++) {
	fields.push_back(ents[i].fields[j].first);
      }
      hf.sets_vec("fields",fields);
    
      for(size_t j=0;j<fields.size();j++) {
	hid_t group3=hf.open_group(ents[i].fields[j].first);
	hf.set_current_id(group3);
      
	hf.sets_vec("value",ents[i].fields[j].second);
      
	hf.close_group(group3);
	hf.set_current_id(group2);
      }

      hf.close_group(group2);
      hf.set_current_id(group);
    }

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
      hf.find_group_by_type(hf,"vector<BibTeXEntry>",name);
      if (name.length()==0) {
	O2SCL_ERR2("No object of type vector<BibTeXEntry> found in ",
		   "hdf_input().",o2scl::exc_efailed);
      }
    }
  
    // Open main group
    hid_t top=hf.get_current_id();
    hid_t group=hf.open_group(name);
    hf.set_current_id(group);
  
    for(size_t i=0;i<ents.size();i++) {
      std::string gname=((std::string)"e")+std::to_string(i);
      hid_t group2=hf.open_group(gname);
      hf.set_current_id(group2);
    
      hf.gets("tag",ents[i].tag);
      hf.gets("key",*ents[i].key);
      std::vector<std::string> fields;
      hf.gets_vec("fields",fields);
    
      for(size_t j=0;j<fields.size();j++) {
	hid_t group3=hf.open_group(fields[j]);
	hf.set_current_id(group3);

	std::vector<std::string> value;
	hf.gets_vec("value",value);
      
	hf.close_group(group3);
	hf.set_current_id(group2);
      }

      hf.close_group(group2);
      hf.set_current_id(group);
    }

    // Close group
    hf.close_group(group);

    // Return location to previous value
    hf.set_current_id(top);

    return;
  }

}

#endif
