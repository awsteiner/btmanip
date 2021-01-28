/*
  -------------------------------------------------------------------

  Copyright (C) 2015-2021, Andrew W. Steiner

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
/** \file btmanip.cpp
    \brief File defining \ref btmanip_class

    OSTI URL is 
    https://www.osti.gov/search/identifier:osti_id
*/
#include "bib_file.h"
#include "hdf_bibtex.h"

// For time()
#include <ctime>

#include <boost/algorithm/string/replace.hpp>

#include <o2scl/cli_readline.h>
#include <o2scl/string_conv.h>

#include "json.hpp"

using namespace std;
using namespace o2scl;
using namespace btmanip;

namespace btmanip {

  /** \brief Main class for the command-line BibTeX manipulator
   */
  class btmanip_class {

  protected:
  
    /// Command-line interface
    o2scl::cli_readline *cl;

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
    o2scl::cli::parameter_bool p_remove_author_tildes;

    /// A file of BibTeX entries
    bib_file bf;

    /// If true, a journal list has been read
    bool jlist_read;

    int get_screen_width() {

      int nrow, ncol=80;

      // AWS 5/20/20: I think this causes problems when run
      // in a script
      //#ifdef O2SCL_NCURSES
      // Use curses
      //get_screen_size(nrow,ncol);
      //#endif
      
      // If not, attempt to obtain the result from the environment
      char *ncstring=getenv("COLUMNS");
      if (ncstring) {
	int nc2;
	int sret=o2scl::stoi_nothrow(ncstring,nc2);
	if (sret==0 && nc2>0) {
	  ncol=nc2;
	} else {
	  cerr << "Failed to interpret COLUMNS value " << ncstring
	       << " as a positive number of columns." << endl;
	}
      }
      return ncol;
    }

    
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

    /** \brief Set the value of one field in one entry
     */
    virtual int set_field(std::vector<std::string> &sv, bool itive_com) {
      if (sv.size()==4) {
	bf.set_field_value(sv[1],sv[2],sv[3]);
      } else if (sv.size()==3) {
	if (bf.entries.size()==1) {
	  bf.set_field_value(bf.entries[0],sv[1],sv[2]);
	} else {
	  cerr << "More than one entry, thus 'set-field' requires three "
	       << "arguments." << endl;
	  return 1;
	}
      } else {
	cerr << "Not enough arguments to 'set-field'." << endl;
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
  
    /** \brief Remove matching entries
     */
    virtual int remove(std::vector<std::string> &sv, bool itive_com) {
      if (sv.size()==3) {
	std::vector<std::string>::iterator it=sv.begin();
	sv.erase(it);
	bf.remove_or(sv);
      } else if (sv[1]=="or") {
	std::vector<std::string>::iterator it=sv.begin();
	sv.erase(it);
	it=sv.begin();
	sv.erase(it);
	bf.remove_or(sv);
      } else if (sv[1]=="and") {
	std::vector<std::string>::iterator it=sv.begin();
	sv.erase(it);
	it=sv.begin();
	sv.erase(it);
	O2SCL_ERR("Remove 'and' unimplemented.",exc_eunimpl);
	//bf.remove_and(sv);
      } else {
	cerr << "Failed in remove." << endl;
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

    /** \brief Return the journal aliases for a given journal
     */
    virtual int journal(std::vector<std::string> &sv, bool itive_com) {

      if (sv.size()<2) {
	cerr << "Need journal name for 'journal' command." << endl;
	return 1;
      }
      std::string sout;
      std::vector<std::string> list;
      int ret=bf.find_abbrevs(sv[1],list);
      if (ret==1) {
	cerr << "Couldn't find journal " << sv[1] << " ." << endl;
	return 2;
      }

      for(size_t k=0;k<list.size();k++) {
	cout << k << ". " << list[k] << endl;
      }
    
      return 0;
    }

    /** \brief Output an HDF5 file
     */
    virtual int hdf5(std::vector<std::string> &sv, bool itive_com) {

      if (sv.size()<2) {
	cerr << "Command 'hdf5' needs filename." << endl;
	return 1;
      }

      o2scl_hdf::hdf_file hf;
      hf.compr_type=1;
      hf.open_or_create(sv[1]);
      hdf_output(hf,bf.entries,"btmanip");
      hf.close();
    
      return 0;
    }
  
    /** \brief Parse an HDF5 file
     */
    virtual int parse_hdf5(std::vector<std::string> &sv, bool itive_com) {

      if (sv.size()<2) {
	cerr << "Command 'parse-hdf5' needs filename." << endl;
	return 1;
      }

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

    /** \brief Get information from adsabs
     */
    virtual int ads_get(std::vector<std::string> &sv, bool itive_com) {

      int verbose=1;

      // Get API token
      std::string token;
      char *token_ptr=getenv("ADSABS_TOKEN");
      if (token_ptr) {
	token=token_ptr;
      } else {
	cerr << "Token not found in \"ads-get\"." << endl;
	return 1;
      }

      std::string prefix="curl -X GET -H 'Authorization: Bearer:"+token+
	"' ";
      std::string base_url="https://api.adsabs.harvard.edu/v1/";
            
      for(size_t i=0;i<bf.entries.size();i++) {
	bibtex::BibTeXEntry &bt=bf.entries[i];
	
	if (bf.is_field_present(bt,"doi")) {
	  
	  string doi=bf.get_field(bt,"doi");

	  while (doi.find("/")!=std::string::npos) {
	    doi.replace(doi.find("/"),1,"%2F");
	  }

	  string cmd=prefix+"\""+base_url+"search/query?q=doi:"+
	    doi+"&fl=bibcode\"";
	  cout << "cmd: " << cmd << endl;

	  string result;
	  int ret=pipe_cmd_string(cmd,result,false,1000);
	  cout << result.size() << " " << result << endl;
	  
	  auto j=nlohmann::json::parse(result);
	  auto j_bibcode=j["response"]["docs"][0]["bibcode"];
	  string bibcode=j_bibcode.get<std::string>();

	  cout << bibcode << endl;

	  cmd=prefix+"-H \"Content-Type: application/json\" "+
	    base_url+"export/bibtex -X POST -d '{\"bibcode\":[\""+
	    bibcode+"\"]}'";
	  cout << "cmd: " << cmd << endl;
	  
	  ret=pipe_cmd_string(cmd,result,false,100000);
	  cout << result.size() << " " << result << endl;

	  auto j2=nlohmann::json::parse(result);
	  cout << j2["export"] << endl;
	  string bib=j2["export"].get<std::string>();
	  cout << bib << endl;
	  
	  exit(-1);
	  
	}
      }
      return 0;
    }
    
    /** \brief Get information from inspirehep.net
     */
    virtual int inspire_get(std::vector<std::string> &sv, bool itive_com) {

      int verbose=1;
      
      for(size_t i=0;i<bf.entries.size();i++) {
	bibtex::BibTeXEntry &bt=bf.entries[i];
	
	if (bf.is_field_present(bt,"doi")) {
	  
	  string doi=bf.get_field(bt,"doi");
	  
	  if (verbose>1) {
	    cout << "doi: " << doi << endl;
	  }
	  
	  string cmd=((string)"curl -X GET \"http://old.inspire")+
	    "hep.net/search?action_search=Search&rg=1&of=recjson&"+
	    "ln=en&p=find+doi+"+doi+"&jrec=0\"";
	  string result;
	  if (verbose>1) {
	    cout << cmd << endl;
	  }
	  static const size_t nbuf=200000;
	  int ret=pipe_cmd_string(cmd,result,false,nbuf);
	  if (verbose>1) {
	    cout << "Result length: " << result.length() << endl;
	  }
	  bool dl_failed=false;
	  if (result.length()>=nbuf-1) {
	    cerr << "Inspire result was longer than buffer size." << endl;
	    dl_failed=true;
	  }
	  if (dl_failed==false) {
	    auto j=nlohmann::json::parse(result);
	    if (verbose>1) {
	      cout << "Number of inspirehep.net results: "
		   << j.size() << endl;
	    }
	    if (j.size()!=1) {
	      cerr << "Inspire search led to more than one result." << endl;
	    } else {
	      
	      bibtex::BibTeXEntry bt_new;
	      bt_new.key=bt.key;
	      bt_new.tag="Article";
	      
	      // References cited
	      //cout << "reference: " << j[0]["reference"] << endl;
	      
	      auto jauthors=j[0]["authors"];
	      string auth_list;
	      bool auth_success=true;
	      for(size_t k=0;k<jauthors.size();k++) {
		auto auth=jauthors[k];
		bool found=false;
		for (nlohmann::json::iterator it=auth.begin();
		     it != auth.end();++it) {
		  if (it.key()==((string)"full_name")) {
		    found=true;
		    string str=it.value().get<std::string>();
		    if (auth_list.length()==0) {
		      auth_list+=str;
		    } else {
		      auth_list+=((string)" and ")+str;
		    }
		  }
		}
		if (found==false) {
		  cerr << "Full name for author of index "
		       << k << " not found." << endl;
		  auth_success=false;
		}
	      }
	      if (verbose>1) {
		cout << "authors: " << auth_list << endl;
	      }
	      bf.set_field_value(bt_new,"author",auth_list);
	      
	      if (auth_success==true) {
		
		auto j_title=j[0]["title"].begin().value();
		std::string title=j_title.get<std::string>();
		if (verbose>1) {
		  cout << "title: " << bf.get_field(bt,"title") << endl;
		}
		bf.set_field_value(bt_new,"title",title);
		
		auto j_doi2=j[0]["doi"].begin().value();
		std::string doi2=j_doi2.get<std::string>();
		if (verbose>1) {
		  cout << "doi2: " << doi2 << endl;
		}
		bf.set_field_value(bt_new,"doi",doi2);
		
		auto j_eprint=j[0]["primary_report_number"].begin();
		std::string eprint=j_eprint.value().get<std::string>();
		if (eprint.substr(0,6)==((string)"arXiv:")) {
		  eprint=eprint.substr(6,eprint.length()-6);
		}
		if (verbose>1) {
		  cout << "eprint: " << eprint << endl;
		}
		bf.set_field_value(bt_new,"eprint",eprint);
		
		auto j_pages=j[0]["publication_info"]["pagination"].begin();
		std::string pages=j_pages.value().get<std::string>();
		if (verbose>1) {
		  cout << "pages: " << pages << endl;
		}
		bf.set_field_value(bt_new,"pages",pages);
		
		auto j_journal=j[0]["publication_info"]["title"].begin();
		std::string journal=j_journal.value().get<std::string>();
		if (verbose>1) {
		  cout << "journal: " << journal << endl;
		}
		bf.set_field_value(bt_new,"journal",journal);
		
		auto j_volume=j[0]["publication_info"]["volume"].begin();
		std::string volume=j_volume.value().get<std::string>();
		if (verbose>1) {
		  cout << "volume: " << volume << endl;
		}
		bf.set_field_value(bt_new,"volume",volume);
		
		auto j_year=j[0]["publication_info"]["year"].begin();
		std::string year=j_year.value().get<std::string>();
		if (verbose>1) {
		  cout << "year: " << year << endl;
		}
		bf.set_field_value(bt_new,"year",year);

		// Reformat journal and volume from inspirehep.net
		bf.entry_remove_vol_letters(bt_new);
		journal=bf.get_field(bt_new,"journal");

		// Normalize journal abbreviation
		std::string abbrev;
		if (bf.find_abbrev(journal,abbrev)==0) {
		  journal=abbrev;
		  bf.set_field_value(bt_new,"journal",journal);
		}
		
		bf.bib_output_twoup(cout,bt,bt_new,"Original",
				    "Inspirehep record");
		
		cout << "Keep original ('<' or ','), "
		     << "use new inspirehep entry ('>' or '.'), "
		     << "cherry pick ('c'), or quit ('q'): " << endl;
		char ch;
		cin >> ch;

	      }
	      
	    }
	  }
	} else {
	  cout << "No DOI present for key: " << *bt.key << endl;
	}
      }
      return 0;
    }
    
    /** \brief Sort the bib file by key
     */
    virtual int sort(std::vector<std::string> &sv, bool itive_com) {
      bf.sort_bib();
      return 0;
    }

    /** \brief Sort the bib file by date
     */
    virtual int sort_by_date(std::vector<std::string> &sv, bool itive_com) {
      if (sv.size()>=2 && sv[1]==((string)"descending")) {
	bf.sort_by_date(true);
      } else {
	bf.sort_by_date();
      }
      return 0;
    }

    /** \brief Find duplicates between two .bib files
     */
    virtual int dup(std::vector<std::string> &sv, bool itive_com) {

      bool found=false;

      std::cout << "size: " << bf.journals.size() << endl;
      
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
	size_t istart=0;
	while (restart) {
	  restart=false;
	  for(size_t i=istart;i<bf.entries.size();i++) {
	    for(size_t j=i+1;j<bf.entries.size();j++) {
	      bibtex::BibTeXEntry &bt=bf.entries[i];
	      bibtex::BibTeXEntry &bt2=bf.entries[j];
	      int dup_val=bf.possible_duplicate(bt,bt2);
	      if (dup_val==1) {
		cout << "Duplicate tag and key." << endl;
		bf.bib_output_one(cout,bt);
		bf.bib_output_one(cout,bt2);
		cout << "Keep first, second, both, rename, or quit "
		     << "(f,s,b,r,q)? " << flush;
		char ch;
		cin >> ch;
		if (ch=='f') {
		  vector<bibtex::BibTeXEntry>::iterator it=bf.entries.begin();
		  it+=j;
		  bf.entries.erase(it);
		  restart=true;
		  istart=i;
		  i=bf.entries.size();
		  j=bf.entries.size();
		} else if (ch=='s') {
		  vector<bibtex::BibTeXEntry>::iterator it=bf.entries.begin();
		  it+=i;
		  bf.entries.erase(it);
		  restart=true;
		  istart=i;
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
		} else if (ch=='q') {
		  i=bf.entries.size();
		  j=bf.entries.size();
		  restart=false;
		  cout << "Quitting early." << endl;
		}
		found=true;
	      } else if (dup_val==2) {
		cout << "Possible duplicate between "
		     << *bt.key << " and " << *bt2.key << endl;
		cout << endl;
		
		bf.bib_output_twoup(cout,bt,bt2,
				    ((std::string)"Entry ")+o2scl::szttos(i),
				    ((std::string)"Entry ")+o2scl::szttos(j));
		
		cout << "Keep left (" << *bt.key << "), right ("
		     << *bt2.key << "), both, or quit (<, , >. , b , q)? ";
		char ch;
		cin >> ch;
		if (ch=='<' || ch==',') {
		  cout << "Keeping " << *bt.key << " ." << endl;
		  vector<bibtex::BibTeXEntry>::iterator it=bf.entries.begin();
		  it+=j;
		  bf.entries.erase(it);
		  restart=true;
		  istart=i;
		  i=bf.entries.size();
		  j=bf.entries.size();
		} else if (ch=='>' || ch=='.') {
		  cout << "Keeping " << *bt2.key << " ." << endl;
		  vector<bibtex::BibTeXEntry>::iterator it=bf.entries.begin();
		  it+=i;
		  bf.entries.erase(it);
		  restart=true;
		  istart=i;
		  i=bf.entries.size();
		  j=bf.entries.size();
		} else if (ch=='q') {
		  restart=false;
		  i=bf.entries.size();
		  j=bf.entries.size();
		  cout << "Quitting early." << endl;
		} else {
		  cout << "Keeping both." << endl;
		}
		found=true;
	      }
	    }
	    if (i%50==49) {
	      std::cout << i+1 << "/" << bf.entries.size()
			<< " records processed." << std::endl;
	    }
	  }
	}
      }

      if (found==false && bf.verbose>0) {
	cout << "No duplicates found." << endl;
      }
    
      return 0;
    }

    /** \brief Parse the o2scl .bib files
     */
    virtual int o2scl(std::vector<std::string> &sv, bool itive_com) {
      std::string data_dir=o2scl_settings.get_data_dir();
      bf.add_bib(data_dir+"/o2scl.bib");
      bf.add_bib(data_dir+"/o2scl_part.bib");
      bf.add_bib(data_dir+"/o2scl_eos.bib");
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

    /** \brief Output the full BibTeX data as plain text
     */
    virtual int text_full(std::vector<std::string> &sv, bool itive_com) {

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

	// Arrange authors with only initials for first and
	// middle names
	stmp=bf.author_firstlast(bf.get_field(bt,"author"),
				 false,true)+", \\\\";
	rewrap(stmp,slist);
	for(size_t k=0;k<slist.size();k++) {
	  (*outs) << slist[k] << std::endl;
	}

	if (bf.is_field_present(bt,"doi") &&
	    bf.is_field_present(bt,"journal")) {
	  // DOI link and reference
	  (*outs) << "\\href{https://doi.org/"
		  << bf.get_field(bt,"doi") << "}" << endl;
	  (*outs) << "{{\\it " << bf.get_field(bt,"journal")
		  << "}";
	  if (bf.is_field_present(bt,"volume")) {
	    (*outs) << " {\\bf " << bf.get_field(bt,"volume")
		    << "}";
	  }
	  if (bf.is_field_present(bt,"year")) {
	    (*outs) << " (" << bf.get_field(bt,"year")
		    << ")";
	  }
	  if (bf.is_field_present(bt,"pages")) {
	    (*outs) << " " << bf.first_page(bf.get_field(bt,"pages"));
	  }
	  (*outs) << ".} \\\\" << endl;
	} else if (bf.is_field_present(bt,"journal") &&
		   bf.get_field(bt,"journal").length()>1) {
	  (*outs) << "{\\it " << bf.get_field(bt,"journal")
		  << "}. \\\\" << endl;
	}
      
	if (bf.is_field_present(bt,"eprint")) {
	  (*outs) << "(\\href{https://www.arxiv.org/abs/"
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
  
    /** \brief Output the BibTeX data as .tex for input in a 
	NSF bio sketch

	This command uses the format:

	\verbatim
	\item \href{URL}{\emph{title}} \\
	First Last, First2 Last2 year, jour, \textbf{volume}, pages.
	\endverbatim

        First and middle names are converted to initials if
	necessary. 

	The URL is set to ``https://doi.org/`` plus 
	the ``DOI`` field (if present) or ``https://www.arxiv.org/``
	plus the ``eprint`` field if no ``DOI`` field is present.
	The ``url`` field is ignored. If a field called 
	``title_latex`` is present, it is used instead of the
	``title`` field.
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
	  (*outs) << "\\item \\href{https://doi.org/"
		  << bf.get_field(bt,"doi") << "}" << endl;
	} else if (bf.is_field_present(bt,"eprint")) {
	  (*outs) << "\\item \\href{https://www.arxiv.org/abs/"
		  << bf.get_field(bt,"eprint") << "}{arXiv:"
		  << bf.get_field(bt,"eprint") << "}" << endl;
	}
      
	// Title
	std::string title;
	if (bf.is_field_present(bt,"title")) {
	  title=bf.get_field(bt,"title");
	}
	if (bf.is_field_present(bt,"title_latex")) {
	  title=bf.get_field(bt,"title_latex");
	}
	stmp=((string)"{\\emph{")+title+"}} \\\\";
	rewrap(stmp,slist);
	for(size_t k=0;k<slist.size();k++) {
	  (*outs) << slist[k] << std::endl;
	}
      
	// Authors
	if (bf.is_field_present(bt,"author")) {
	  stmp=bf.author_firstlast(bf.get_field(bt,"author"),
				   true,true)+",";
	  rewrap(stmp,slist);
	  for(size_t k=0;k<slist.size();k++) {
	    if (k!=slist.size()-1) {
	      (*outs) << slist[k] << std::endl;
	    } else {
	      (*outs) << slist[k] << " ";
	    }
	  }
	}

	if (bf.is_field_present(bt,"year")) {
	  (*outs) << bf.get_field(bt,"year") << ", ";
	}
	if (bf.is_field_present(bt,"journal")) {
	  (*outs) << bf.get_field(bt,"journal") << ", \\textbf{";
	  if (bf.is_field_present(bt,"volume")) {
	    (*outs) << bf.get_field(bt,"volume") << "}, ";
	  }
	  if (bf.is_field_present(bt,"volume")) {
	    (*outs) << bf.first_page(bf.get_field(bt,"pages")) << "." << endl;
	  }
	} else if (bf.is_field_present(bt,"eprint")) {
	  (*outs) << "arXiv:" << bf.get_field(bt,"eprint") << "." << endl;
	}
	(*outs) << endl;
      }
    
      if (sv.size()>1) {
	fout.close();
      }

      return 0;
    }

    /** \brief Reverse the order
     */
    virtual int reverse(std::vector<std::string> &sv, bool itive_com) {
      bf.reverse_bib();
      return 0;
    }
    
    /** \brief A tex format for UTK review
     */
    virtual int utk_review(std::vector<std::string> &sv, bool itive_com) {

      ostream *outs=&cout;
      ofstream fout;
      if (sv.size()>1) {
	std::string fname=sv[1];
	fout.open(fname);
	outs=&fout;
      }

      // Chronological order
      bf.reverse_bib();
      
      std::string stmp;
      std::vector<std::string> slist;
      (*outs) << "\\begin{enumerate}" << endl;
      for(size_t i=0;i<bf.entries.size();i++) {
	bibtex::BibTeXEntry &bt=bf.entries[i];
	cout << "Formatting entry " << i << " " << *bt.key << endl;

	if (bf.is_field_present(bt,"doi")) {
	  // DOI link and reference
	  (*outs) << "\\item \\href{https://doi.org/"
		  << bf.get_field(bt,"doi") << "}{" << endl;
	} else if (bf.is_field_present(bt,"eprint")) {
	  (*outs) << "\\item \\href{https://www.arxiv.org/abs/"
		  << bf.get_field(bt,"eprint") << "}{" << endl;
	} else {
	  cout << "No doi or eprint for: " << *bt.key << endl;
	}
      
	// Title
	std::string title=bf.get_field(bt,"title");
	if (bf.is_field_present(bt,"title_latex")) {
	  title=bf.get_field(bt,"title_latex");
	}
	stmp=((string)"{\\emph{")+title+"}}} \\\\";
	rewrap(stmp,slist);
	for(size_t k=0;k<slist.size();k++) {
	  (*outs) << slist[k] << std::endl;
	}
      
	// Authors
	stmp=bf.author_firstlast(bf.get_field(bt,"author"),
				 false,false)+", ";
	rewrap(stmp,slist);
	for(size_t k=0;k<slist.size();k++) {
	  if (k!=slist.size()-1) {
	    (*outs) << slist[k] << std::endl;
	  } else {
	    (*outs) << slist[k] << " ";
	  }
	}
	(*outs) << bf.get_field(bt,"year");
	if (bf.is_field_present(bt,"journal")) {
	  (*outs) << ", " << bf.get_field(bt,"journal") << ", \\textbf{";
	  (*outs) << bf.get_field(bt,"volume") << "}, ";
	  (*outs) << bf.first_page(bf.get_field(bt,"pages"));
	}
	(*outs) << ". \\\\" << endl;
	(*outs) << "~[" << bf.get_field(bt,"utknote") << "]~" << endl;
      }
      (*outs) << "\\end{enumerate}" << endl;
    
      if (sv.size()>1) {
	fout.close();
      }

      return 0;
    }
  
    /** \brief Output of talks for DOE progress reports 
     */
    virtual int doe_talks(std::vector<std::string> &sv, bool itive_com) {

      ostream *outs=&cout;
      ofstream fout;
      if (sv.size()>1) {
	std::string fname=sv[1];
	fout.open(fname);
	outs=&fout;
      }

      std::string stmp;
      int count=0;
      for(size_t i=0;i<bf.entries.size();i++) {
	bibtex::BibTeXEntry &bt=bf.entries[i];

	// Authors
        if (bf.is_field_present(bt,"author")) {
          stmp=bf.author_firstlast(bf.get_field(bt,"author"),
                                   false,false);
          bf.tilde_to_space(stmp);
          (*outs) << count+1 << ") ";
          (*outs) << stmp << ", ";
        }

        if (bf.is_field_present(bt,"month")) {
          (*outs) << bf.get_field(bt,"month") << " ";
        }
        if (bf.is_field_present(bt,"year")) {
          (*outs) << bf.get_field(bt,"year") << ", ";
        }
	
	// Title
	if (bf.is_field_present(bt,"title")) {
	  std::string title=bf.get_field(bt,"title");
	  std::vector<std::string> slist;
	  rewrap(title,slist,800);
          if (slist.size()>0) {
            (*outs) << slist[0] << ", ";
          }
	}

	// Conference
	if (bf.is_field_present(bt,"conference")) {
	  std::string conf=bf.get_field(bt,"conference");
	  std::vector<std::string> slist;
	  rewrap(conf,slist,800);
	  (*outs) << slist[0] << ", ";
	}

	// Institution
	if (bf.is_field_present(bt,"city")) {
	  (*outs) << bf.get_field(bt,"city") << ", ";
	}
	if (bf.is_field_present(bt,"country")) {
	  if (bf.is_field_present(bt,"state")) {
	    (*outs) << bf.get_field(bt,"state") << ", ";
	  }
	  (*outs) << bf.get_field(bt,"country") << ".";
	} else {
	  if (bf.is_field_present(bt,"state")) {
	    (*outs) << bf.get_field(bt,"state") << ".";
	  }
	}

	(*outs) << endl;
	count++;
      }
    
      if (sv.size()>1) {
	fout.close();
      }

      return 0;
    }
  
    /** \brief Output in a short text format
     */
    virtual int text_short(std::vector<std::string> &sv, bool itive_com) {

      ostream *outs=&cout;
      ofstream fout;
      if (sv.size()>1) {
	std::string fname=sv[1];
	fout.open(fname);
	outs=&fout;
      }

      std::string stmp;
      int count=0;
      for(size_t i=0;i<bf.entries.size();i++) {
	bibtex::BibTeXEntry &bt=bf.entries[i];

        if (bf.lower_string(bt.tag)==((string)"article")) {

          // Authors
          if (bf.is_field_present(bt,"author")) {

            std::vector<std::string> firstv, lastv;
            bf.parse_author(bf.get_field(bt,"author"),firstv,lastv,true);
            for(size_t j=0;j<firstv.size();j++) {
              firstv[j]=bf.spec_char_to_uni(firstv[j]);
              lastv[j]=bf.spec_char_to_uni(lastv[j]);
            }

            if (firstv.size()>3) {
              (*outs) << firstv[0] << " " << lastv[0] << ", ";
              (*outs) << firstv[1] << " " << lastv[1] << ", ";
              (*outs) << firstv[2] << " " << lastv[2] << ", et al., ";
            } else if (firstv.size()==3) {
              (*outs) << firstv[0] << " " << lastv[0] << ", ";
              (*outs) << firstv[1] << " " << lastv[1] << ", and ";
              (*outs) << firstv[2] << " " << lastv[2] << ", ";
            } else if (firstv.size()==2) {
              (*outs) << firstv[0] << " " << lastv[0] << " and ";
              (*outs) << firstv[1] << " " << lastv[1] << ", ";
            } else {
              (*outs) << firstv[0] << " " << lastv[0] << ", ";
            }
          }
          
          if (bf.is_field_present(bt,"journal")) {
            (*outs) << bf.spec_char_to_uni(bf.get_field(bt,"journal")) << " ";
          }
          
          if (bf.is_field_present(bt,"volume")) {
            (*outs) << bf.get_field(bt,"volume") << " ";
          }
          
          if (bf.is_field_present(bt,"year")) {
            (*outs) << "(" << bf.get_field(bt,"year") << ") ";
          }
          
          if (bf.is_field_present(bt,"pages")) {
            (*outs) << bf.first_page(bf.get_field(bt,"pages")) << ".";
          }
          
          (*outs) << std::endl;

        } else {
          
        }
        
	count++;
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
  
    /** \brief Get one bibtex entry by it's key
     */
    virtual int get_key(std::vector<std::string> &sv, bool itive_com) {

      if (sv.size()<2) {
	cerr << "Command 'get-key' requires a key to get." << endl;
      }

      std::vector<std::string> list;
      bf.search_keys(sv[1],list);

      if (list.size()==0) {
	cerr << "No keys matching pattern " << sv[1] << " ." << endl;
	return 1;
      }
      for(size_t i=0;i<list.size();i++) {
	bibtex::BibTeXEntry &bt=bf.get_entry_by_key(list[i]);
	bf.bib_output_one(cout,bt);
      }
    
      return 0;
    }

    /** \brief Find all duplicate entries cited in a set of bbl files
     */
    virtual int bbl_dups(std::vector<std::string> &sv, bool itive_com) {

      vector<string> bbl_filelist;
      vector<string> bib_filelist;
      vector<string> bbl_keys;

      // Go through all of the arguments and determine if it
      // ends in .bib or in .bbl
      for(size_t i=1;i<sv.size();i++) {
        size_t len=sv[i].length();
        if (sv[i][len-4]=='.' && sv[i][len-3]=='b' &&
            sv[i][len-2]=='b' && sv[i][len-1]=='l') {
          bbl_filelist.push_back(sv[i]);
          cout << "Found bbl file: " << sv[i] << endl;
        } else if (sv[i][len-4]=='.' && sv[i][len-3]=='b' &&
                   sv[i][len-2]=='i' && sv[i][len-1]=='b') {
          bib_filelist.push_back(sv[i]);
          cout << "Found bib file: " << sv[i] << endl;
        } else {
          // Return early if the file is not a .bib or .bbl file
          cerr << "File not .bbl or .bib in bbl-dups." << endl;
          return 1;
        }
      }

      // Return early if there are no .bib or .bbl files
      if (bbl_filelist.size()==0 || bib_filelist.size()==0) {
        cerr << "Missing .bib or .bbl file in bbl-dups." << endl;
        return 2;
      }

      // Parse the .bbl files to find all keys cited
      for(size_t i=0;i<bbl_filelist.size();i++) {
        
        cout << "Reading .bbl file: " << bbl_filelist[i] << endl;
        ifstream fin;
        fin.open(bbl_filelist[i]);
        
        std::string stemp;
        while (!fin.eof()) {
          
          getline(fin,stemp);
          string key;

          // If the next line does not begin with "\bibitem", then
          // ignore it
          if (stemp.find("\\bibitem")==0) {

            // If there is a square bracket argument, then handle
            // it separately
            if (stemp[8]=='[') {

              // Count of square and curly braces in the square
              // bracet argument, +1 for left and -1 for right
              int count=0;

              // If true, then the square bracket argument has
              // ended and the key has begun
              bool key_started=false;

              // Proceed through the remaining characters one by one
              for(size_t j=8;j<stemp.length();j++) {
                
                if (key_started==false) {
                  
                  if (stemp[j]=='[' || stemp[j]=='{') {
                    count++;
                  } else if (stemp[j]==']' || stemp[j]=='}') {
                    count--;
                  }
                  
                  //cout << "stemp[j]: " << stemp[j] << " count: "
                  //<< count << endl;

                  // If count is zero, then we should find a curly
                  // bracket next and then key_started should be set
                  // to true
                  if (count==0) {
                    if (j<stemp.length()-2) {
                      j++;
                      if (stemp[j]!='{') {
                        cerr << "Count is zero but no left brace in "
                             << "bbl-dups." << endl;
                        return 5;
                      }
                      j++;
                    } else {
                      cerr << "Count is zero but line ended in bbl-dups."
                           << endl;
                      return 4;
                    }
                    key_started=true;
                  }

                  // If the line has ended, it could be because the
                  // square bracket argument covers multiple lines, so
                  // we read the next line and set j to zero to
                  // continue reading.
                  if (count>0 && j==stemp.length()-1) {
                    //cout << "Reading next line." << endl;
                    getline(fin,stemp);
                    j=0;
                  }

                  // End of if (key_started==false) 
                }
                
                if (key_started==true) {
                  // If the key has been started, then start adding
                  // characters to the key.
                  if (stemp[j]=='}') {
                    // If we get a right curly brace, then end the
                    // loop.
                    j=stemp.length();
                  } else {
                    key+=stemp[j];
                  }
                }
              }
              
              // Add the key to the list
              cout << "Found key (with square bracket argument): "
                   << key << endl;
              bbl_keys.push_back(key);
              
              //char ch;
              //cin >> ch;
              
            } else if (stemp[8]=='{') {

              // If there is no square bracket argument, then
              // we can just begin adding characters one by one
              // to the key
              for(size_t j=9;j<stemp.length() && stemp[j]!='}';j++) {
                key+=stemp[j];
              }

              // If no key was found, then stop early
              if (key.length()==0) {
                cerr << "Could not find key in bibitem line." << endl;
                return 8;
              }

              // Add the key to the list
              cout << "Found key (no square bracket argument): " << key << endl;
              bbl_keys.push_back(key);
              
            } else {

              // There was no square bracket argument and no key, so
              // return early.
              cerr << "Line:\n  " << stemp << "\n  does not contain "
                   << "\\bibitem[ or \\bibitem{" << endl;
              return 3;
            }
            
            // End of if (stemp.find("\\bibitem")==0)
          }

          // End of while (!fin.eof())
        }

        // Close the file and move to the next
        fin.close();
        cout << endl;
      }

      // Sort keys and remove duplicate keys
      o2scl::vector_sort<vector<string>,string>(bbl_keys.size(),bbl_keys);
      vector<string> bbl_keys_unique;
      for(size_t i=0;i<bbl_keys.size();i++) {
        if (i==0) {
          bbl_keys_unique.push_back(bbl_keys[0]);
        } else {
          if (bbl_keys[i]!=bbl_keys_unique[i-1]) {
            bbl_keys_unique.push_back(bbl_keys[i]);
          }
        }
      }

      // Read all of the .bib files into an array. We do this so we
      // can tell the user which .bib file each bbl key came from.
      
      vector<bib_file> bf_arr;
      for(size_t i=0;i<bib_filelist.size();i++) {
        cout << "Parsing .bib file: " << bib_filelist[i] << endl;
        bf_arr[i].parse_bib(bib_filelist[i]);
        cout << endl;
      }

      
      
      /*
      for(size_t i=0;i<bbl_keys.size();i++) {
        for(size_t j=0;j<bf.entries.size();j++) {
          bibtex::BibTeXEntry &bt=bf.entries[j];
          if ((*bt.key)==bbl_keys[i]) {
            bf.entries.push_back(bt);
          }
        }
      }

      for(size_t j=0;j<bf.entries.size();j++) {
        bibtex::BibTeXEntry &bt=bf.entries[j];
        std::vector<size_t> list;
        bf.list_possible_duplicates(bt,list);
      }
      */
      
      return 0;
    }
    
    /** \brief Loop over all entries and change keys 
	to a standard key if possible
    */
    virtual int auto_key(std::vector<std::string> &sv, bool itive_com) {
      
      for(size_t i=0;i<bf.entries.size();i++) {
	
	bibtex::BibTeXEntry &bt=bf.entries[i];

	// Ensure title, year, and author are present
	if (bf.is_field_present(bt,"title") &&
	    bf.is_field_present(bt,"year") &&
	    bf.is_field_present(bt,"author")) {

	  // Separate the title into words
	  std::vector<std::string> title_words;
	  o2scl::split_string(bf.get_field(bt,"title"),title_words);
	  
	  if (bf.get_field(bt,"title").length()>5 && title_words.size()>1) {

	    // Start with the last name of the first author
	    std::string auth2=bf.last_name_first_author(bt);

	    // Remove non-alphabetic characters
	    for(size_t j=0;j<auth2.length();j++) {
	      if (!std::isalpha(auth2[j])) {
		std::string tmp=auth2.substr(0,j)+
		  auth2.substr(j+1,auth2.length()-j-1);
		auth2=tmp;
		j=0;
	      }
	    }

	    // Add the year
	    std::string key2=auth2+
	      bf.get_field(bt,"year").substr(2,2);

	    // Add the first characters of the first two
	    // title words which begin with alphabetic characters
	    int nadd=0;
	    for(size_t j=0;j<title_words.size() && nadd<2;j++) {
	      if (std::isalpha(title_words[j][0])) {
		char c=std::tolower(title_words[j][0]);
		key2+=c;
		nadd++;
	      }
	    }

	    // Ensure the new key is not already present
	    if (bf.sort.find(key2)==bf.sort.end()) {
	      
	      std::cout << "Proposing change " << *bt.key << " to " << key2
			<< std::endl;
	      bf.change_key(*bt.key,key2);
	      
	      if (i>0) i--;
	    }
	  }
	  
	}
      }
      return 0;
    }
  
    /** \brief Change the key of an entry
     */
    virtual int change_key(std::vector<std::string> &sv, bool itive_com) {
      
      if (sv.size()<2) { 
	cerr << "Command 'change-key' requires more arguments." << endl;
	return 1;
      }
      
      if (sv.size()==2) {
	if (bf.entries.size()==1) {
	  if (bf.is_key_present(sv[1])==true) {
	    cerr << "Key " << sv[1] << " already present." << endl;
	    return 2;
	  }
	  bf.change_key(*(bf.entries[0].key),sv[1]);
	  return 0;
	} else {
	  cerr << "Command 'change-key' requires more arguments." << endl;
	  return 2;
	}
      }

      std::vector<std::string> list;
      bf.search_keys(sv[1],list);
      
      if (list.size()==0) {
	cerr << "Key matching " << sv[1] << " not found." << endl;
	return 1;
      }
      if (list.size()>1) {
	cerr << "More than one key matches " << sv[1] << endl;
	for(size_t k=0;k<list.size();k++) {
	  cout << k << ". " << list[k] << endl;
	}
	return 2;
      }
      if (bf.is_key_present(sv[2])==true) {
	cerr << "Key " << sv[2] << " already present." << endl;
	return 3;
      }
      bf.change_key(list[0],sv[2]);
    
      return 0;
    }
  
    /** \brief List current keys or those matching a pattern
     */
    virtual int list_keys(std::vector<std::string> &sv, bool itive_com) {
    
      vector<string> klist, kscreen;

      if (sv.size()>=2) {
	bf.search_keys(sv[1],klist);
	for(size_t k=0;k<klist.size();k++) {
	  klist[k]=o2scl::szttos(k)+". "+klist[k];
	}
      } else {
	for(size_t k=0;k<bf.entries.size();k++) {
	  klist.push_back(o2scl::szttos(k)+". "+*(bf.entries[k].key));
	}
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
	cout << "Removing fields: " << endl;
	for(std::vector<std::string>::iterator it=bf.remove_fields.begin();
	    it!=bf.remove_fields.end();it++) {
	  cout << " " << *it << endl;
	}
	return 0;
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
	cout << "Removing fields: " << endl;
	for(std::vector<std::string>::iterator it=bf.remove_fields.begin();
	    it!=bf.remove_fields.end();it++) {
	  cout << " " << *it << endl;
	}
	return 0;
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

    /** \brief Clean the bibliography
     */
    virtual int clean(std::vector<std::string> &sv, bool itive_com) {
      if (sv.size()>1 && sv[1]==((std::string)"fast")) {
	bf.clean(false);
      } else {
	bf.clean();
      }
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
		  << "\">" << bf.spec_char_to_html(bf.short_author(bt))
		  << " (" << bf.get_field(bt,"year") << ")</a><br>" << endl;
	} else if (bf.is_field_present(bt,"doi") &&
		   bf.get_field(bt,"doi").length()>0) {
	  (*outs) << "<a href=\"https://doi.org/" << bf.get_field(bt,"url")
		  << "\">" << bf.spec_char_to_html(bf.short_author(bt))
		  << " (" << bf.get_field(bt,"year") << ")</a><br>" << endl;
	} else {
	  (*outs) << bf.spec_char_to_html(bf.short_author(bt)) << " ("
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

      std::string prefix="";
      if (sv.size()>2) {
	prefix=sv[2];
      }

      for(size_t i=0;i<bf.entries.size();i++) {
	bibtex::BibTeXEntry &bt=bf.entries[i];
      
	if (bt.key) {
	  (*outs) << "    \\anchor " << prefix << *bt.key << " " << *bt.key
		  << ":" << endl;
	}

	if (bf.lower_string(bt.tag)==((string)"article")) {

	  if (bf.is_field_present(bt,"author")) {
	    if (bf.is_field_present(bt,"url")) {
	      (*outs) << "    <a href=\""
		      << bf.get_field(bt,"url") << "\">" << endl;
	      (*outs) << "    "
		      << bf.author_firstlast(bf.get_field(bt,"author"))
		      << "</a>," << endl;
	    } else if (bf.is_field_present(bt,"doi")) {
	      (*outs) << "    <a href=\"https://doi.org/"
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

	} else if (bf.lower_string(bt.tag)==((string)"inbook")) {

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
	      (*outs) << "    <a href=\"https://www.worldcat.org/isbn/"
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
	      (*outs) << "    <a href=\"https://www.worldcat.org/isbn/"
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

	} else if (bf.lower_string(bt.tag)==((string)"book")) {

	  if (bf.is_field_present(bt,"author")) {
	    (*outs) << bf.author_firstlast(bf.get_field(bt,"author"))
		    << "," << endl;
	  }
	  if (bf.is_field_present(bt,"url")) {
	    (*outs) << "<a href=\""
		    << bf.get_field(bt,"url") << "\">" << endl;
	    (*outs) << "    "
		    << bf.get_field(bt,"title")
		    << "</a>," << endl;
	  } else if (bf.is_field_present(bt,"isbn")) {
	    (*outs) << "<a href=\"https://www.worldcat.org/isbn/"
		    << bf.get_field(bt,"isbn") << "\">" << endl;
	    (*outs) << "    "
		    << bf.get_field(bt,"title")
		    << "</a>," << endl;
	  } else {
	    (*outs) << bf.get_field(bt,"title") << "," << endl;
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
  
    /** \brief Output the BibTeX data in a longer HTML format
     */
    virtual int html(std::vector<std::string> &sv, bool itive_com) {

      bool list=false;
      
      ostream *outs=&cout;
      ofstream fout;
      
      if (sv.size()>1) {
	if (sv[1]==((string)"list")) {
	  list=true;
	  if (sv.size()>2) {
	    std::string fname=sv[2];
	    fout.open(fname);
	    outs=&fout;
	  }
	} else {
	  std::string fname=sv[1];
	  fout.open(fname);
	  outs=&fout;
	}
      }

      for(size_t i=0;i<bf.entries.size();i++) {
	bibtex::BibTeXEntry &bt=bf.entries[i];

	if (list) {
	  (*outs) << "<li>" << endl;
	}

	if (bf.lower_string(bt.tag)==((string)"article")) {
	  
	  if (bf.is_field_present(bt,"title")) {
	    std::string	title_temp=bf.get_field(bt,"title");
	    // Remove quotes, braces, and spaces if necessary
	    if (title_temp[0]=='\"' &&
		title_temp[title_temp.size()-1]=='\"') {
	      title_temp=title_temp.substr(1,title_temp.size()-2);
	    }
	    if (title_temp[0]=='{' &&
		title_temp[title_temp.size()-1]=='}') {
	      title_temp=title_temp.substr(1,title_temp.size()-2);
	    }
	    if (title_temp[0]==' ') {
	      title_temp=title_temp.substr(1,title_temp.size()-1);
	    }
	    if (title_temp[title_temp.size()-1]==' ') {
	      title_temp=title_temp.substr(0,title_temp.size()-1);
	    }
	    
	    if (bf.is_field_present(bt,"url")) {
	      (*outs) << "\"<a href=\""
		      << bf.get_field(bt,"url") << "\">";
	      (*outs) << title_temp << "</a>\", ";
	    } else if (bf.is_field_present(bt,"doi")) {
	      (*outs) << "\"<a href=\"https://doi.org/"
		      << bf.get_field(bt,"doi") << "\">";
	      (*outs) << title_temp << "</a>\", ";
	    } else {
	      (*outs) << "\"" << title_temp << "\", ";
	    }
	  }
	  (*outs) << bf.author_firstlast(bf.get_field(bt,"author"))
		  << ", ";
	  if (bf.is_field_present(bt,"journal")) {
	    (*outs) << bf.get_field(bt,"journal") << " ";
	  }
	  if (bf.is_field_present(bt,"volume")) {
	    (*outs) << "<b>" << bf.get_field(bt,"volume") << "</b> ";
	  }
	  if (bf.is_field_present(bt,"year")) {
	    (*outs) << "(" << bf.get_field(bt,"year") << ") ";
	  }
	  if (bf.is_field_present(bt,"pages")) {
	    (*outs) << bf.first_page(bf.get_field(bt,"pages"));
	  }
	  if (bf.is_field_present(bt,"eprint")) {
	    string eprint_temp=bf.get_field(bt,"eprint");
	    if (eprint_temp[0]==' ') {
	      eprint_temp=eprint_temp.substr(1,eprint_temp.size()-1);
	    }
	    if (eprint_temp[eprint_temp.size()-1]==' ') {
	      eprint_temp=eprint_temp.substr(0,eprint_temp.size()-1);
	    }
	    (*outs) << " [<a href=\"https://arxiv.org/abs/"
		    << eprint_temp << "\">";
	    (*outs) << eprint_temp << "</a>]." << endl;
	  } else {
	    (*outs) << "." << endl;
	  }

	} else if (bf.lower_string(bt.tag)==((string)"inbook")) {

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
	      (*outs) << "    <a href=\"https://www.worldcat.org/isbn/"
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
	    std::string title_temp;
	    if (bf.is_field_present(bt,"title")) {
	      title_temp=bf.get_field(bt,"title");
	    }
	    // Remove quotes, braces, and spaces if necessary
	    if (title_temp[0]=='\"' &&
		title_temp[title_temp.size()-1]=='\"') {
	      title_temp=title_temp.substr(1,title_temp.size()-2);
	    }
	    if (title_temp[0]=='{' &&
		title_temp[title_temp.size()-1]=='}') {
	      title_temp=title_temp.substr(1,title_temp.size()-2);
	    }
	    if (title_temp[0]==' ') {
	      title_temp=title_temp.substr(1,title_temp.size()-1);
	    }
	    if (title_temp[title_temp.size()-1]==' ') {
	      title_temp=title_temp.substr(0,title_temp.size()-1);
	    }
	    
	    if (bf.is_field_present(bt,"url")) {
	      (*outs) << "    <a href=\""
		      << bf.get_field(bt,"url") << "\">" << endl;
	      (*outs) << "    "
		      << title_temp
		      << "</a>," << endl;
	    } else if (bf.is_field_present(bt,"isbn")) {
	      (*outs) << "    <a href=\"https://www.worldcat.org/isbn/"
		      << bf.get_field(bt,"isbn") << "\">" << endl;
	      (*outs) << "    "
		      << title_temp
		      << "</a>," << endl;
	    } else {
	      (*outs) << "    " << title_temp << "," << endl;
	    }
	    (*outs) << "    (" << bf.get_field(bt,"year") << ") "
		    << bf.get_field(bt,"publisher") << ", p. "
		    << bf.get_field(bt,"pages") << "." << endl;
	    (*outs) << endl;
	  }

	} else if (bf.lower_string(bt.tag)==((string)"book")) {

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
	    (*outs) << "    <a href=\"https://www.worldcat.org/isbn/"
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

	if (list) {
	  (*outs) << "</li>" << endl;
	}
	
      }

      if (sv.size()>1) {
	fout.close();
      }

      return 0;
    }

    /** \brief Desc
     */
    virtual int inspire_cites(std::vector<std::string> &sv,
			      bool itive_com) {

      for(size_t i=0;i<bf.entries.size();i++) {
	
	bibtex::BibTeXEntry &bt=bf.entries[i];
	
	if (bf.is_field_present(bt,"inspireid")) {
	  
	  string id=bf.get_field(bt,"inspireid");
	  cout << "Found inspireid " << id << " in "
	       << *bt.key << endl;
	  
	  string cmd=((string)"curl -X GET \"http://old.inspirehep.net/")+
	    "search?action_search=Search&rg=100&of=recjson&"+
	    "ln=en&p=find+recid+"+id+"&jrec=0&ot=number_of_citations\"";
	  string result;
	  int ret=pipe_cmd_string(cmd,result,false);
	  
	  // The result string is of the form:
	  // [{"number_of_citations": 6}]
	  // so we reformat
	  if (result.size()<27) {
	    cout << "Result failed for key "
		 << *bt.key << " in btmanip::inspire_cites()." << endl;
	  } else {
	    result=result.substr(25,result.length()-27);
	    
	    if (bf.is_field_present(bt,"citations")) {
	      cout << "Current value of citations field for " << *bt.key
		   << " is: "
		   << bf.get_field(bt,"citations") << endl;
	    }
	    cout << "Setting citations field of " << *bt.key
		 << " to " << result << endl;
	    bf.set_field_value(bt,"citations",
			       o2scl::itos(o2scl::stoi(result)));
	  }
	  
	  cout << "Sleeping for 1 minute." << endl;
	  sleep(60);
	}
      }
      
      return 0;
    }
    
    /** \brief Desc
     */
    virtual int ads_cites(std::vector<std::string> &sv,
			  bool itive_com) {

      // Get API token
      std::string token;
      char *token_ptr=getenv("ADSABS_TOKEN");
      if (token_ptr) {
	token=token_ptr;
      } else {
	cerr << "Token not found in \"ads-cites\"." << endl;
	return 1;
      }

      std::string prefix="curl -X GET -H 'Authorization: Bearer:"+token+
	"' '";
      std::string base_url="https://api.adsabs.harvard.edu/v1/search";
      
      for(size_t i=0;i<bf.entries.size();i++) {
	
	bibtex::BibTeXEntry &bt=bf.entries[i];
	
	if (bf.is_field_present(bt,"bibcode")) {
	  
	  string bibcode=bf.get_field(bt,"bibcode");
	  cout << "Found bibcode " << bibcode << " in "
	       << *bt.key << endl;

	  boost::replace_all(bibcode,"&","%26");
	  
	  string cmd=prefix+base_url+"/query?q="+bibcode+
	    "&fl=citation_count'";
	  cout << "cmd: " << cmd << endl;
	  string result;
	  int ret=pipe_cmd_string(cmd,result,false,400);
	  cout << result.size() << " " << result << endl;
	  
	  // The result string is of the form:
	  //
	  // {"responseHeader":{"status":0,"QTime":193,"params":
	  // {"q":"2019PhRvD..99d3010C","x-amzn-trace-id":
	  // "Root=1-5ce59b6e-195040826fa8b1c4925506dc","fl":
	  // "citation_count","start":"0","rows":"10","wt":
	  // "json"}},"response":{"numFound":1,"start":
	  // 0,"docs":[{"citation_count":6}]}}
	  //
	  // so we reformat:
	  size_t loc=result.find("\"citation_count\":");
	  if (loc==std::string::npos) {
	    cerr << "Failed to find correct field." << endl;
	    return 2;
	  }
	  result=result.substr(loc+17,result.length()-loc-17);
	  cout << result.size() << " " << result << endl;
	  
	  if (bf.is_field_present(bt,"adscites")) {
	    cout << "Current value of adscites field for " << *bt.key
		 << " is: "
		 << bf.get_field(bt,"adscites") << endl;
	  }
	  cout << "Setting adscites field of " << *bt.key
	       << " to " << result << endl;
	  bf.set_field_value(bt,"adscites",
			     o2scl::itos(o2scl::stoi(result)));
	  
	  cout << "Sleeping for 1 minute." << endl;
	  sleep(60);
	}
      }
      
      return 0;
    }

    virtual int cites_per_month(std::vector<std::string> &sv, bool itive_com) {

      // Get current time using C functions
      std::time_t t = std::time(0); 
      std::tm* now = std::localtime(&t);
      int curr_year=now->tm_year+1900;
      int curr_month=now->tm_mon+1;

      /*
	std::cout << (now->tm_year + 1900) << '-' 
	<< (now->tm_mon + 1) << '-'
	<<  now->tm_mday
	<< std::endl;
      */

      for(size_t i=0;i<bf.entries.size();i++) {
	bibtex::BibTeXEntry &bt=bf.entries[i];
	if (bf.is_field_present(bt,"year") &&
	    bf.is_field_present(bt,"month") &&
	    bf.is_field_present(bt,"citations")) {
	  int pub_year=std::stoi(bf.get_field(bt,"year"));
	  int delta_year=curr_year-pub_year;
	  string month_str=bf.get_field(bt,"month");
	  int pub_month=0;
	  std::transform(month_str.begin(),month_str.end(),
			 month_str.begin(),
			 [](unsigned char c){ return std::tolower(c); });
	  if (month_str.substr(0,3)==((string)"jan")) {
	    pub_month=1;
	  } else if (month_str.substr(0,3)==((string)"feb")) {
	    pub_month=2;
	  } else if (month_str.substr(0,3)==((string)"mar")) {
	    pub_month=3;
	  } else if (month_str.substr(0,3)==((string)"apr")) {
	    pub_month=4;
	  } else if (month_str.substr(0,3)==((string)"may")) {
	    pub_month=5;
	  } else if (month_str.substr(0,3)==((string)"jun")) {
	    pub_month=6;
	  } else if (month_str.substr(0,3)==((string)"jul")) {
	    pub_month=7;
	  } else if (month_str.substr(0,3)==((string)"aug")) {
	    pub_month=8;
	  } else if (month_str.substr(0,3)==((string)"sep")) {
	    pub_month=9;
	  } else if (month_str.substr(0,3)==((string)"oct")) {
	    pub_month=10;
	  } else if (month_str.substr(0,3)==((string)"nov")) {
	    pub_month=11;
	  } else if (month_str.substr(0,3)==((string)"dec")) {
	    pub_month=12;
	  }
	  if (pub_month!=0) {
	    int delta_month=curr_month-pub_month;
	    if (delta_month<0) {
	      delta_year--;
	      delta_month+=12;
	    }
	    int citations=std::stoi(bf.get_field(bt,"citations"));
	    cout.width(20);
	    cout << *bt.key << " ";
	    cout.width(3);
	    cout << delta_year*12+delta_month << " ";
	    cout.width(3);
	    cout << citations << " ";
	    cout << ((double)citations)/
	      ((double)(delta_year*12+delta_month)) << endl;
	  } else {
	    cout << *bt.key << " results in bad month calculation."
		 << endl;
	  }
	} else {
	  cout << *bt.key << " is missing a proper field." << endl;
	}
      }
      
      return 0;
    }  
    
    /** \brief Output the BibTeX data as a file in
	reStructured Text format for Sphinx
    */
    virtual int rst(std::vector<std::string> &sv, bool itive_com) {

      ostream *outs=&cout;
      ofstream fout;
      if (sv.size()>1) {
	std::string fname=sv[1];
	fout.open(fname);
	outs=&fout;
      }

      std::string prefix="";
      if (sv.size()>2) {
	prefix=sv[2];
      }

      // In this function, we remove extra whitespace for titles which
      // have more than one line because ReST is picky about spacing
      
      for(size_t i=0;i<bf.entries.size();i++) {
	bibtex::BibTeXEntry &bt=bf.entries[i];
      
	if (bt.key) {
	  (*outs) << ".. [" << *bt.key << "] : ";
	}

	if (bf.lower_string(bt.tag)==((string)"article")) {
	  // Create rst output for an article

	  if (bf.is_field_present(bt,"author")) {
	    string auth=bf.author_firstlast(bf.get_field(bt,"author"),
					    true,true);
	    auth=bf.spec_char_to_uni(auth);
	    if (bf.is_field_present(bt,"url")) {
	      (*outs) << "`" << auth << endl;
	      (*outs) << "   <" << bf.get_field(bt,"url") 
		      << ">`_," << endl;
	    } else if (bf.is_field_present(bt,"doi")) {
	      (*outs) << "`" << auth << endl;
	      (*outs) << "   <https://doi.org/" << bf.get_field(bt,"doi") 
		      << ">`_," << endl;
	    } else {
	      (*outs) << auth << "," << endl;
	    }
	  }
	  if (bf.is_field_present(bt,"journal")) {
	    (*outs) << "   "
		    << bf.spec_char_to_uni(bf.get_field(bt,"journal"))
		    << " **";
	  }
	  if (bf.is_field_present(bt,"volume")) {
	    (*outs) << bf.get_field(bt,"volume") << "** ";
	  }
	  if (bf.is_field_present(bt,"year")) {
	    (*outs) << "(" << bf.get_field(bt,"year") << ") ";
	  }
	  if (bf.is_field_present(bt,"pages")) {
	    (*outs) << bf.first_page(bf.get_field(bt,"pages")) << "." << endl;
	  } else {
	    (*outs) << "." << endl;
	  }
	  (*outs) << endl;

	} else if (bf.lower_string(bt.tag)==((string)"inbook")) {
	  // Create rst output for an inbook entry
	  
	  if (bf.is_field_present(bt,"crossref") &&
	      bf.get_field(bt,"crossref").length()>0) {
	    // Create rst output for an inbook entry with a crossref
	  
	    bibtex::BibTeXEntry &bt2=
	      bf.get_entry_by_key(bf.get_field(bt,"crossref"));

	    // Remove extra whitespace for titles which have more than
	    // one line because ReST is picky about spacing
	    string title2_temp=bf.get_field(bt2,"title");
	    bf.thin_whitespace(title2_temp);

	    string title_temp=bf.get_field(bt,"title");
	    bf.thin_whitespace(title_temp);
	    
	    if (bf.is_field_present(bt,"author")) {
	      string auth=bf.author_firstlast(bf.get_field(bt,"author"),
					      true,true);
	      auth=bf.spec_char_to_uni(auth);
	      (*outs) << auth << ", "
		      << bf.spec_char_to_uni(title_temp)
		      << endl;
	    } else {
	      (*outs) << endl;
	    }
	    
	    if (bf.is_field_present(bt2,"url")) {
	      (*outs) << "   in `"
		      << bf.spec_char_to_uni(title2_temp) << " <"
		      << bf.get_field(bt2,"url") << ">`_," << endl;
	    } else if (bf.is_field_present(bt2,"isbn")) {
	      (*outs) << "   `"
		      << bf.spec_char_to_uni(title2_temp)
		      << " <https://www.worldcat.org/isbn/" 
		      << bf.get_field(bt2,"isbn") << ">`_," << endl;
	    } else {
	      (*outs) << "   "
		      << bf.spec_char_to_uni(title2_temp)
		      << "," << endl;
	    }
	    (*outs) << "   (" << bf.get_field(bt2,"year") << ") "
		    << bf.spec_char_to_uni(bf.get_field(bt2,"publisher"))
		    << ", p. " << bf.get_field(bt,"pages") << "." << endl;
	    (*outs) << endl;
	    
	  } else {
	    // Create rst output for an inbook entry without a crossref
	    
	    if (bf.is_field_present(bt,"author")) {
	      string auth=bf.author_firstlast(bf.get_field(bt,"author"),
					      true,true);
	      auth=bf.spec_char_to_uni(auth);
	      (*outs) << auth << "," << endl;
	    } else {
	      (*outs) << endl;
	    }
	    
	    string title_temp=bf.get_field(bt,"title");
	    bf.thin_whitespace(title_temp);
	    
	    if (bf.is_field_present(bt,"url")) {
	      (*outs) << "   `" << bf.spec_char_to_uni(title_temp)
		      << " <" << bf.get_field(bt,"url") << ">`_," << endl;
	    } else if (bf.is_field_present(bt,"isbn")) {
	      (*outs) << "   `" << bf.spec_char_to_uni(title_temp)
		      << " <https://www.worldcat.org/isbn/"
		      << bf.get_field(bt,"isbn") << ">`_," << endl;
	    } else {
	      (*outs) << "   " << bf.spec_char_to_uni(title_temp)
		      << "," << endl;
	    }
	    (*outs) << "   (" << bf.get_field(bt,"year") << ") "
		    << bf.spec_char_to_uni(bf.get_field(bt,"publisher"))
		    << ", p. "
		    << bf.get_field(bt,"pages") << "." << endl;
	    (*outs) << endl;
	  }

	} else if (bf.lower_string(bt.tag)==((string)"book")) {
	  // Create rst output for a book entry

	  if (bf.is_field_present(bt,"author")) {
	    string auth=bf.author_firstlast(bf.get_field(bt,"author"),
					    true,true);
	    auth=bf.spec_char_to_uni(auth);
	    (*outs) << auth << "," << endl;
	  } else {
	    (*outs) << endl;
	  }
	  
	  // Remove extra whitespace for titles which have more than
	  // one line because ReST is picky about spacing
	  string title_temp=bf.get_field(bt,"title");
	  bf.thin_whitespace(title_temp);
	  
	  if (bf.is_field_present(bt,"url")) {
	    (*outs) << "   `" << bf.spec_char_to_uni(title_temp);
	    (*outs) << " <" << bf.get_field(bt,"url") << ">`_," << endl;
	  } else if (bf.is_field_present(bt,"isbn")) {
	    (*outs) << "   `" << bf.spec_char_to_uni(title_temp)
		    << " <https://www.worldcat.org/isbn/"
		    << bf.get_field(bt,"isbn") << ">`_," << endl;
	  } else {
	    (*outs) << "   " << bf.spec_char_to_uni(title_temp)
		    << "," << endl;
	  }
	  (*outs) << "   (" << bf.get_field(bt,"year") << ") "
		  << bf.spec_char_to_uni(bf.get_field(bt,"publisher"));
	  if (bf.is_field_present(bt,"note") &&
	      bf.get_field(bt,"note").length()>0) {

	    string note_temp=bf.get_field(bt,"note");
	    bf.thin_whitespace(note_temp);
	    
	    (*outs) << "\n   ("
		    << bf.spec_char_to_uni(note_temp)
		    << ")";
	  }
	  (*outs) << ".\n" << endl;

	} else if (bf.lower_string(bt.tag)==((string)"mastersthesis")) {
	  // Create rst output for a mastersthesis entry
	  
	  if (bf.is_field_present(bt,"author")) {
	    string auth=bf.author_firstlast(bf.get_field(bt,"author"),
					    true,true);
	    auth=bf.spec_char_to_uni(auth);
	    if (bf.is_field_present(bt,"url")) {
	      (*outs) << "`" << auth << endl;
	      (*outs) << "    <" << bf.get_field(bt,"url")
		      << ">`_," << endl;
	    } else if (bf.is_field_present(bt,"doi")) {
	      (*outs) << "`" << auth << endl;
	      (*outs) << "    <https://doi.org/" << bf.get_field(bt,"doi")
		      << ">`_," << endl;
	    }
	  } 
	  (*outs) << "    Thesis: " << bf.get_field(bt,"title") << endl;
	  (*outs) << "    (" << bf.get_field(bt,"year") << ")";
	  (*outs) << ".\n" << endl;

	} else if (bf.lower_string(bt.tag)==((string)"misc")) {
	  // Create rst output for a misc entry
	  
	  if (bf.is_field_present(bt,"author")) {
	    string auth=bf.author_firstlast(bf.get_field(bt,"author"),
					    true,true);
	    auth=bf.spec_char_to_uni(auth);
	    if (bf.is_field_present(bt,"url")) {
	      (*outs) << "`" << auth << endl;
	      (*outs) << "    <" << bf.get_field(bt,"url")
		      << ">`_," << endl;
	    } else if (bf.is_field_present(bt,"doi")) {
	      (*outs) << "`" << auth << endl;
	      (*outs) << "    <https://doi.org/" << bf.get_field(bt,"doi")
		      << ">`_," << endl;
	    }
	  } 
	  (*outs) << "   " << bf.get_field(bt,"title");
	  (*outs) << "(" << bf.get_field(bt,"year") << ")";
	  (*outs) << ".\n" << endl;
	}
      }

      if (sv.size()>1) {
	fout.close();
      }

      return 0;
    }
  
  public:

    /** \brief Create a ``btmanip_class`` object
     */
    btmanip_class() {
      jlist_read=false;
      std::string histfile;
      char *hd=getenv("HOME");
      if (hd) {
	histfile=hd;
	histfile+="/.btmanip_hist";
      }
      cl=new cli_readline(histfile);
    }
    
    /** \brief Destroy a ``btmanip_class`` object
     */
    ~btmanip_class() {
      delete cl;
    }

    /** \brief Main class interface
     */
    virtual int run(int argc, char *argv[]) {
    
      static const int nopt=44;
      comm_option_s options[nopt]={
	{'a',"add","Add a specified .bib file.",1,1,"<file>",
	 ((std::string)"This command adds the entries in <file> to ")+
	 "the current list of entries, prompting the user how to "+
	 "handle possible duplicate entries.",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::add),
	 cli::comm_option_both},
        {0,"bbl-dups","Look for duplicates among all .bbl entries.",-1,-1,
         "","",new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::bbl_dups),cli::comm_option_both},
	{0,"auto-key","Automatically set keys for all entries.",0,0,"",
	 ((std::string)"This command automatically sets the key ")+
	 "for all entries equal to the Last name of the first "+
	 "author, a two-digit year, and the first two letters of "+
	 "the title.",new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::auto_key),cli::comm_option_both},
	{'b',"bib","Output a .bib file.",0,1,"[file]",
	 ((std::string)"Output all of the current entries in .bib ")+
	 "format to the screen, or if a file is specified, to the file.",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::bib),
	 cli::comm_option_both},
	{0,"change-key","Change an entry's key.",2,2,
	 "<key pattern> <new key>",
	 ((std::string)"This command searches the current list of ")+
	 "BibTeX entries for keys matching <key pattern>. If only "+
	 "one key matches the pattern, then this key is renamed to "+
	 "<new key>. Otherwise, an error message is output and no "+
	 "keys are renamed.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::change_key),cli::comm_option_both},
	{0,"ck","Change an entry's key (alias of change-key).",2,2,
	 "<key before> <key after>",
	 "This command is an alias for 'change-key'.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::change_key),cli::comm_option_both},
	{0,"clean","Clean up the bibliography.",0,1,"[\"fast\"]",
	 ((std::string)"This command cleans up the bibliography ")+
	 "in several ways. First, if 'normalize_tags' is true, "+
	 "it ensures all of the tags have the standard capitalization "+
	 "It converts all fields to "+
	 "lowercase if 'lowercase_fields' is true. It removes extra "+
	 "braces ({}) from every field. It removes any unwanted "+
	 "fields (see 'remove-field'). It removes extra whitespace "+
	 "from all all fields. If a journal name list has been loaded "+
	 "it renames the journal to the standard abbreviation. If "+
	 "'remove_vol_letters' is true, it moves some letters in "+
	 "volume numbers to the end of the journal name. If "+
	 "'autoformat_urls' is true, it automatically creates a URL "+
	 "from the DOI or ISBN. If 'add_empty_titles' is true, it adds "+
	 "empty titles to articles or proceedings which don't have a "+
	 "title. Finally, if 'check_required' is true, it checks to make "+
	 "sure that all required fields are included.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::clean),cli::comm_option_both},
	{'c',"cv","Create output for a LaTeX CV.",0,1,"[file]",
	 ((std::string)"Output articles in a list format for LaTeX. They ")+
	 "are formatted using: \n \n"+
	 "\\noindent 1. ``Title'', \\\\\n"+
	 "Author list, \\\\\n"+
	 "\\href{URL}\n"+
	 "{{\\it Journal} {\\bf Volume} (Year) "+
	 "First page.} \\\\\n"+
	 "(\\href{Arxiv URL}{arXiv:number} - citations)\\\\\n \n"+
	 "The title is wrapped to 80 columns. The "+
	 "author list is converted to initials and "+
	 "last names and then wrapped to 80 columns. "+
	 "The URL is constructed from "+
	 "the DOI entry, and the citations is taken from a custom field "+
	 "named 'citations'. A footnote is added to the first "+
	 "article with at least one citation.",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::cv),
	 cli::comm_option_both},
	{0,"cvt","Create talks output for a LaTeX CV.",0,1,"[file]",
	 ((std::string)"Output talks in a list format for LaTeX. They ")+
	 "are formatted using: \n \n"+
	 "\\noindent 1. ``Title'', \\\\\n"+
	 "{\\bf \\href{URL}{Conference}}, \\\\\n"+
	 "Institution, City, State, Country, Month Year.\n \n"+
	 "The title is wrapped to 80 columns. If the field 'title_latex' "+
	 "is present it is used for the title. A three-letter "+
	 "abbreviation is used for the month.",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::cv_talks),
	 cli::comm_option_both},
	{0,"dox","Output a doxygen file.",0,2,"[file]",
	 ((std::string)"This outputs articles in the following format: ")+
	 "\n \n\\anchor [prefix]key key:\n"+
	 "<a href=\"URL\">\n"+
	 "Author List</a>,\n"+
	 "journal \\\\b volume (year) pages.\n \n"+
	 "and books in the following format:\n \n"+
	 "\\anchor [prefix]key key:\n"+
	 "Author List,\n"+
	 "<a href=\"URL\">\n"+
	 "title</a>,\n"+
	 "(year) publisher, p. page.",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::dox),
	 cli::comm_option_both},
	{0,"html","Output HTML.",0,2,"[\"list\"] [file]",
	 ((std::string)"This outputs articles in the following format: ")+
	 "\"<a href=\"URL\">Title</a>\", Author List, "+
	 "journal <b>volume</b> (year) pages "+
	 "<a href=\"arXiv URL\">arXiv number</a>.\n \n"+
	 "and books in the following format:\n \n"+
	 "\\anchor [prefix]key key:\n"+
	 "Author List,\n"+
	 "<a href=\"URL\">\n"+
	 "title</a>,\n"+
	 "(year) publisher, p. page.\n \nIf the first argument to 'html' "+
	 "is \"list\", then all entries are preceeded by \"<li>\" and "+
	 "succeeded by \"</li>\".",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::html),
	 cli::comm_option_both},
	{'d',"dup","Find duplicates between .bib files.",0,2,
	 "[file 1] [file 2]",
	 ((std::string)"If no filenames are specified, then look for ")+
	 "duplicates among the current entries. If one filename is "+
	 "specified, then compare the current entries to the specified "+
	 "file to find duplicate entries. Finally, if two filenames are "+
	 "specified, find duplicates between the two files.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::dup),cli::comm_option_both},
	{'g',"get-key","Get entry by key.",1,1,"<key pattern>",
	 ((std::string)"Get a single BibTeX entry from the current ")+
	 "list of entries by matching keys to the specified pattern "+
	 "and output that key to the screen in .bib format. If no "+
	 "keys match then an error "+
	 "message is output to the screen.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::get_key),cli::comm_option_both},
	{0,"gk","Get entry by key (alias for get-key).",1,1,"<key pattern>",
	 "This command is an alias for 'get-key'.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::get_key),cli::comm_option_both},
	{0,"hay","Create a simple HTML author-year list.",0,1,
	 "[file]","",new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::hay),cli::comm_option_both},
	{0,"hdf5","Output an HDF5 file.",1,1,
	 "<file>","",new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::hdf5),cli::comm_option_both},
	{0,"journal","Look up journal name.",1,1,"<name>",
	 ((std::string)"If a journal name list is loaded, look up ")+
	 "<name> in the list and output all the synonyms. Journal "+
	 "name matching ignores spacing, case, and punctuation.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::journal),cli::comm_option_both},
	{'k',"keep-field","Remove a field from the remove list.",
	 0,1,"[field]",
	 ((std::string)"Remove <field> from the list of unwanted fields ")+
	 "to remove with the 'clean' command. If [field] is unspecified, "+
	 "then just output the list of unwanted fields to remove.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::keep_field),cli::comm_option_both},
	{'l',"list-keys","List entry keys.",0,1,"[pattern]",
	 ((std::string)"List all entry keys from the ")+
	 "current bibliography, or if a pattern is specified, list "+
	 "only the keys in the current list which patch that pattern.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::list_keys),cli::comm_option_both},
	{0,"lk","List entry keys (alias of list-keys).",0,1,"[pattern]",
	 "This command is an alias of 'list-keys'.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::list_keys),cli::comm_option_both},
	{'n',"nsf","Output LaTeX source for an NSF bio sketch.",
	 0,1,"[file]","",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::nsf),
	 cli::comm_option_both},
	{'p',"parse","Parse a specified .bib file.",1,1,"<file>",
	 ((std::string)"This function parses a .bib file and ")+
	 "loads it into the current BibTeX entry list. It does not "+
	 "do any reformatting or checking for duplicate entries.",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::parse),
	 cli::comm_option_both},
	{0,"parse-hdf5","Parse a bibliography stored in an HDF5 file.",
	 1,1,"<file>","",new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::parse_hdf5),cli::comm_option_both},
	{0,"doe-talks","Output of talks for DOE progress report.",
	 0,1,"[file]","",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::doe_talks),
	 cli::comm_option_both},
	{0,"prop","Output a proposal .bib file.",0,1,"[file]","",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::proposal),
	 cli::comm_option_both},
	{'j',"read-jlist","Read a new journal list.",1,1,"<file>",
	 ((std::string)"Read a list of journal names and synonyms ")+
	 "from <file>.",new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::read_jlist),cli::comm_option_both},
	{0,"remove",
	 "Remove fields which match a set of field and pattern pairs.",2,-1,
	 ((std::string)"[\"and\"] [\"or\"] <field 1> ")+
	 "<pattern 1> [field 2] [pattern 2] ...",
	 ((std::string)"Remove entries from the current list of ")+
	 "which have fields which match a specified pattern. Combine "+
	 "multiple criteria with \"or\" or \"and\" if specified.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::remove),cli::comm_option_both},
	{'r',"remove-field","Add a field to the remove list.",0,1,"<field>",
	 ((std::string)"Add <field> to the list of unwanted fields ")+
	 "to remove with the 'clean' command. If [field] is unspecified, "+
	 "then just output the list of unwanted fields to remove.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::remove_field),cli::comm_option_both},
	{0,"reverse","Reverse the order of the current bibliography.",
	 0,0,"","",new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::reverse),cli::comm_option_both},
	{0,"rst","Output a rst file.",0,2,"[file]","",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::rst),
	 cli::comm_option_both},
	{0,"cpm","Compute citations per month.",0,0,"","",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::cites_per_month),
	 cli::comm_option_both},
	{0,"o2scl","Parse the o2scl bib files",0,0,"","",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::o2scl),
	 cli::comm_option_both},
	{0,"inspire-cites","Calculate Inspire citations.",0,0,"","",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::inspire_cites),cli::comm_option_both},
	{0,"inspire-get","",0,0,"","",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::inspire_get),cli::comm_option_both},
	{0,"ads-cites","Calculate ADSABS citations.",0,0,"","",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::ads_cites),cli::comm_option_both},
	{0,"ads-get","",0,0,"","",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::ads_get),cli::comm_option_both},
	{'s',"search","Search current list for field and pattern pairs.",
	 2,-1,((std::string)"[\"and\"] [\"or\"] <field 1> ")+
	 "<pattern 1> [field 2] [pattern 2] ...",
	 ((std::string)"Search the current list for entries ")+
	 "which have fields which match a specified pattern. Combine "+
	 "multiple criteria with \"or\" or \"and\" if specified. If "+
	 "at least one entry is found, then the current list is "+
	 "replaced with the search results. Replace one of the "+
	 "field arguments with \"key\" to search by key name.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::search),cli::comm_option_both},
	{'f',"set-field",
	 "For entry <key> and field <field>, set its value to <value>.",2,3,
	 "<entry> <field> <value> or if one entry, <field> <value>",
	 ((std::string)"Set the value of <field> in <entry> to ")+
	 "<value>, or if there is only one entry in the current list "+
	 "and only two arguments are given, set <field> to <value>.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::set_field),cli::comm_option_both},
	{0,"sort","Sort current BibTeX entries by key.",0,0,
	 "","Sort current BibTeX entries by key.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::sort),cli::comm_option_both},
	{0,"sort-by-date","Sort current BibTeX entries by date.",0,1,
	 "[\"descending\"]","Sort current BibTeX entries by date.",
	 new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::sort_by_date),cli::comm_option_both},
	{'u',"sub","Subtract a .bib file from the current entries.",
	 1,1,"<file>",((std::string)"This takes all entries in ")+
	 "<file> and looks for them in the list of current entries. "+
	 "If any duplicates are found, they are removed from the "+
	 "current list.",new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::sub),cli::comm_option_both},
	{0,"text-full","Output full data as a text file.",0,1,"[file]",
         ((string)"This outputs the bibliography in a simple text-based ")+
         "format. Typical output is\n\ntag: <tag>\nkey: <key>\n"+
         "field 1: <value 1>\n"+
         "field 2: <value 2>\n...\n",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::text_full),
	 cli::comm_option_both},
	{0,"text-short","Output as text in a short format",0,1,"[file]","",
	 new comm_option_mfptr<btmanip_class>(this,&btmanip_class::text_short),
	 cli::comm_option_both},
	{0,"utk-rev","UTK review format.",0,1,
	 "<file>","",new comm_option_mfptr<btmanip_class>
	 (this,&btmanip_class::utk_review),cli::comm_option_both}
      };
      cl->set_comm_option_vec(nopt,options);    
    
      p_verbose.i=&bf.verbose;
      p_verbose.help=((string)"Verbosity parameter ")+
	"(default 1).";
      cl->par_list.insert(make_pair("verbose",&p_verbose));

      p_recase_tag.b=&bf.recase_tag;
      p_recase_tag.help="If true, unify the case of the tags (default true).";
      cl->par_list.insert(make_pair("recase_tag",&p_recase_tag));
    
      p_reformat_journal.b=&bf.reformat_journal;
      p_reformat_journal.help="If true, reformat journal names (default true).";
      cl->par_list.insert(make_pair("reformat_journal",&p_reformat_journal));
    
      p_trans_latex_html.b=&bf.trans_latex_html;
      p_trans_latex_html.help=((string)"Translate LaTeX symbols to ")+
	"HTML and vice versa (default true).";
      cl->par_list.insert(make_pair("trans_latex_html",&p_trans_latex_html));
    
      p_normalize_tags.b=&bf.normalize_tags;
      p_normalize_tags.help=((string)"If true, convert tag strings ")+
	"to standard capitalization (default true).";
      cl->par_list.insert(make_pair("normalize_tags",&p_normalize_tags));
    
      p_lowercase_fields.b=&bf.lowercase_fields;
      p_lowercase_fields.help=((string)"If true, convert fields to ")+
	"lowercase (default true).";
      cl->par_list.insert(make_pair("lowercase_fields",&p_lowercase_fields));
    
      p_check_required.b=&bf.check_required;
      p_check_required.help=((string)"If true, check that all ")+
	"required fields are present (default false).";
      cl->par_list.insert(make_pair("check_required",&p_check_required));

      p_natbib_jours.b=&bf.natbib_jours;
      p_natbib_jours.help=((string)"Prefer natbib journal ")+
	"abbreviations (default false).";
      cl->par_list.insert(make_pair("natbib_jours",&p_natbib_jours));
    
      p_remove_vol_letters.b=&bf.remove_vol_letters;
      p_remove_vol_letters.help=((string)"Move letters in some volumes to")+
	"journal names (default false).";
      cl->par_list.insert(make_pair("remove_vol_letters",
				    &p_remove_vol_letters));

      p_remove_author_tildes.b=&bf.remove_author_tildes;
      p_remove_author_tildes.help=((string)"Remove tildes from ")+
	"author fields (default true).";
      cl->par_list.insert(make_pair("remove_author_tildes",
				    &p_remove_author_tildes));

      p_autoformat_urls.b=&bf.autoformat_urls;
      p_autoformat_urls.help=((string)"If DOI or ISBN is present, ")+
	"autoformat URLs (default true).";
      cl->par_list.insert(make_pair("autoformat_urls",&p_autoformat_urls));

      p_add_empty_titles.b=&bf.add_empty_titles;
      p_add_empty_titles.help=((string)"If article titles are not present, ")+
	"add empty ones (default true).";
      cl->par_list.insert(make_pair("add_empty_titles",&p_add_empty_titles));
    
      cl->prompt="btmanip> ";
      cl->addl_help_cmd=((string)"\n There is a custom BibTeX entry ")+
	"called 'Talk' which btmanip is designed to work with.\n \n"+
	"@Talk{\n"+
	"author={},\n"+
	"title={},\n"+
	"city={},\n"+
	"state={},\n"+
	"country={},\n"+
	"year={},\n"+
	"month={},\n"+
	"date={},\n"+
	"institution={},\n"+
	"conference={},\n"+
	"url={},\n"+
	"type={}\n"+
	"}";

      char *dc=getenv("BTMANIP_DEFAULTS");
      if (dc) {
	std::vector<cmd_line_arg> ca;
	std::string def_args=dc;
	cout << "Using default arguments: " << def_args << endl;
	cl->process_args_str(dc,ca);
	cl->call_args(ca);
      }
    
      if (argc<=1) {
	cl->run_interactive();
      } else {
	std::vector<cmd_line_arg> ca2;
	cl->process_args_c(argc,argv,ca2);
	cl->call_args(ca2);
      }
    
      return 0;
    }
  
  };

}

int main(int argc, char *argv[]) {
  
  btmanip_class bt;
  bt.run(argc,argv);

  return 0;
}
