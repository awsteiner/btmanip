/*
  -------------------------------------------------------------------

  Copyright (C) 2015-2017, Andrew W. Steiner

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
/** \file bib_file.h
    \brief File defining \ref bib_file
*/
#ifndef O2SCL_BIB_FILE_H
#define O2SCL_BIB_FILE_H

#include <iostream>
#include <fstream>

#include <fnmatch.h>

#include <boost/algorithm/string.hpp>

#include <bibtexentry.hpp>
#include <bibtexreader.hpp>
#include <bibtexwriter.hpp>

#include <o2scl/err_hnd.h>

/** \brief Main <tt>btmanip</tt> namespace
    
    This namespace is documented in \ref bib_file.h .
 */
namespace btmanip {
  
  /** \brief Manipulate BibTeX files using bibtex-spirit
      
      \future Move code from header to source file.
   */
  class bib_file {

  protected:

    /** \brief LaTeX strings for translation
     */
    std::vector<std::string> trans_latex;

    /** \brief HTML strings for translation
     */
    std::vector<std::string> trans_html;

    /** \brief Journal name list
     */
    std::map<std::string,std::vector<std::string>,
      std::greater<std::string> > journals;
    
    /** \brief Type for journal name list iterator
     */
    typedef std::map<std::string,std::vector<std::string>,
      std::greater<std::string> >::iterator journal_it;

  public:

    /** \brief Fields automatically removed by parse()
     */
    std::vector<std::string> remove_fields;
    
    /** \brief The full list of entries
     */
    std::vector<bibtex::BibTeXEntry> entries;

    /** \brief A sorted list of keys and indexes
     */
    std::map<std::string,size_t,std::less<std::string> > sort;
    
    /** \brief Remove extra whitespace inside entries (default false)
     */
    bool remove_extra_whitespace;
    /** \brief Recase tag so that only the first letter is upper case
     */
    bool recase_tag;
    /** \brief Reformat journal name from, e.g. \prl to Phys. Rev. Lett.
     */
    bool reformat_journal;
    /** \brief Translate LaTeX symbols to HTML and vice versa
     */
    bool trans_latex_html;
    /** \brief If true, convert tag strings to standard capitalization
     */
    bool normalize_tags;
    /** \brief If true, convert fields to lowercase
     */
    bool lowercase_fields;
    /** \brief If true, check to make sure all required fields are 
	present

	\note This only works if \ref normalize_tags and \ref
	lowercase_fields are both true.
    */
    bool check_required;
    /** \brief If true, prefer natbib journal abbreviations 
	(default false)
    */
    bool natbib_jours;
    /** \brief If true, move letters from volumes for some journals
	(default false)
    */
    bool remove_vol_letters;
    /** \brief If true, add and reformat URLs (default true)
     */
    bool autoformat_urls;
    /** \brief Add empty titles (default true)
     */
    bool add_empty_titles;
    /** \brief Verbosity parameter
     */
    int verbose;

    /** \brief Create a ``bib_file`` object
     */
    bib_file() {
      remove_extra_whitespace=false;
      recase_tag=true;
      reformat_journal=true;
      trans_latex_html=true;
      normalize_tags=true;
      lowercase_fields=true;
      check_required=false;
      natbib_jours=false;
      remove_vol_letters=false;
      autoformat_urls=true;
      add_empty_titles=true;
      verbose=1;

      trans_latex.push_back("\\'{a}"); trans_html.push_back("&aacute;");
      trans_latex.push_back("\\'{e}"); trans_html.push_back("&eacute;");
      trans_latex.push_back("\\'{i}"); trans_html.push_back("&iacute;");
      trans_latex.push_back("\\'{o}"); trans_html.push_back("&oacute;");
      trans_latex.push_back("\\'{u}"); trans_html.push_back("&uacute;");
      trans_latex.push_back("\\`{a}"); trans_html.push_back("&agrave;");
      trans_latex.push_back("\\`{e}"); trans_html.push_back("&egrave;");
      trans_latex.push_back("\\`{i}"); trans_html.push_back("&igrave;");
      trans_latex.push_back("\\`{o}"); trans_html.push_back("&ograve;");
      trans_latex.push_back("\\`{u}"); trans_html.push_back("&ugrave;");
      trans_latex.push_back("\\\"{a}"); trans_html.push_back("&auml;");
      trans_latex.push_back("\\\"{e}"); trans_html.push_back("&euml;");
      trans_latex.push_back("\\\"{i}"); trans_html.push_back("&iuml;");
      trans_latex.push_back("\\\"{o}"); trans_html.push_back("&ouml;");
      trans_latex.push_back("\\\"{u}"); trans_html.push_back("&uuml;");
      
      remove_fields={"adsnote","date-added","annote","bdsk-url-1",
		     "bdsk-url-2","date-modified","archiveprefix",
		     "primaryclass","abstract"};
    }

    /** \brief Read a journal name list from file \c fname

	\note If a list was read previously, that list is
	deleted before reading the new list
    */
    int read_journals(std::string fname="") {
      if (journals.size()>0) {
	journals.clear();
      }
      
      std::ifstream fin(fname);
      if (!fin.is_open()) {
	return o2scl::exc_efilenotfound;
      }
      std::vector<std::string> list;
      std::string line, line2;
      std::getline(fin,line);
      while (line.length()>0) {
	std::getline(fin,line2);
	while (line2!=((std::string)"done")) {
	  list.push_back(line2);
	  std::getline(fin,line2);
	}
	if (natbib_jours) {
	  for(size_t k=0;k<list.size();k++) {
	    if (list[k][0]=='\\') {
	      std::string temp=list[k];
	      list[k]=line;
	      line=temp;
	    }
	  }
	}
	if (verbose>1) {
	  std::cout << "Abbr: " << line << std::endl;
	  for(size_t k=0;k<list.size();k++) {
	    std::cout << "List " << k << " " << list[k] << std::endl;
	  }
	}
	journals.insert(std::make_pair(line,list));
	std::getline(fin,line);
	list.clear();
      }
      fin.close();
      if (verbose>0) {
	std::cout << journals.size() << " journal name entries read from \""
		  << fname << "\"." << std::endl;
      }
      return 0;
    }

    /** \brief Remove extra whitespace by parsing through a stringstream
	
	This function ensures that each word is separated by one and
	only one space, removing all other whitespace.
    */
    void thin_whitespace(std::string &s) {
      std::string s2, st;
      std::istringstream *is=new std::istringstream(s.c_str());
      if ((*is) >> st) {
	s2=st;
      }
      while((*is) >> st) {
	s2+=((std::string)" ")+st;
      }
      s=s2;
      return;
    }

    /** \brief Remove all whitespace and punctuation and
	convert to upper case
    */
    std::string journal_simplify(std::string s) {
      for(size_t i=0;i<s.size();i++) {
	if (!isalpha(s[i])) {
	  s=s.substr(0,i)+s.substr(i+1,s.length()-(i+1));
	  i=0;
	}
      }
      for(size_t i=0;i<s.size();i++) {
	s[i]=std::toupper(s[i]);
      }
      return s;
    }

    /** \brief Find the main abbreviation for a journal with
	name \c jour
    */
    int find_abbrev(std::string jour, std::string &abbrev) {
      if (journals.size()==0) {
	O2SCL_ERR("No journal list read in bib_file::find_abbrev().",
		  o2scl::exc_einval);
      }
      jour=journal_simplify(jour);
      for(journal_it jit=journals.begin();jit!=journals.end();jit++) {
	std::string stemp=jit->first;
	stemp=journal_simplify(stemp);
	if (jour==stemp) {
	  abbrev=jit->first;
	  return 0;
	}
	for(size_t k=0;k<jit->second.size();k++) {
	  stemp=journal_simplify(jit->second[k]);
	  if (jour==stemp) {
	    abbrev=jit->first;
	    return 0;
	  }
	}
      }
      return 1;
    }
    
    /** \brief Find the main abbreviation for a journal with
	name \c jour
    */
    int find_abbrevs(std::string jour, std::vector<std::string> &list) {
      if (journals.size()==0) {
	O2SCL_ERR("No journal list read in bib_file::find_abbrevs().",
		  o2scl::exc_einval);
      }
      jour=journal_simplify(jour);
      for(journal_it jit=journals.begin();jit!=journals.end();jit++) {
	bool match=false;
	std::string stemp=journal_simplify(jit->first);
	if (jour==stemp) {
	  match=true;
	}
	for(size_t k=0;k<jit->second.size();k++) {
	  stemp=journal_simplify(jit->second[k]);
	  if (jour==stemp) {
	    match=true;
	  }
	}
	if (match==true) {
	  list.push_back(jit->first);
	  for(size_t k=0;k<jit->second.size();k++) {
	    list.push_back(jit->second[k]);
	  }
	  return 0;
	}
      }
      return 1;
    }
    
    /** \brief Extract the first page from a list of pages
     */
    std::string first_page(std::string pages) {
      size_t loc=pages.find('-');
      std::string pages2=pages;
      if (loc!=std::string::npos) {
	pages2=pages.substr(0,loc);
      }
      return pages2;
    }

    /** \brief Search for entries using 'or'
     */
    void search_or(std::vector<std::string> &args) {

      if (args.size()%2!=0) {
	O2SCL_ERR("Need a set of field and pattern pairs in search_or().",
		  o2scl::exc_einval);
      }
      
      std::vector<bibtex::BibTeXEntry> entries2;
      
      for(size_t i=0;i<entries.size();i++) {

	bool entry_matches=false;
	bibtex::BibTeXEntry &bt=entries[i];

	for(size_t k=0;k<args.size();k+=2) {
	  std::string field=args[k];
	  std::string pattern=args[k+1];
	  for(size_t j=0;j<bt.fields.size();j++) {
	    if (bt.fields[j].first==field &&
		fnmatch(pattern.c_str(),bt.fields[j].second[0].c_str(),0)==0) {
	      entry_matches=true;
	    }
	  }
	}
	if (entry_matches) {
	  entries2.push_back(bt);
	}
      }
      if (entries2.size()>0) {
	if (verbose>0) {
	  if (entries2.size()==1) {
	    std::cout << "1 record found." << std::endl;
	  } else {
	    std::cout << entries2.size() << " records found." << std::endl;
	  }
	}
	std::swap(entries,entries2);
      } else {
	if (verbose>0) {
	  std::cout << "No records found." << std::endl;
	}
      }
      return;
    }
    
    /** \brief Remove matching entries using 'or'
     */
    void remove_or(std::vector<std::string> &args) {

      if (args.size()%2!=0) {
	O2SCL_ERR("Need a set of field and pattern pairs in remove_or().",
		  o2scl::exc_einval);
      }
      
      bool restart_loop=true;
      while (restart_loop) {
	restart_loop=false;
	
	for(std::vector<bibtex::BibTeXEntry>::iterator it=entries.begin();
	    restart_loop==false && it!=entries.end();it++) {
	  
	  bibtex::BibTeXEntry &bt=*it;
	  
	  for(size_t k=0;restart_loop==false && k<args.size();k+=2) {
	    std::string field=args[k];
	    std::string pattern=args[k+1];
	    for(size_t j=0;restart_loop==false && j<bt.fields.size();j++) {
	      if (bt.fields[j].first==field &&
		  fnmatch(pattern.c_str(),
			  bt.fields[j].second[0].c_str(),0)==0) {
		entries.erase(it);
		// Reset the interator and restart the loops
		restart_loop=true;
	      }
	    }
	  }
	}
      }
      
      if (verbose>0) {
	if (entries.size()==0) {
	  std::cout << "No records remaining." << std::endl;
	} else if (entries.size()==1) {
	  std::cout << "1 record remaining." << std::endl;
	} else {
	  std::cout << entries.size() << " records remaining." << std::endl;
	}
      }
      return;
    }
    
    /** \brief Search for entries using 'and'
     */
    void search_and(std::vector<std::string> &args) {

      if (args.size()%2!=0) {
	O2SCL_ERR("Need a set of field and pattern pairs in search_and().",
		  o2scl::exc_einval);
      }
      
      for(size_t k=0;k<args.size();k+=2) {

	std::string field=args[k];
	std::string pattern=args[k+1];
	
	std::vector<bibtex::BibTeXEntry> entries2;

	for(size_t i=0;i<entries.size();i++) {
	  bool entry_matches=false;
	  bibtex::BibTeXEntry &bt=entries[i];
	  for(size_t j=0;j<bt.fields.size();j++) {
	    if (bt.fields[j].first==field &&
		fnmatch(pattern.c_str(),bt.fields[j].second[0].c_str(),0)==0) {
	      entry_matches=true;
	    }
	  }
	  if (entry_matches) {
	    entries2.push_back(bt);
	  }
	}

	if (entries2.size()>0) {
	  std::swap(entries,entries2);
	} else {
	  if (verbose>0) {
	    std::cout << "No records found." << std::endl;
	  }
	  return;
	}
      }

      if (verbose>0) {
	if (entries.size()==1) {
	  std::cout << "1 record found." << std::endl;
	} else {
	  std::cout << entries.size() << " records found." << std::endl;
	}
      }

      return;
    }

    /** \brief Check entry for required fields
     */
    void entry_check_required(bibtex::BibTeXEntry &bt) {
      if (bt.tag==((std::string)"Article")) {
	if (!is_field_present(bt,"author")) {
	  O2SCL_ERR("Article missing author field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("Article missing title field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"journal")) {
	  O2SCL_ERR("Article missing journal field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"year")) {
	  O2SCL_ERR("Article missing year field.",
		    o2scl::exc_einval);
	}
      } else if (bt.tag==((std::string)"Book")) {
	if (!is_field_present(bt,"author","editor")) {
	  O2SCL_ERR("Book missing author field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("Book missing title field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"publisher")) {
	  O2SCL_ERR("Book missing publisher field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"year")) {
	  O2SCL_ERR("Book missing year field.",
		    o2scl::exc_einval);
	}
      } else if (bt.tag==((std::string)"Booklet")) {
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("Booklet missing title field.",
		    o2scl::exc_einval);
	}
      } else if (bt.tag==((std::string)"Conference")) {
	if (!is_field_present(bt,"author")) {
	  O2SCL_ERR("Conference missing author field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("Conference missing title field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"booktitle")) {
	  O2SCL_ERR("Conference missing booktitle field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"year")) {
	  O2SCL_ERR("Conference missing year field.",
		    o2scl::exc_einval);
	}
      } else if (bt.tag==((std::string)"InBook")) {
	if (!is_field_present(bt,"author","editor")) {
	  O2SCL_ERR("InBook missing author field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("InBook missing title field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"chapter","pages")) {
	  O2SCL_ERR("InBook missing chapter field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"publisher")) {
	  O2SCL_ERR("InBook missing publisher field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"year")) {
	  O2SCL_ERR("InBook missing year field.",
		    o2scl::exc_einval);
	}
      } else if (bt.tag==((std::string)"InCollection")) {
	if (!is_field_present(bt,"author","editor")) {
	  O2SCL_ERR("InCollection missing author field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("InCollection missing title field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"publisher")) {
	  O2SCL_ERR("InCollection missing publisher field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"year")) {
	  O2SCL_ERR("InCollection missing year field.",
		    o2scl::exc_einval);
	}
      } else if (bt.tag==((std::string)"InProceedings")) {
	if (!is_field_present(bt,"author")) {
	  O2SCL_ERR("InProceedings missing author field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("InProceedings missing title field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"booktitle")) {
	  O2SCL_ERR("InProceedings missing booktitle field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"year")) {
	  O2SCL_ERR("InProceedings missing year field.",
		    o2scl::exc_einval);
	}
      } else if (bt.tag==((std::string)"Manual")) {
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("Manual missing title field.",
		    o2scl::exc_einval);
	}
      } else if (bt.tag==((std::string)"MastersThesis")) {
	if (!is_field_present(bt,"author")) {
	  O2SCL_ERR("MastersThesis missing author field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("MastersThesis missing title field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"school")) {
	  O2SCL_ERR("MastersThesis missing school field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"year")) {
	  O2SCL_ERR("MastersThesis missing year field.",
		    o2scl::exc_einval);
	}
	//} else if (bt.tag==((std::string)"Misc")) {
	// Misc has no required fields
      } else if (bt.tag==((std::string)"PhDThesis")) {
	if (!is_field_present(bt,"author")) {
	  O2SCL_ERR("PhDThesis missing author field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("PhDThesis missing title field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"school")) {
	  O2SCL_ERR("PhDThesis missing school field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"year")) {
	  O2SCL_ERR("PhDThesis missing year field.",
		    o2scl::exc_einval);
	}
      } else if (bt.tag==((std::string)"Proceedings")) {
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("Proceedings missing title field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"year")) {
	  O2SCL_ERR("Proceedings missing year field.",
		    o2scl::exc_einval);
	}
      } else if (bt.tag==((std::string)"TechReport")) {
	if (!is_field_present(bt,"author")) {
	  O2SCL_ERR("TechReport missing author field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("TechReport missing title field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"institution")) {
	  O2SCL_ERR("TechReport missing institution field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"year")) {
	  O2SCL_ERR("TechReport missing year field.",
		    o2scl::exc_einval);
	}
      } else if (bt.tag==((std::string)"Unpublished")) {
	if (!is_field_present(bt,"author")) {
	  O2SCL_ERR("Unpublished missing author field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"title")) {
	  O2SCL_ERR("Unpublished missing title field.",
		    o2scl::exc_einval);
	}
	if (!is_field_present(bt,"note")) {
	  O2SCL_ERR("Unpublished missing note field.",
		    o2scl::exc_einval);
	}
      }
      return;
    }

    /** \brief If an 'article' or 'inproceedings' and no
	title, add an empty title
     */
    size_t entry_add_empty_title(bibtex::BibTeXEntry &bt) {

      size_t empty_titles_added=0;
      if (bt.tag==((std::string)"Article") ||
	  bt.tag==((std::string)"InProceedings")) {
	if (!is_field_present(bt,"title")) {
	  std::vector<std::string> val;
	  val.push_back(" ");
	  bt.fields.push_back(std::make_pair("title",val));
	  empty_titles_added++;
	  if (verbose>1) {
	    std::cout << "In entry with key " << *bt.key
		      << " added empty title." << std::endl;
	  }
	}
      }
      return empty_titles_added;
    }
    
    /** \brief If DOI number is present, ensure URL matches
     */
    size_t entry_autoformat_url(bibtex::BibTeXEntry &bt) {
      size_t urls_reformatted=0;
      if (bt.tag==((std::string)"Article")) {
	if (is_field_present(bt,"doi")) {
	  if (is_field_present(bt,"url")) {
	    std::string &url=get_field(bt,"url");
	    if (url.substr(0,17)!=((std::string)"http://dx.doi.org")) {
	      url=((std::string)"http://dx.doi.org/")+
		get_field(bt,"doi");
	      if (verbose>1) {
		std::cout << "In entry with key " << *bt.key
			  << " reformatted url to " << url << std::endl;
	      }
	    }
	  } else {
	    std::vector<std::string> val;
	    val.push_back(((std::string)"http://dx.doi.org/")+
			  get_field(bt,"doi"));
	    bt.fields.push_back(std::make_pair("url",val));
	    if (verbose>1) {
	      std::cout << "In entry with key " << *bt.key
			<< " added url field " << val[0] << std::endl;
	    }
	  }
	  urls_reformatted++;
	}
      } else if (bt.tag==((std::string)"Book")) {
	if (is_field_present(bt,"isbn") && !is_field_present(bt,"url")) {
	    std::vector<std::string> val;
	    val.push_back(((std::string)"http://www.worldcat.org/isbn/")+
			  get_field(bt,"isbn"));
	    bt.fields.push_back(std::make_pair("url",val));
	    if (verbose>1) {
	      std::cout << "In entry with key " << *bt.key
			<< " added url field " << val[0] << std::endl;
	    }
	}
      }
      return urls_reformatted;
    }
    
    /** \brief Remove volume letters and move to journal names
	for some journals
     */
    void entry_remove_vol_letters(bibtex::BibTeXEntry &bt) {
      if (is_field_present(bt,"journal") &&
	  is_field_present(bt,"volume")) {
	std::string volume=get_field(bt,"volume");
	std::string journal=get_field(bt,"journal");
	if (journal==((std::string)"Phys. Rev.") &&
	    (volume[0]=='A' || volume[0]=='a' ||
	     volume[0]=='B' || volume[0]=='b' ||
	     volume[0]=='C' || volume[0]=='c' ||
	     volume[0]=='D' || volume[0]=='d' ||
	     volume[0]=='E' || volume[0]=='e')) {
	  if (verbose>1) {
	    std::cout << "In entry with key " << *bt.key
		      << " reformatting journal and volume from "
		      << journal << ", " << volume << " to ";
	  }
	  journal+=" ";
	  journal+=std::toupper(volume[0]);
	  volume=volume.substr(1,volume.length()-1);
	  if (verbose>1) {
	    std::cout << journal << ", " << volume << std::endl;
	  }
	  get_field(bt,"journal")=journal;
	  get_field(bt,"volume")=volume;
	}
	if (journal==((std::string)"Eur. Phys. J.") &&
	    (volume[0]=='A' || volume[0]=='a' ||
	     volume[0]=='B' || volume[0]=='b' ||
	     volume[0]=='C' || volume[0]=='c' ||
	     volume[0]=='D' || volume[0]=='d' ||
	     volume[0]=='E' || volume[0]=='e')) {
	  if (verbose>1) {
	    std::cout << "In entry with key " << *bt.key
		      << " reformatting journal and volume from "
		      << journal << ", " << volume << " to ";
	  }
	  journal+=" ";
	  journal+=std::toupper(volume[0]);
	  volume=volume.substr(1,volume.length()-1);
	  if (verbose>1) {
	    std::cout << journal << ", " << volume << std::endl;
	  }
	  get_field(bt,"journal")=journal;
	  get_field(bt,"volume")=volume;
	}
	if (journal==((std::string)"Nucl. Phys.") &&
	    (volume[0]=='A' || volume[0]=='a' ||
	     volume[0]=='B' || volume[0]=='b')) {
	  if (verbose>1) {
	    std::cout << "In entry with key " << *bt.key
		      << " reformatting journal and volume from "
		      << journal << ", " << volume << " to ";
	  }
	  journal+=" ";
	  journal+=std::toupper(volume[0]);
	  volume=volume.substr(1,volume.length()-1);
	  if (verbose>1) {
	    std::cout << journal << ", " << volume << std::endl;
	  }
	  get_field(bt,"journal")=journal;
	  get_field(bt,"volume")=volume;
	}
	if (journal==((std::string)"Phys. Lett.") &&
	    (volume[0]=='A' || volume[0]=='a' ||
	     volume[0]=='B' || volume[0]=='b')) {
	  if (verbose>1) {
	    std::cout << "In entry with key " << *bt.key
		      << " reformatting journal and volume from "
		      << journal << ", " << volume << " to ";
	  }
	  journal+=" ";
	  journal+=std::toupper(volume[0]);
	  volume=volume.substr(1,volume.length()-1);
	  if (verbose>1) {
	    std::cout << journal << ", " << volume << std::endl;
	  }
	  get_field(bt,"journal")=journal;
	  get_field(bt,"volume")=volume;
	}
      }
      return;
    }
    
    /** \brief Clean the current BibTeX entries
     */
    void clean() {

      size_t empty_titles_added=0;
      size_t duplicates_found=0;
      size_t entries_fields_removed=0;
      size_t journals_renamed=0;
      size_t urls_reformatted=0;
      
      if (verbose>1) {
	std::cout << "normalize_tags: " << normalize_tags << std::endl;
	std::cout << "lowercase_fields: " << lowercase_fields << std::endl;
	std::cout << "recase_tag: " << recase_tag << std::endl;
	std::cout << "reformat_journal: " << reformat_journal << std::endl;
	std::cout << "check_required: " << check_required << std::endl;
	std::cout << "remove_extra_whitespace: "
		  << remove_extra_whitespace << std::endl;
	std::cout << "remove_vol_letters: " << remove_vol_letters << std::endl;
	std::cout << "natbib_jours: " << natbib_jours << std::endl;
	std::cout << "autoformat_urls: " << autoformat_urls << std::endl;
	std::cout << "add_empty_titles: " << add_empty_titles << std::endl;
      }

      if (journals.size()==0) {
	std::cout << "No entries in journal name list." << std::endl;
      }

      if (entries.size()==0) {
	std::cout << "No entries to clean." << std::endl;
      }
      
      // First loop to fix tags (must be done before looking
      // for duplicates)
      for(size_t i=0;i<entries.size();i++) {
      
	bibtex::BibTeXEntry &bt=entries[i];
      
	if (normalize_tags) {
	  // Capitalize first letter and downcase all other letters
	  bt.tag[0]=std::toupper(bt.tag[0]);
	  for(size_t i=1;i<bt.tag.size();i++) {
	    bt.tag[i]=std::tolower(bt.tag[i]);
	  }
	  // Manually fix tags which normally have more than one
	  // uppercase letter
	  if (bt.tag==((std::string)"Inbook")) {
	    bt.tag="InBook";
	  } else if (bt.tag==((std::string)"Incollection")) {
	    bt.tag="InCollection";
	  } else if (bt.tag==((std::string)"Inproceedings")) {
	    bt.tag="InProceedings";
	  } else if (bt.tag==((std::string)"Mastersthesis")) {
	    bt.tag="MastersThesis";
	  } else if (bt.tag==((std::string)"Phdthesis")) {
	    bt.tag="PhDThesis";
	  } else if (bt.tag==((std::string)"Techreport")) {
	    bt.tag="TechReport";
	  }
	}
      }
      
      // Now look for duplicates. Use int rather than size_t
      // so we can reset to i=-1 when we erase an entry
      for(int i=0;i<((int)entries.size());i++) {
	for(int j=i+1;j<((int)entries.size());j++) {
	  std::string key1=*(entries[i].key);
	  std::string key2=*(entries[j].key);
	  if (entries[i].tag==entries[j].tag && key1==key2) {
	    entries.erase(entries.begin()+j);
	    j=entries.size();
	    i=-1;
	    duplicates_found++;
	  }
	}
      }
      
      // Reformat if necessary. This loop is over each entry
      for(size_t i=0;i<entries.size();i++) {
      
	bibtex::BibTeXEntry &bt=entries[i];

	// Loop over each field

	bool restart_loop=true;
	while (restart_loop) {
	  restart_loop=false;
	  
	  for(size_t j=0;j<bt.fields.size();j++) {
	    
	    // Ensure the field name is all lowercase
	    if (lowercase_fields) {
	      std::string fitemp=bt.fields[j].first, fitemp2=fitemp;
	      for(size_t k=0;k<fitemp2.size();k++) {
		fitemp2[k]=std::tolower(fitemp2[k]);
	      }
	      if (fitemp2!=fitemp) {
		bt.fields[j].first=fitemp2;
	      }
	    }

	    // Remove extra braces from each value
	    std::string &valtemp=bt.fields[j].second[0];
	    bool removed_verb=false;
	    while (valtemp.length()>=4 && valtemp[0]=='{' &&
		   valtemp[1]=='{' && valtemp[valtemp.size()-1]=='}' &&
		   valtemp[valtemp.size()-2]=='}') {
	      valtemp=valtemp.substr(1,valtemp.size()-2);
	      if (removed_verb==false && verbose>1) {
		std::cout << "Removing extra braces in entry with key "
			  << *bt.key << " for field " << bt.fields[j].first
			  << "with value:\n" << valtemp << std::endl;
		removed_verb=true;
	      }
	    }

	    // Remove extra fields
	    bool field_removed=false;
	    for(size_t k=0;k<remove_fields.size();k++) {
	      if (bt.fields[j].first==((std::string)remove_fields[k])) {
		if (verbose>1) {
		  std::cout << "Removing extra field "
			    << bt.fields[j].first
			    << " in entry with key " << *bt.key << std::endl;
		}
		bt.fields.erase(bt.fields.begin()+j);
		restart_loop=true;
		j=bt.fields.size();
		field_removed=true;
	      }
	    }

	    if (field_removed) {
	      entries_fields_removed++;
	    }
	  
	    if (field_removed==false) {
	    
	      if (bt.fields[j].second.size()==0) {
		std::string err=((std::string)"Field ")+bt.fields[j].first+
		  " has no values";
		O2SCL_ERR(err.c_str(),o2scl::exc_einval);
	      } else if (bt.fields[j].second.size()>1) {
		std::string err=((std::string)"Field ")+bt.fields[j].first+
		  " has more than one value";
		O2SCL_ERR(err.c_str(),o2scl::exc_einval);
	      }
	  
	      if (remove_extra_whitespace) {
		for(size_t k=0;k<bt.fields[j].second.size();k++) {
		  thin_whitespace(bt.fields[j].second[k]);
		}
	      }

	      // Reformat journal name by replacing it with the
	      // standard abbreviation
	      if (reformat_journal &&
		  bt.fields[j].first==((std::string)"journal") &&
		  journals.size()>0 ) {
		if (bt.fields[j].second.size()>0) {
		  std::string jour=bt.fields[j].second[0];
		  std::string abbrev;
		  if (find_abbrev(jour,abbrev)==0) {
		    if (jour!=abbrev) {
		      if (verbose>1) {
			std::cout << "Reformatting journal " << jour << " to "
				  << abbrev << std::endl;
		      }
		      bt.fields[j].second[0]=abbrev;
		      journals_renamed++;
		    }
		  } else {
		    std::cout << "Journal " << jour << " not found in key "
			      << *bt.key << " ." << std::endl;
		  }
		}
	      }

	      // End of if (field_removed==false)
	    }
	    // End of loop over fields
	  }
	}

	// If the journal letter is in the volume, move to
	// the journal field
	if (remove_vol_letters) {
	  entry_remove_vol_letters(bt);
	}

	// If necessary, create an article URL from the
	// DOI entry
	if (autoformat_urls) {
	  entry_autoformat_url(bt);
	}
		  
	// Add empty title to an article if necessary
	if (add_empty_titles) {
	  empty_titles_added=entry_add_empty_title(bt);
	}

	// If requested, check that required fields are present
	// for each entry
	if (normalize_tags && lowercase_fields && check_required) {
	  entry_check_required(bt);
	}
      
	// End of loop over entries
      }

      if (verbose>0) {
	std::cout << empty_titles_added << " emtpy titles added." << std::endl;
	std::cout << duplicates_found << " duplicates found." << std::endl;
	std::cout << entries_fields_removed
		  << " entries with extra fields removed." << std::endl;
	std::cout << journals_renamed << " journal names standardized."
		  << std::endl;
	std::cout <<  urls_reformatted << " URLs reformatted."
		  << std::endl;
	  
      }
      
      return;
    }

    /** \brief In entry \c bt, set the value of \c field
	equal to \c value
    */
    int set_field_value(bibtex::BibTeXEntry &bt, std::string field,
			std::string value) {
      for(size_t j=0;j<bt.fields.size();j++) {
	if (bt.fields[j].first==field) {
	  bt.fields[j].second[0]=value;
	}
      }
      
      return 0;
    }
    
    /** \brief In entry with key \c key, set the value of \c field
	equal to \c value
    */
    int set_field_value(std::string key, std::string field,
			std::string value) {
      bibtex::BibTeXEntry &bt=get_entry_by_key(key);
      return set_field_value(bt,field,value);
    }
    
    /** \brief Parse a BibTeX file and perform some extra reformatting
     */
    void parse_bib(std::string fname) {

      // Erase current entries
      if (entries.size()>0) {
	entries.clear();
	sort.clear();
      }
      
      // Main parse call
      if (verbose>1) std::cout << "Main parse call." << std::endl;
      std::ifstream in(fname.c_str());
      bibtex::read(in,entries); 
      in.close();
      if (verbose>1) std::cout << "Done with main parse call." << std::endl;

      // Loop over entries
      for(size_t i=0;i<entries.size();i++) {
      
	bibtex::BibTeXEntry &bt=entries[i];

	// Insert to the map for sorting
	if (bt.key) {
	  if (sort.find(*bt.key)==sort.end()) {
	    sort.insert(make_pair(*bt.key,i));
	  } else {
	    std::cout << "Warning: multiple copies with key "
		      << *bt.key << "." << std::endl;
	  }
	} else {
	  O2SCL_ERR("This class does not support keyless entries.",
		    o2scl::exc_efailed);
	}

	if (verbose>1) {
	  std::cout << "Entry " << i+1 << " of " << entries.size();
	  std::cout << ", tag: " << bt.tag;
	  if (bt.key) {
	    std::cout << ", key: " << *bt.key << std::endl;
	  } else {
	    std::cout << ", (no key)." << std::endl;
	  }
	}

	// End of loop over entries
      }

      /* 
	 This test will fail if there are multiple entries with
	 the same key. We currently allow this, thus this 
	 test must be removed. 
      */
      if (false && entries.size()!=sort.size()) {
	std::cout << "Entries: " << entries.size() << " sort: "
		  << sort.size() << std::endl;
	O2SCL_ERR2("Entries and sort structures mismatched in ",
		   "bib_file::parse_bib().",o2scl::exc_efailed);
      }
      
      if (verbose>0) {
	std::cout << "Read " << entries.size() << " entries from file "
		  << fname << std::endl;
      }

      // Just for debugging
      if (false) {
	std::cout << "Sort: " << std::endl;
	for(std::map<std::string,size_t,
	      std::greater<std::string> >::iterator it=sort.begin();
	    it!=sort.end();it++) {
	  std::cout << it->first << " " << it->second << std::endl;
	}
      }
      
      return;
    }
    
    /** \brief Refresh the \ref sort object which contains a set
	of keys an indexes for the \ref entries array
     */
    void refresh_sort() {
      sort.clear();
      // Go through all entries
      for(size_t i=0;i<entries.size();i++) {
	bibtex::BibTeXEntry &bt=entries[i];
	// If the key is not already in the sort map, add it
	if (sort.find(*bt.key)==sort.end()) {
	  sort.insert(make_pair(*bt.key,i));
	}
      }
      return;
    }
    
    /** \brief Sort the bibliography by key
     */
    void sort_bib() {
      std::vector<bibtex::BibTeXEntry> entries2;
      for(std::map<std::string,size_t,
	    std::greater<std::string> >::iterator it=sort.begin();
	  it!=sort.end();it++) {
	entries2.push_back(entries[it->second]);
      }
      std::swap(entries,entries2);
      refresh_sort();
      return;
    }
    
    /** \brief Output one entry \c bt to stream \c outs in 
	.bib format
    */
    void bib_output_one(std::ostream &outs, bibtex::BibTeXEntry &bt) {

      // Output tag and key
      outs << "@" << bt.tag << "{";
      if (bt.key) outs << *bt.key;
      outs << "," << std::endl;
      
      for(size_t j=0;j<bt.fields.size();j++) {

	if (bt.fields[j].second.size()>0) {

	  // Construct and output field string, including spaces to
	  // make it 16 characters. This is the same as the default
	  // emacs formatting.
	  std::string field_s=((std::string)"  ")+bt.fields[j].first+" =";
	  while (field_s.length()<16) field_s+=" ";
	  outs << field_s;

	  // Determine if the field value needs extra braces
	  bool with_braces=true;
	  if (bt.fields[j].first=="year") {
	    with_braces=false;
	  }
	  if (bt.fields[j].second[0][0]=='{' &&
	      bt.fields[j].second[0][bt.fields[j].second[0].size()-1]=='}' &&
	      bt.fields[j].second[0].find('{',1)==std::string::npos) {
	    with_braces=false;
	  }
	  // Don't surround purely numeric values with braces
	  // unless they begin with a '0'. 
	  if ((bt.fields[j].first=="pages" ||
	       bt.fields[j].first=="numpages" ||
	       bt.fields[j].first=="volume" ||
	       bt.fields[j].first=="issue" ||
	       bt.fields[j].first=="isbn" ||
	       bt.fields[j].first=="citations" ||
	       bt.fields[j].first=="adscites" ||
	       bt.fields[j].first=="number") &&
	      bt.fields[j].second[0].size()>0 &&
	      bt.fields[j].second[0][0]!='0') {
	    bool has_nonnum=false;
	    for(size_t i=0;i<bt.fields[j].second[0].size();i++) {
	      if (!isdigit(bt.fields[j].second[0][i])) {
		has_nonnum=true;
	      }
	    }
	    if (has_nonnum==false) {
	      with_braces=false;
	    }
	  }

	  // Output with or without braces, and with or without a
	  // comma, as necessary
	  if (with_braces==false) {
	    if (j+1==bt.fields.size()) {
	      outs << bt.fields[j].second[0] << std::endl;
	    } else {
	      outs << bt.fields[j].second[0] << "," << std::endl;
	    }
	  } else {
	    if (j+1==bt.fields.size()) {
	      outs << "{" << bt.fields[j].second[0] << "}" << std::endl;
	    } else {
	      outs << "{" << bt.fields[j].second[0] << "}," << std::endl;
	    }
	  }
	}
      }

      // Output final brace
      outs << "}" << std::endl;
      
      return;
    }

    /** \brief Return a positive number if \c bt and \c bt2 are possible
	duplicates

	This function returns 1 if the tags and keys are identical and
	2 if the tags are the same and the keys are different, but the
	volume pages, and journal are the same. This function returns
	zero otherwise.
    */
    int possible_duplicate(bibtex::BibTeXEntry &bt, bibtex::BibTeXEntry &bt2) {
      if (bt.tag==bt2.tag && (*bt.key)==(*bt2.key)) {
	return 1;
      }
      if (bt.tag==bt2.tag &&
	  is_field_present(bt,"volume") &&
	  is_field_present(bt,"pages") &&
	  is_field_present(bt,"journal") &&
	  is_field_present(bt2,"volume") &&
	  is_field_present(bt2,"pages") &&
	  is_field_present(bt2,"journal") &&
	  get_field(bt,"volume")== get_field(bt2,"volume") &&
	  first_page(get_field(bt,"pages"))==
	  first_page(get_field(bt2,"pages")) &&
	  get_field(bt,"journal")== get_field(bt2,"journal")) {
	return 2;
      }
      return 0;
    }

    /** \brief Create a list of possible duplicates of \c bt
	in the current set of BibTeX entries
    */
    void list_possible_duplicates(bibtex::BibTeXEntry &bt,
				  std::vector<size_t> &list) {
      list.clear();
      for(size_t i=0;i<entries.size();i++) {
	bibtex::BibTeXEntry &bt2=entries[i];
	if (possible_duplicate(bt,bt2)>0) {
	  list.push_back(i);
	}
      }
      return;
    }
    
    /** \brief Output one entry \c bt to stream \c outs in 
	plain text
    */
    void text_output_one(std::ostream &outs, bibtex::BibTeXEntry &bt) {
      outs << "tag: " << bt.tag << std::endl;
      if (bt.key) outs << "key: " << *bt.key << std::endl;
      for(size_t j=0;j<bt.fields.size();j++) {
	outs << bt.fields[j].first << ": ";
	if (bt.fields[j].second.size()>0) {
	  outs << bt.fields[j].second[0] << std::endl;
	} else {
	  outs << "(none)" << std::endl;
	}
	if (bt.fields[j].first==((std::string)"author")) {
	  if (bt.fields[j].second.size()>0) {
	    outs << "author (reformat): " 
		 << author_firstlast(bt.fields[j].second[0])
		 << std::endl;
	  }
	}
      }
      return;
    }
    
    /** \brief Parse a BibTeX file and perform some extra reformatting
     */
    void add_bib(std::string fname) {

      std::vector<bibtex::BibTeXEntry> entries2;

      // Main parse call
      if (verbose>1) std::cout << "Main parse call." << std::endl;
      std::ifstream in(fname.c_str());
      bibtex::read(in,entries2); 
      in.close();
      if (verbose>1) std::cout << "Done with main parse call." << std::endl;

      // Loop over entries
      for(size_t i=0;i<entries2.size();i++) {

	bibtex::BibTeXEntry &bt=entries2[i];

	std::vector<size_t> list;
	list_possible_duplicates(bt,list);
	if (list.size()>0) {
	  std::cout << "When adding entry:\n" << std::endl;
	  bib_output_one(std::cout,bt);
	  std::cout << "\n" << list.size() << " possible duplicates in the "
		    << "current list were found:\n" << std::endl;
	  for(size_t j=0;j<list.size();j++) {
	    bib_output_one(std::cout,entries[list[j]]);
	  }
	  if (list.size()==1) {
	    std::cout << "\nAdd entry anyway (y), replace (r), "
		      << "stop add (s) or ignore (i)? " << std::flush;
	  } else {
	    std::cout << "\nAdd entry anyway (y), "
		      << "stop add (s) or ignore (i)? " << std::flush;
	  }
	  char ch;
	  std::cin >> ch;
	  if (ch=='y' || ch=='Y') {
	    entries.push_back(bt);
	    
	    if (!bt.key) {
	      O2SCL_ERR("This class does not support keyless entries.",
			o2scl::exc_efailed);
	    }
	    
	    // Insert to the map for sorting
	    sort.insert(make_pair(*bt.key,entries.size()-1));
	    
	    if (verbose>1) {
	      std::cout << "Entry " << i+1 << " of " << entries2.size();
	      std::cout << ", tag: " << bt.tag;
	      std::cout << ", key: " << *bt.key << std::endl;
	    }
	  } else if (list.size()==1 && (ch=='r' || ch=='R')) {
	    std::cout << "Replacing " << *(entries[list[0]].key)
		      << " with " << *bt.key << std::endl;
	    entries[list[0]]=bt;
	  } else if (ch=='s' || ch=='S') {
	    std::cout << "Stopping add." << std::endl;
	    i=entries2.size();
	  } else {
	    std::cout << "Ignoring " << *bt.key << std::endl;
	  }
	}
	
	// End of loop over entries
      }
      
      /* 
	 This test will fail if there are multiple entries with
	 the same key. We currently allow this, thus this 
	 test must be removed. 
      */
      if (false && entries.size()!=sort.size()) {
	std::cout << "Entries: " << entries.size() << " sort: "
		  << sort.size() << std::endl;
	O2SCL_ERR2("Entries and sort structures mismatched in ",
		   "bib_file::parse_bib().",o2scl::exc_efailed);
      }
      
      if (verbose>0) {
	std::cout << "Read " << entries2.size() << " entries from file \""
		  << fname << "\". Now " << entries.size() 
		  << " total entries with " << sort.size()
		  << " sortable entries." << std::endl;
      }

      return;
    }

    /** \brief Get entry by key name
     */
    bibtex::BibTeXEntry &get_entry_by_key(std::string key) {
      return entries[sort.find(key)->second];
    }

    /** \brief Get index of entry by key name
     */
    size_t get_index_by_key(std::string key) {
      return sort.find(key)->second;
    }

    /** \brief Translate LaTeX sequences to HTML
     */
    std::string reformat_latex_html(std::string s_in) {
      for(size_t i=0;i<trans_latex.size();i++) {
	if (s_in.find(trans_latex[i])!=std::string::npos) {
	  boost::replace_all(s_in,trans_latex[i],trans_html[i]);
	}
      }
      return s_in;
    }

    /** \brief Return the last name of the first author,
	and "et al." if there is more than one author
    */
    std::string short_author(bibtex::BibTeXEntry &bt) {
      std::string auth=get_field(bt,"author");
      std::vector<std::string> firstv, lastv;
      parse_author(auth,firstv,lastv);
      std::string ret;
      if (firstv.size()>1) {
	ret=lastv[0]+" et al.";
      } else {
	ret=lastv[0];
      }
      return ret;
    }
    
    /** \brief Reformat author string into first and last names
     */
    void parse_author(std::string s_in,
		      std::vector<std::string> &firstv, 
		      std::vector<std::string> &lastv) {

      std::istringstream *is=new std::istringstream(s_in.c_str());
      std::string stmp;
      while ((*is) >> stmp) {
	std::string first, last=stmp;
	bool done=false;
	while (done==false && stmp[stmp.length()-1]!=',') {
	  if (!((*is) >> stmp)) {
	    done=true;
	  } else {
	    last+=((std::string)" ")+stmp;
	  }
	}
	if (done==true) {
	  firstv.push_back("(none)");
	  lastv.push_back(last);
	} else {
	  while ((*is) >> stmp && stmp!=((std::string)"and")) {
	    if (first.length()==0) {
	      first=stmp;
	    } else {
	      first+=((std::string)" ")+stmp;
	    }
	  }
	  if (last[last.length()-1]==',') {
	    last=last.substr(0,last.length()-1);
	  }
	  firstv.push_back(first);
	  lastv.push_back(last);
	}
      }
      /*
	std::cout << "ra done" << std::endl;
	for(size_t i=0;i<firstv.size();i++) {
	std::cout << i << " First: " << firstv[i] << " Last: "
	<< lastv[i] << std::endl;
	}
      */
      return;
    }

    /** \brief Reformat author string into a list with commas and
	the word <tt>"and"</tt> before the last author
    */
    std::string author_firstlast(std::string s_in) {

      std::vector<std::string> firstv, lastv;

      parse_author(s_in,firstv,lastv);

      // Now construct s_out from the firstv and lastv objects
      if (firstv.size()==1) {
	s_in=firstv[0]+" "+lastv[0];
      } else if (firstv.size()==2) {
	s_in=firstv[0]+" "+lastv[0]+" ";
	s_in+="and "+firstv[firstv.size()-1]+" "+lastv[1];
      } else {
	s_in="";
	for(size_t i=0;i<firstv.size()-1;i++) {
	  s_in+=firstv[i]+" "+lastv[i]+", ";
	}
	s_in+="and "+firstv[firstv.size()-1]+" "+lastv[lastv.size()-1];
      }
      return s_in;
    }

    /** \brief Return true if field named \c field is present in entry \c bt
     */
    bool is_field_present(bibtex::BibTeXEntry &bt, std::string field) {
      for(size_t j=0;j<bt.fields.size();j++) {
	if (bt.fields[j].first==field && bt.fields[j].second.size()>0) {
	  return true;
	}
      } 
      return false;
    }

    /** \brief Return true if field named \c field1 or field named \c
	field2 is present in entry \c bt
    */
    bool is_field_present(bibtex::BibTeXEntry &bt, std::string field1,
			  std::string field2) {
      for(size_t j=0;j<bt.fields.size();j++) {
	if (bt.fields[j].first==field1 || bt.fields[j].first==field2) {
	  return true;
	}
      } 
      return false;
    }
  
    /** \brief Get field named \c field from entry \c bt
     */
    std::string &get_field(bibtex::BibTeXEntry &bt, std::string field) {
      for(size_t j=0;j<bt.fields.size();j++) {
	if (bt.fields[j].first==field) {
	  if (bt.fields[j].second.size()>0) {
	    return bt.fields[j].second[0];
	  } else {
	    O2SCL_ERR("Field found but value vector was empty.",
		      o2scl::exc_einval);
	  }
	}
      }
      O2SCL_ERR((((std::string)"Field ")+field+" not found.").c_str(),
		o2scl::exc_einval);
      return trans_latex[0];
    }

    /** \brief Get field named \c field from entry \c bt
     */
    std::vector<std::string> &get_field_list
      (bibtex::BibTeXEntry &bt, std::string field) {
      for(size_t j=0;j<bt.fields.size();j++) {
	if (bt.fields[j].first==field) {
	  return bt.fields[j].second;
	}
      }
      O2SCL_ERR("Field not found.",o2scl::exc_einval);
      return trans_latex;
    }

    /** \brief Convert tildes to spaces
     */
    void tilde_to_space(std::string &s) {
      for(size_t i=0;i<s.length();i++) {
	if (s[i]=='~') s[i]=' ';
      }
      return;
    }
  
    /** \brief Output an entry in HTML format
     */
    void output_html(std::ostream &os, bibtex::BibTeXEntry &bt) {
      std::string s=get_field(bt,"author");
      std::string s2=author_firstlast(s);
      tilde_to_space(s2);
      os << s2 << ", <em>"
	 << get_field(bt,"journal") << "</em> <b>"
	 << get_field(bt,"volume") << "</b> ("
	 << get_field(bt,"year") << ") "
	 << get_field(bt,"pages") << ".";
      return;
    }

    /** \brief Output an entry in LaTeX format
     */
    void output_latex(std::ostream &os, bibtex::BibTeXEntry &bt) {
      std::string s=get_field(bt,"author");
      std::string s2=author_firstlast(s);
      os << s2 << ", {\\i"
	 << get_field(bt,"journal") << "} {\\b "
	 << get_field(bt,"volume") << "} ("
	 << get_field(bt,"year") << ") "
	 << get_field(bt,"pages") << ".";
      return;
    }

    /** \brief Add an entry to the list
     */
    void add_entry(bibtex::BibTeXEntry &bt) {
      entries.push_back(bt);
      if (bt.key) sort.insert(make_pair(*bt.key,entries.size()-1));
      return;
    }

  };

}
 
#endif
