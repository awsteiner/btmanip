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
#include "bib_file.h"
#include "hdf_bibtex.h"

#include <o2scl/cli_readline.h>
#include <o2scl/string_conv.h>

using namespace std;
using namespace o2scl;
using namespace btmanip;

/** \brief Main class for the command-line BibTeX manipulator

    \future Use the bibtex-spirit writer
*/
class btmanip_class {

protected:
  
  /// Command-line interface
  o2scl::cli_readline cl;

  /// \name Parameters for 'set' command
  //@{
  o2scl::cli::parameter_int p_verbose;
  o2scl::cli::parameter_bool p_recase_tag;
  o2scl::cli::parameter_bool p_reformat_journal;
  o2scl::cli::parameter_bool p_trans_latex_html;
  o2scl::cli::parameter_bool p_normalize_tags;
  o2scl::cli::parameter_bool p_lowercase_fields;
  o2scl::cli::parameter_bool p_check_required;
  o2scl::cli::parameter_bool p_natbib_jours;
  o2scl::cli::parameter_bool p_remove_vol_letters;
  o2scl::cli::parameter_bool p_autoformat_urls;
  o2scl::cli::parameter_bool p_add_empty_titles;

  o2scl::cli::parameter_string p_jlist_fname;
  //@}
  
  /// A file of BibTeX entries
  bib_file bf;

  /// If true, the journal list has been read
  bool jlist_read;
  
  /// Journal list
  std::string jlist_fname;

  /** \brief Read journal list file
   */
  virtual int read_jlist(std::vector<std::string> &sv, bool itive_com) {
    if (sv.size()<2) {
      cerr << "Command 'read-jlist' needs filename." << endl;
      return 1;
    }
    int ret=bf.read_journals(sv[1]);
    if (ret!=o2scl::exc_efilenotfound) {
      jlist_read=true;
    } else {
      O2SCL_ERR((((string)"Failed to read journal list file named '")+
		 sv[1]+"'.").c_str(),o2scl::exc_efilenotfound);
    }
    return 0;
  }

  /** \brief Desc
   */
  virtual int set_field(std::vector<std::string> &sv, bool itive_com) {
    if (sv.size()==4) {
      bf.set_field_value(sv[1],sv[2],sv[3]);
    } else if (sv.size()==3) {
      if (bf.entries.size()==1) {
	bf.set_field_value(bf.entries[0],sv[1],sv[2]);
      }
    }
    return 0;
  }

  /** \brief Search among current entries
   */
  virtual int search(std::vector<std::string> &sv, bool itive_com) {
    if (sv.size()==3) {
      std::vector<std::string>::iterator it=sv.begin();
      sv.erase(it);
      bf.search_or(sv);
    } else if (sv[1]=="or") {
      std::vector<std::string>::iterator it=sv.begin();
      sv.erase(it);
      it=sv.begin();
      sv.erase(it);
      bf.search_or(sv);
    } else if (sv[1]=="and") {
      std::vector<std::string>::iterator it=sv.begin();
      sv.erase(it);
      it=sv.begin();
      sv.erase(it);
      bf.search_and(sv);
    } else {
      cerr << "Failed in search." << endl;
      return 1;
    }
    return 0;
  }
  
  /** \brief Subtract the current entries from a .bib file
   */
  virtual int sub(std::vector<std::string> &sv, bool itive_com) {

    if (sv.size()<2) {
      cerr << "Not enough arguments to 'sub'" << endl;
      return 1;
    }

    bib_file bf2;
    bf2.parse_bib(sv[1]);

    bool found_duplicate=false;

    for(size_t i=0;i<bf.entries.size();) {

      bool found=false;

      for(size_t j=0;j<bf2.entries.size();j++) {
	std::string key1=*(bf.entries[i].key);
	std::string key2=*(bf2.entries[j].key);
	if (key1==key2 && bf.entries[i].tag==bf2.entries[j].tag) {
	  cout << "Duplicate keys and duplicate tags: "
	       << bf.entries[i].tag << " " << key1 << endl;
	  bf.entries.erase(bf.entries.begin()+i);
	  j=bf2.entries.size();
	  found_duplicate=true;
	  found=true;
	}
      }
      if (!found) i++;
    }

    // Remake 'sort' object if necessary
    if (found_duplicate) {
      bf.sort.clear();
      for(size_t i=0;i<bf.entries.size();i++) {
	bibtex::BibTeXEntry &bt=bf.entries[i];
	if (bf.sort.find(*bt.key)==bf.sort.end()) {
	  bf.sort.insert(make_pair(*bt.key,i));
	}
      }
    }
    return 0;
  }

  /** \brief Desc
   */
  virtual int hdf5(std::vector<std::string> &sv, bool itive_com) {

    o2scl_hdf::hdf_file hf;
    hf.compr_type=1;
    hf.open_or_create(sv[1]);
    hdf_output(hf,bf.entries,"btmanip");
    hf.close();
    
    return 0;
  }
  
  /** \brief Desc
   */
  virtual int parse_hdf5(std::vector<std::string> &sv, bool itive_com) {

    bf.entries.clear();
    bf.sort.clear();

    o2scl_hdf::hdf_file hf;
    hf.open(sv[1]);
    std::string name;
    hdf_input(hf,bf.entries,name);
    hf.close();

    bf.refresh_sort();
    
    return 0;
  }
  
  /** \brief Find duplicates between two .bib files
   */
  virtual int sort(std::vector<std::string> &sv, bool itive_com) {
    bf.sort_bib();
    return 0;
  }

  /** \brief Find duplicates between two .bib files
   */
  virtual int dup(std::vector<std::string> &sv, bool itive_com) {

    bool found=false;

    if (sv.size()==2) {
      if (bf.entries.size()==0) {
	cerr << "No BibTeX entries to compare to." << endl;
	return 1;
      }
      bib_file bf2;
      bf2.parse_bib(sv[1]);
      for(size_t i=0;i<bf.entries.size();i++) {
	for(size_t j=0;j<bf2.entries.size();j++) {
	  std::string key1=*(bf.entries[i].key);
	  std::string key2=*(bf2.entries[j].key);
	  if (key1==key2 && bf.entries[i].tag==bf2.entries[j].tag) {
	    cout << "Duplicate: " << bf.entries[i].tag << " "
		 << key1 << endl;
	    found=true;
	  }
	}
      }
    } else if (sv.size()>=3) {
      bib_file bf2;
      bf2.parse_bib(sv[1]);
      bib_file bf3;
      bf3.parse_bib(sv[2]);
      for(size_t i=0;i<bf2.entries.size();i++) {
	for(size_t j=0;j<bf3.entries.size();j++) {
	  std::string key1=*(bf2.entries[i].key);
	  std::string key2=*(bf3.entries[j].key);
	  if (key1==key2 && bf2.entries[i].tag==bf3.entries[j].tag) {
	    cout << "Duplicate: " << bf2.entries[i].tag << " "
		 << key1 << endl;
	    found=true;
	  }
	}
      }
    } else {
      if (bf.verbose>0) {
	cout << "Looking for duplicates among current BibTeX entries."
	     << endl;
      }
      bool restart=true;
      while (restart) {
	restart=false;
	for(size_t i=0;i<bf.entries.size();i++) {
	  for(size_t j=i+1;j<bf.entries.size();j++) {
	    bibtex::BibTeXEntry &bt=bf.entries[i];
	    bibtex::BibTeXEntry &bt2=bf.entries[j];
	    int dup_val=bf.possible_duplicate(bt,bt2);
	    if (dup_val==1) {
	      cout << "Duplicate key." << endl;
	      bf.bib_output_one(cout,bt);
	      bf.bib_output_one(cout,bt2);
	      cout << "Keep first, second, both, or rename (f,s,b,r)? "
		   << flush;
	      char ch;
	      cin >> ch;
	      if (ch=='f') {
		vector<bibtex::BibTeXEntry>::iterator it=bf.entries.begin();
		it+=j;
		bf.entries.erase(it);
		restart=true;
		i=bf.entries.size();
		j=bf.entries.size();
	      } else if (ch=='s') {
		vector<bibtex::BibTeXEntry>::iterator it=bf.entries.begin();
		it+=i;
		bf.entries.erase(it);
		restart=true;
		i=bf.entries.size();
		j=bf.entries.size();
	      } else if (ch=='r') {
		// Get new names
		cout << "Enter new name for first entry:\n" << flush;
		std::string new1;
		cin >> new1;
		cout << "Enter new name for second entry:\n" << flush;
		std::string new2;
		cin >> new2;
		// Set new name
		*bt.key=new1;
		*bt2.key=new2;
		// Remake 'sort' object
		bf.sort.clear();
		for(size_t i=0;i<bf.entries.size();i++) {
		  bibtex::BibTeXEntry &bt=bf.entries[i];
		  if (bf.sort.find(*bt.key)==bf.sort.end()) {
		    bf.sort.insert(make_pair(*bt.key,i));
		  }
		}
	      }
	      found=true;
	    } else if (dup_val==2) {
	      cout << "Possible duplicate between "
		   << *bt.key << " and " << *bt2.key << endl;
	      cout << "  journal: " << bf.get_field(bt,"journal") << endl;
	      cout << "  volume:  " << bf.get_field(bt,"volume") << endl;
	      cout << "  page:    "
		   << bf.first_page(bf.get_field(bt,"pages")) << endl;
	      cout << endl;
	      cout << "Keep first, second, or both (f,s,b)? ";
	      char ch;
	      cin >> ch;
	      if (ch=='f') {
		vector<bibtex::BibTeXEntry>::iterator it=bf.entries.begin();
		it+=j;
		bf.entries.erase(it);
		restart=true;
		i=bf.entries.size();
		j=bf.entries.size();
	      } else if (ch=='s') {
		vector<bibtex::BibTeXEntry>::iterator it=bf.entries.begin();
		it+=i;
		bf.entries.erase(it);
		restart=true;
		i=bf.entries.size();
		j=bf.entries.size();
	      }
	      found=true;
	    }
	  }
	}
      }
    }
    if (found==false && bf.verbose>0) {
      cout << "No duplicates found." << endl;
    }
    
    return 0;
  }
  
  /** \brief Parse a .bib file
   */
  virtual int parse(std::vector<std::string> &sv, bool itive_com) {

    if (sv.size()<2) {
      cerr << "Command 'parse' needs filename." << endl;
      return 1;
    }

    bf.parse_bib(sv[1]);
    return 0;
  }

  /** \brief Add a .bib file
   */
  virtual int add(std::vector<std::string> &sv, bool itive_com) {

    if (sv.size()<2) {
      cerr << "Command 'add' needs filename." << endl;
      return 1;
    }

    bf.add_bib(sv[1]);
    return 0;
  }

  /** \brief Output the BibTeX data as plain text
   */
  virtual int text(std::vector<std::string> &sv, bool itive_com) {

    ostream *outs=&cout;
    ofstream fout;
    if (sv.size()>1) {
      std::string fname=sv[1];
      fout.open(fname);
      outs=&fout;
    }

    for(size_t i=0;i<bf.entries.size();i++) {
      bibtex::BibTeXEntry &bt=bf.entries[i];
      bf.text_output_one(*outs,bt);
      if (i+1<bf.entries.size()) (*outs) << endl;
    }
    
    if (sv.size()>1) {
      fout.close();
    }

    return 0;
  }
  
  /** \brief Output the BibTeX data as .tex for input in a CV
   */
  virtual int cv(std::vector<std::string> &sv, bool itive_com) {

    ostream *outs=&cout;
    ofstream fout;
    if (sv.size()>1) {
      std::string fname=sv[1];
      fout.open(fname);
      outs=&fout;
    }

    bool cite_footnote=false;
    
    std::string stmp;
    std::vector<std::string> slist;
    for(size_t i=0;i<bf.entries.size();i++) {
      bibtex::BibTeXEntry &bt=bf.entries[i];

      // Title
      std::string title=bf.get_field(bt,"title");
      if (bf.is_field_present(bt,"title_latex")) {
	title=bf.get_field(bt,"title_latex");
      }
      stmp=((string)"\\noindent ")+std::to_string(i+1)+". ``"+
	title+"'', \\\\";
      rewrap(stmp,slist);
      for(size_t k=0;k<slist.size();k++) {
	(*outs) << slist[k] << std::endl;
      }

      // Authors
      stmp=bf.author_firstlast(bf.get_field(bt,"author"))+", \\\\";
      rewrap(stmp,slist);
      for(size_t k=0;k<slist.size();k++) {
	(*outs) << slist[k] << std::endl;
      }

      if (bf.is_field_present(bt,"doi")) {
	// DOI link and reference
	(*outs) << "\\href{http://dx.doi.org/"
		<< bf.get_field(bt,"doi") << "}" << endl;
	(*outs) << "{{\\it " << bf.get_field(bt,"journal")
		<< "} {\\bf " << bf.get_field(bt,"volume")
		<< "} (" << bf.get_field(bt,"year")
		<< ") " << bf.first_page(bf.get_field(bt,"pages"))
		<< ".} \\\\" << endl;
      } else if (bf.is_field_present(bt,"journal")) {
	(*outs) << "{\\it " << bf.get_field(bt,"journal")
		<< " (submitted)} \\\\" << endl;
      }
      
      if (bf.is_field_present(bt,"eprint")) {
	(*outs) << "(\\href{http://www.arxiv.org/abs/"
		<< bf.get_field(bt,"eprint") << "}{arXiv:"
		<< bf.get_field(bt,"eprint") << "}";
	if (bf.is_field_present(bt,"citations")
	    && bf.get_field(bt,"citations")!=((string)"0")) {
	  if (bf.get_field(bt,"citations")==((string)"1")) {
	    (*outs) << " - " << bf.get_field(bt,"citations")
		    << " citation";
	  } else {
	    (*outs) << " - " << bf.get_field(bt,"citations")
		    << " citations";
	  }
	  if (cite_footnote==false) {
	    (*outs) << "\\footnote{Citation counts from {\\tt"
		    << " inspirehep.net}.}";
	    cite_footnote=true;
	  }
	}
	(*outs) << ")\\\\" << endl;
      }
      (*outs) << endl;
    }
    
    if (sv.size()>1) {
      fout.close();
    }

    return 0;
  }
  
  /** \brief Output the BibTeX data as .tex for input in a CV
   */
  virtual int cv_talks(std::vector<std::string> &sv, bool itive_com) {

    ostream *outs=&cout;
    ofstream fout;
    if (sv.size()>1) {
      std::string fname=sv[1];
      fout.open(fname);
      outs=&fout;
    }

    bool cite_footnote=false;
    
    std::string stmp;
    std::vector<std::string> slist;
    for(size_t i=0;i<bf.entries.size();i++) {
      bibtex::BibTeXEntry &bt=bf.entries[i];

      // Title
      std::string title=bf.get_field(bt,"title");
      if (bf.is_field_present(bt,"title_latex")) {
	title=bf.get_field(bt,"title_latex");
      }
      stmp=((string)"\\noindent ")+std::to_string(i+1)+". ``"+
	title+"'' ("+bf.get_field(bt,"type")+"), \\\\";
      rewrap(stmp,slist);
      for(size_t k=0;k<slist.size();k++) {
	(*outs) << slist[k] << std::endl;
      }

      // Conference and institution
      if (bf.is_field_present(bt,"conference") &&
	  bf.get_field(bt,"conference").length()>0) {
	if (bf.is_field_present(bt,"url") &&
	    bf.get_field(bt,"url").length()>0) {
	  (*outs) << "{\\bf \\href{" << bf.get_field(bt,"url")
		  << "}{"<< bf.get_field(bt,"conference")
		  << "}}, \\\\" << endl;
	} else {
	  (*outs) << "{\\bf " << bf.get_field(bt,"conference")
		  << "}, \\\\" << endl;
	}
      }
      if (bf.is_field_present(bt,"institution") &&
	  bf.get_field(bt,"institution").length()>0) {
	(*outs) << bf.get_field(bt,"institution") << ", ";
      }

      // Location
      (*outs) << bf.get_field(bt,"city") << ", ";
      if (bf.is_field_present(bt,"state") &&
	  bf.get_field(bt,"state").length()>0) {
	(*outs) << bf.get_field(bt,"state") << ", ";
      }
      if (bf.is_field_present(bt,"country") &&
	  bf.get_field(bt,"country").length()>0 &&
	  bf.get_field(bt,"country")!=((string)"USA")) {
	(*outs) << bf.get_field(bt,"country") << ", ";
      }

      // Date
      string mon=bf.get_field(bt,"month");
      if (mon.length()<3) {
	O2SCL_ERR("Problem in month.",o2scl::exc_einval);
      }
      if (mon=="May") {
	(*outs) << bf.get_field(bt,"month") << ". ";
      } else {
	(*outs) << mon[0] << mon[1] << mon[2] << ". ";
      }
      (*outs) << bf.get_field(bt,"year") << ".\\\\" << endl;

      (*outs) << endl;
    }
    
    if (sv.size()>1) {
      fout.close();
    }

    return 0;
  }
  
  /** \brief Output the BibTeX data as .tex for input in a NSF bio sketch
   */
  virtual int nsf(std::vector<std::string> &sv, bool itive_com) {

    ostream *outs=&cout;
    ofstream fout;
    if (sv.size()>1) {
      std::string fname=sv[1];
      fout.open(fname);
      outs=&fout;
    }

    std::string stmp;
    std::vector<std::string> slist;
    for(size_t i=0;i<bf.entries.size();i++) {
      bibtex::BibTeXEntry &bt=bf.entries[i];

      if (bf.is_field_present(bt,"doi")) {
	// DOI link and reference
	(*outs) << "\\item \\href{http://dx.doi.org/"
		<< bf.get_field(bt,"doi") << "}" << endl;
      } else {
	(*outs) << "\\item \\href{http://www.arxiv.org/abs/"
		<< bf.get_field(bt,"eprint") << "}{arXiv:"
		<< bf.get_field(bt,"eprint") << "}" << endl;
      }
      
      // Title
      std::string title=bf.get_field(bt,"title");
      if (bf.is_field_present(bt,"title_latex")) {
	title=bf.get_field(bt,"title_latex");
      }
      stmp=((string)"{\\emph{")+title+"}} \\\\";
      rewrap(stmp,slist);
      for(size_t k=0;k<slist.size();k++) {
	(*outs) << slist[k] << std::endl;
      }
      
      // Authors
      stmp=bf.author_firstlast(bf.get_field(bt,"author"))+", \\\\";
      rewrap(stmp,slist);
      for(size_t k=0;k<slist.size();k++) {
	if (k!=slist.size()-1) {
	  (*outs) << slist[k] << std::endl;
	} else {
	  (*outs) << slist[k] << ", ";
	}
      }
      (*outs) << bf.get_field(bt,"year") << ", ";
      (*outs) << bf.get_field(bt,"journal") << ", \\textbf{";
      (*outs) << bf.get_field(bt,"volume") << "},";
      (*outs) << bf.first_page(bf.get_field(bt,"pages")) << "." << endl;
      (*outs) << endl;
    }
    
    if (sv.size()>1) {
      fout.close();
    }

    return 0;
  }
  
  /** \brief Output the BibTeX data as a new .bib file
   */
  virtual int bib(std::vector<std::string> &sv, bool itive_com) {
    
    ostream *outs=&cout;
    ofstream fout;
    if (sv.size()>1) {
      std::string fname=sv[1];
      fout.open(fname);
      outs=&fout;
    }
    
    for(size_t i=0;i<bf.entries.size();i++) {
      bibtex::BibTeXEntry &bt=bf.entries[i];
      bf.bib_output_one(*outs,bt);
      if (i+1<bf.entries.size()) (*outs) << endl;
    }

    if (sv.size()>1) {
      fout.close();
    }
    
    return 0;
  }
  
  /** \brief Get one bibtex entry
   */
  virtual int get_key(std::vector<std::string> &sv, bool itive_com) {

    bibtex::BibTeXEntry &bt=bf.get_entry_by_key(sv[1]);
    bf.bib_output_one(cout,bt);
    
    return 0;
  }
  
  /** \brief List current keys
   */
  virtual int list_keys(std::vector<std::string> &sv, bool itive_com) {
    
    vector<string> klist, kscreen;
    
    for(size_t k=0;k<bf.entries.size();k++) {
      klist.push_back(o2scl::szttos(k)+". "+*(bf.entries[k].key));
    }

    screenify(klist.size(),klist,kscreen);
    
    for(size_t k=0;k<kscreen.size();k++) {
      cout << kscreen[k] << endl;
    }
    
    return 0;
  }
  
  /** \brief Output the BibTeX data as a new .bib file 
      suitable for proposals
  */
  virtual int proposal(std::vector<std::string> &sv, bool itive_com) {

    ostream *outs=&cout;
    ofstream fout;
    if (sv.size()>1) {
      std::string fname=sv[1];
      fout.open(fname);
      outs=&fout;
    }
    
    for(size_t i=0;i<bf.entries.size();i++) {

      bibtex::BibTeXEntry &bt=bf.entries[i];

      // Output tag and key
      (*outs) << "@" << bt.tag << "{";
      if (bt.key) (*outs) << *bt.key;
      (*outs) << "," << endl;

      // Go through each field
      for(size_t j=0;j<bt.fields.size();j++) {

	// If it's a number, eprint, volume, or ISBN, skip it.
	if (bt.fields[j].first!=((string)"number") &&
	    bt.fields[j].first!=((string)"eprint") &&
	    bt.fields[j].first!=((string)"ISBN")) {
	  
	  if (bt.fields[j].first==((string)"pages")) {
	    
	    // If it's a set of pages, only print out
	    // the first page
	    if (bt.fields[j].second.size()>0) {
	      string first_page=bf.first_page(bt.fields[j].second[0]);
	      (*outs) << bt.fields[j].first << " = {";
	      (*outs) << first_page << "}," << endl;
	    }

	  } else if (bt.fields[j].second.size()>0) {
	    // Output other fields
	    (*outs) << bt.fields[j].first << " = {";
	    (*outs) << bt.fields[j].second[0] << "}," << endl;
	  }
	}
      }
      (*outs) << "}" << endl;
      (*outs) << endl;
    }
    
    if (sv.size()>1) {
      fout.close();
    }

    return 0;
  }

  /** \brief Add a field to the remove list
   */
  virtual int remove_field(std::vector<std::string> &sv, bool itive_com) {
    if (sv.size()<2) {
      cerr << "Command 'remove-field' needs filename." << endl;
      return 1;
    }
    bool found=false;
    for(size_t k=0;k<bf.remove_fields.size();k++) {
      if (sv[1]==bf.remove_fields[k]) found=true;
    }
    if (found==false) {
      bf.remove_fields.push_back(sv[1]);
      cout << "Removing fields:" << endl;
      for(size_t k=0;k<bf.remove_fields.size();k++) {
	cout << k << " " << bf.remove_fields[k] << endl;
      }
    } else {
      cerr << "Field " << sv[1] << " already present in remove list."
	   << endl;
      return 2;
    }
    return 0;
  }

  /** \brief Remove a field from the remove list
   */
  virtual int keep_field(std::vector<std::string> &sv, bool itive_com) {
    if (sv.size()<2) {
      cerr << "Command 'keep-field' needs filename." << endl;
      return 1;
    }
    bool found=false;
    for(std::vector<std::string>::iterator it=bf.remove_fields.begin();
	it!=bf.remove_fields.end();it++) {
      if (sv[1]==(*it)) {
	bf.remove_fields.erase(it);
	it=bf.remove_fields.begin();
	found=true;
      }
    }
    if (found==false) {
      cerr << "Field " << sv[1] << " not present in remove list."
	   << endl;
      cout << "Removing fields:" << endl;
      for(size_t k=0;k<bf.remove_fields.size();k++) {
	cout << k << " " << bf.remove_fields[k] << endl;
      }
      return 2;
    }
    cout << "Removing fields:" << endl;
    for(size_t k=0;k<bf.remove_fields.size();k++) {
      cout << k << " " << bf.remove_fields[k] << endl;
    }
    return 0;
  }

  /** \brief Desc
   */
  virtual int clean(std::vector<std::string> &sv, bool itive_com) {

    if (jlist_read==false && jlist_fname.length()>0) {
      vector<string> sv={"",jlist_fname};
      read_jlist(sv,false);
    }

    bf.clean();
    return 0;
  }
  
  /** \brief Output in a short HTML author-year format
   */
  virtual int hay(std::vector<std::string> &sv, bool itive_com) {

    ostream *outs=&cout;
    ofstream fout;
    if (sv.size()>1) {
      std::string fname=sv[1];
      fout.open(fname);
      outs=&fout;
    }

    for(size_t i=0;i<bf.entries.size();i++) {
      bibtex::BibTeXEntry &bt=bf.entries[i];

      if (bf.is_field_present(bt,"url") &&
	  bf.get_field(bt,"url").length()>0) {
	(*outs) << "<a href=\"" << bf.get_field(bt,"url")
		<< "\">" << bf.reformat_latex_html(bf.short_author(bt)) << " ("
		<< bf.get_field(bt,"year") << ")</a><br>" << endl;
      } else if (bf.is_field_present(bt,"doi") &&
		 bf.get_field(bt,"doi").length()>0) {
	(*outs) << "<a href=\"http://dx.doi.org/" << bf.get_field(bt,"url")
		<< "\">" << bf.reformat_latex_html(bf.short_author(bt)) << " ("
		<< bf.get_field(bt,"year") << ")</a><br>" << endl;
      } else {
	(*outs) << bf.reformat_latex_html(bf.short_author(bt)) << " ("
		<< bf.get_field(bt,"year") << ")<br>" << endl;
      }
    }
    
    if (sv.size()>1) {
      fout.close();
    }

    return 0;
  }
  
  /** \brief Output the BibTeX data as a new .bib file 
      in a doxygen format for O2scl
  */
  virtual int dox(std::vector<std::string> &sv, bool itive_com) {
    
    ostream *outs=&cout;
    ofstream fout;
    if (sv.size()>1) {
      std::string fname=sv[1];
      fout.open(fname);
      outs=&fout;
    }

    for(size_t i=0;i<bf.entries.size();i++) {
      bibtex::BibTeXEntry &bt=bf.entries[i];
      
      if (bt.key) {
	(*outs) << "    \\anchor " << *bt.key << " " << *bt.key
		<< ":" << endl;
      }

      if (bt.tag==((string)"Article")) {

	if (bf.is_field_present(bt,"author")) {
	  if (bf.is_field_present(bt,"url")) {
	    (*outs) << "    <a href=\""
		    << bf.get_field(bt,"url") << "\">" << endl;
	    (*outs) << "    "
		    << bf.author_firstlast(bf.get_field(bt,"author"))
		    << "</a>," << endl;
	  } else if (bf.is_field_present(bt,"doi")) {
	    (*outs) << "    <a href=\"http://dx.doi.org/"
		    << bf.get_field(bt,"doi") << "\">" << endl;
	    (*outs) << "    "
		    << bf.author_firstlast(bf.get_field(bt,"author"))
		    << "</a>," << endl;
	  } else {
	    (*outs) << "    "
		    << bf.author_firstlast(bf.get_field(bt,"author"))
		    << "," << endl;
	  }
	}
	if (bf.is_field_present(bt,"journal")) {
	  (*outs) << "    " << bf.get_field(bt,"journal") << " \\b ";
	}
	if (bf.is_field_present(bt,"volume")) {
	  (*outs) << bf.get_field(bt,"volume") << " ";
	}
	if (bf.is_field_present(bt,"year")) {
	  (*outs) << "(" << bf.get_field(bt,"year") << ") ";
	}
	if (bf.is_field_present(bt,"pages")) {
	  (*outs) << bf.first_page(bf.get_field(bt,"pages")) << "." << endl;
	} else {
	  (*outs) << "." << endl;
	}
	if (bf.is_field_present(bt,"title") &&
	    bf.get_field(bt,"title").length()>1) {
	  (*outs) << "    \\comment" << endl;
	  std::vector<std::string> svx;
	  rewrap(bf.get_field(bt,"title"),svx,70);
	  for(size_t kk=0;kk<svx.size();kk++) {
	    if (kk==0) {
	      (*outs) << "    Title: " << svx[kk] << endl;
	    } else {
	      (*outs) << "    " << svx[kk] << endl;
	    }
	  }
	  (*outs) << "    \\endcomment" << endl;
	}
	(*outs) << endl;

      } else if (bt.tag==((string)"InBook")) {

	if (bf.is_field_present(bt,"crossref") &&
	    bf.get_field(bt,"crossref").length()>0) {
	  
	  bibtex::BibTeXEntry &bt2=
	    bf.get_entry_by_key(bf.get_field(bt,"crossref"));
	  
	  if (bf.is_field_present(bt,"author")) {
	    (*outs) << "    "
		    << bf.author_firstlast(bf.get_field(bt,"author"))
		    << ", \"" << bf.get_field(bt2,"title")
		    << "\" in" << endl;
	  }
	  if (bf.is_field_present(bt2,"url")) {
	    (*outs) << "    <a href=\""
		    << bf.get_field(bt2,"url") << "\">" << endl;
	    (*outs) << "    "
		    << bf.get_field(bt2,"title")
		    << "</a>," << endl;
	  } else if (bf.is_field_present(bt2,"isbn")) {
	    (*outs) << "    <a href=\"http://www.worldcat.org/isbn/"
		    << bf.get_field(bt2,"isbn") << "\">" << endl;
	    (*outs) << "    "
		    << bf.get_field(bt2,"title")
		    << "</a>," << endl;
	  } else {
	    (*outs) << "    " << bf.get_field(bt,"title") << "," << endl;
	  }
	  (*outs) << "    (" << bf.get_field(bt2,"year") << ") "
		  << bf.get_field(bt2,"publisher") << ", p. "
		  << bf.get_field(bt,"pages") << "." << endl;
	  (*outs) << endl;
	} else {
	  if (bf.is_field_present(bt,"author")) {
	    (*outs) << "    "
		    << bf.author_firstlast(bf.get_field(bt,"author"))
		    << "," << endl;
	  }
	  if (bf.is_field_present(bt,"url")) {
	    (*outs) << "    <a href=\""
		    << bf.get_field(bt,"url") << "\">" << endl;
	    (*outs) << "    "
		    << bf.get_field(bt,"title")
		    << "</a>," << endl;
	  } else if (bf.is_field_present(bt,"isbn")) {
	    (*outs) << "    <a href=\"http://www.worldcat.org/isbn/"
		    << bf.get_field(bt,"isbn") << "\">" << endl;
	    (*outs) << "    "
		    << bf.get_field(bt,"title")
		    << "</a>," << endl;
	  } else {
	    (*outs) << "    " << bf.get_field(bt,"title") << "," << endl;
	  }
	  (*outs) << "    (" << bf.get_field(bt,"year") << ") "
		  << bf.get_field(bt,"publisher") << ", p. "
		  << bf.get_field(bt,"pages") << "." << endl;
	  (*outs) << endl;
	}

      } else if (bt.tag==((string)"Book")) {

	if (bf.is_field_present(bt,"author")) {
	  (*outs) << "    "
		  << bf.author_firstlast(bf.get_field(bt,"author"))
		  << "," << endl;
	}
	if (bf.is_field_present(bt,"url")) {
	  (*outs) << "    <a href=\""
		  << bf.get_field(bt,"url") << "\">" << endl;
	  (*outs) << "    "
		  << bf.get_field(bt,"title")
		  << "</a>," << endl;
	} else if (bf.is_field_present(bt,"isbn")) {
	  (*outs) << "    <a href=\"http://www.worldcat.org/isbn/"
		  << bf.get_field(bt,"isbn") << "\">" << endl;
	  (*outs) << "    "
		  << bf.get_field(bt,"title")
		  << "</a>," << endl;
	} else {
	  (*outs) << "    " << bf.get_field(bt,"title") << "," << endl;
	}
	(*outs) << "    (" << bf.get_field(bt,"year") << ") "
		<< bf.get_field(bt,"publisher");
	if (bf.is_field_present(bt,"note") &&
	    bf.get_field(bt,"note").length()>0) {
	  (*outs) << "\n    (" << bf.get_field(bt,"note") << ")";
	}
	(*outs) << ".\n" << endl;
      }
    }

    if (sv.size()>1) {
      fout.close();
    }

    return 0;
  }
  
public:
  
  btmanip_class() {
    jlist_read=false;
    jlist_fname="btmanip_jlist";
  }

  /** \brief Main class interface
   */
  virtual int run(int argc, char *argv[]) {
    
    static const int nopt=23;
    comm_option_s options[nopt]={
      {'p',"parse","Parse a specified .bib file.",1,1,"<file>","",
       new comm_option_mfptr<btmanip_class>(this,&btmanip_class::parse),
       cli::comm_option_both},
      {'a',"add","Add a specified .bib file.",1,1,"<file>","",
       new comm_option_mfptr<btmanip_class>(this,&btmanip_class::add),
       cli::comm_option_both},
      {0,"prop","Output a proposal .bib file.",0,1,"[file]","",
       new comm_option_mfptr<btmanip_class>(this,&btmanip_class::proposal),
       cli::comm_option_both},
      {0,"dox","Output a doxygen file for O2scl.",0,1,"[file]","",
       new comm_option_mfptr<btmanip_class>(this,&btmanip_class::dox),
       cli::comm_option_both},
      {'t',"text","Output a text file.",0,1,"[file]","",
       new comm_option_mfptr<btmanip_class>(this,&btmanip_class::text),
       cli::comm_option_both},
      {'c',"cv","Create output for a CV.",0,1,"[file]","",
       new comm_option_mfptr<btmanip_class>(this,&btmanip_class::cv),
       cli::comm_option_both},
      {0,"cvt","Create presentations output for a CV.",0,1,"[file]","",
       new comm_option_mfptr<btmanip_class>(this,&btmanip_class::cv_talks),
       cli::comm_option_both},
      {'n',"nsf","Output LaTeX source for an NSF bio sketch.",
       0,1,"[file]","",
       new comm_option_mfptr<btmanip_class>(this,&btmanip_class::nsf),
       cli::comm_option_both},
      {'b',"bib","Output a .bib file.",0,1,"[file]","",
       new comm_option_mfptr<btmanip_class>(this,&btmanip_class::bib),
       cli::comm_option_both},
      {'u',"sub","Subtract the current entries from a .bib file.",
       1,1,"<file>","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::sub),cli::comm_option_both},
      {'d',"dup","Find duplicates between .bib files.",0,2,
       "[file 1] [file 2]","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::dup),cli::comm_option_both},
      {'r',"remove-field","Add a field to the remove list.",1,1,
       "<field>","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::remove_field),cli::comm_option_both},
      {'k',"keep-field","Remove a field from the remove list.",1,1,
       "<field>","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::keep_field),cli::comm_option_both},
      {'j',"read-jlist","Read a new journal list.",1,1,
       "<file>","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::read_jlist),cli::comm_option_both},
      {0,"hay","Create a simple HTML author-year list.",0,1,
       "[file]","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::hay),cli::comm_option_both},
      {0,"clean","Clean.",0,0,
       "","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::clean),cli::comm_option_both},
      {0,"sort","Sort.",0,0,
       "","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::sort),cli::comm_option_both},
      {'g',"get-key","Get entry by key.",1,1,
       "<key>","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::get_key),cli::comm_option_both},
      {'f',"set-field","Get entry by key.",2,3,
       "<key>","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::set_field),cli::comm_option_both},
      {'l',"list-keys","List entry keys.",0,0,
       "","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::list_keys),cli::comm_option_both},
      {'s',"search","Search.",2,-1,
       "","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::search),cli::comm_option_both},
      {0,"hdf5","Output an HDF5 file.",1,1,
       "","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::hdf5),cli::comm_option_both},
      {0,"parse-hdf5","Parse a bibliography in an HDF5 file.",1,1,
       "","",new comm_option_mfptr<btmanip_class>
       (this,&btmanip_class::parse_hdf5),cli::comm_option_both}
    };
    cl.set_comm_option_vec(nopt,options);    
    
    p_verbose.i=&bf.verbose;
    p_verbose.help=((string)"Verbosity parameter ")+
      "(default 1).";
    cl.par_list.insert(make_pair("verbose",&p_verbose));

    p_recase_tag.b=&bf.recase_tag;
    p_recase_tag.help="If true, unify the case of the tags (default true).";
    cl.par_list.insert(make_pair("recase_tag",&p_recase_tag));
    
    p_reformat_journal.b=&bf.reformat_journal;
    p_reformat_journal.help="If true, reformat journal names (default true).";
    cl.par_list.insert(make_pair("reformat_journal",&p_reformat_journal));
    
    p_trans_latex_html.b=&bf.trans_latex_html;
    p_trans_latex_html.help=((string)"Translate LaTeX symbols to ")+
      "HTML and vice versa (default true).";
    cl.par_list.insert(make_pair("trans_latex_html",&p_trans_latex_html));
    
    p_normalize_tags.b=&bf.normalize_tags;
    p_normalize_tags.help=((string)"If true, convert tag strings ")+
      "to standard capitalization (default true).";
    cl.par_list.insert(make_pair("normalize_tags",&p_normalize_tags));
    
    p_lowercase_fields.b=&bf.lowercase_fields;
    p_lowercase_fields.help=((string)"If true, convert fields to ")+
      "lowercase (default true).";
    cl.par_list.insert(make_pair("lowercase_fields",&p_lowercase_fields));
    
    p_check_required.b=&bf.check_required;
    p_check_required.help=((string)"If true, check that all ")+
      "required fields are present (default false).";
    cl.par_list.insert(make_pair("check_required",&p_check_required));
    
    p_jlist_fname.str=&jlist_fname;
    p_jlist_fname.help="Journal list filename (default 'jlist_fname').";
    cl.par_list.insert(make_pair("jlist_fname",&p_jlist_fname));
    
    p_natbib_jours.b=&bf.natbib_jours;
    p_natbib_jours.help=((string)"Prefer natbib journal ")+
      "abbreviations (default false).";
    cl.par_list.insert(make_pair("natbib_jours",&p_natbib_jours));
    
    p_remove_vol_letters.b=&bf.remove_vol_letters;
    p_remove_vol_letters.help=((string)"Move letters in some volumes ")+
      "(default false).";
    cl.par_list.insert(make_pair("remove_vol_letters",&p_remove_vol_letters));

    p_autoformat_urls.b=&bf.autoformat_urls;
    p_autoformat_urls.help=((string)"If DOI or ISBN is present, ")+
      "autoformat URLs (default true).";
    cl.par_list.insert(make_pair("autoformat_urls",&p_autoformat_urls));

    p_add_empty_titles.b=&bf.add_empty_titles;
    p_add_empty_titles.help=((string)"If article titles are not present, ")+
      "add empty ones (default true).";
    cl.par_list.insert(make_pair("add_empty_titles",&p_add_empty_titles));

    
    cl.prompt="btmanip> ";

    std::vector<cmd_line_arg> ca;
    
    char *dc=getenv("BTMANIP_DEFAULTS");
    if (dc) {
      std::string def_args=dc;
      cout << "Using default arguments: " << def_args << endl;
      cl.process_args(dc,ca);
    }
    
    cl.process_args(argc,argv,ca);
    if (argc<=1) {
      cl.run_interactive();
    } else {
      cl.call_args(ca);
    }
    
    return 0;
  }
  
};

int main(int argc, char *argv[]) {
  
  btmanip_class bt;
  bt.run(argc,argv);

  return 0;
}
