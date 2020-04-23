/*
  -------------------------------------------------------------------

  Copyright (C) 2015-2020, Andrew W. Steiner

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
		  std::string name);
  /** \brief Desc
   */
  void hdf_output(o2scl_hdf::hdf_file &hf,
		  std::vector<bibtex::BibTeXEntry> &ents, 
		  std::string name);
  /** \brief Desc
   */
  void hdf_input(o2scl_hdf::hdf_file &hf,
		 std::vector<bibtex::BibTeXEntry> &ents, 
		 std::string name="");

}

#endif
