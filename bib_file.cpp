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
/** \file bib_file.cpp
    \brief Source file for \ref bib_file class
*/
#include "bib_file.h"
#include "hdf_bibtex.h"

#include "curses.h"

#include <o2scl/cli_readline.h>
#include <o2scl/string_conv.h>

using namespace std;
using namespace o2scl;
using namespace btmanip;

bib_file::bib_file() {
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
  remove_author_tildes=true;
  verbose=1;
      
  trans_latex.push_back("{\\'a}");
  trans_latex_alt.push_back("\\'{a}");
  trans_html.push_back("&aacute;");
  trans_uni.push_back("á");
      
  trans_latex.push_back("{\\'e}");
  trans_latex_alt.push_back("\\'{e}");
  trans_html.push_back("&eacute;");
  trans_uni.push_back("é");
      
  trans_latex.push_back("{\\'i}");
  trans_latex_alt.push_back("\\'{i}");
  trans_html.push_back("&iacute;");
  trans_uni.push_back("í");
      
  trans_latex.push_back("{\\'o}");
  trans_latex_alt.push_back("\\'{o}");
  trans_html.push_back("&oacute;");
  trans_uni.push_back("ó");
      
  trans_latex.push_back("{\\'s}");
  trans_latex_alt.push_back("\\'{s}");
  trans_html.push_back("&sacute;");
  trans_uni.push_back("ś");
      
  trans_latex.push_back("{\\'u}");
  trans_latex_alt.push_back("\\'{u}");
  trans_html.push_back("&uacute;");
  trans_uni.push_back("ú");
      
  trans_latex.push_back("{\\`a}");
  trans_latex_alt.push_back("\\`{a}");
  trans_html.push_back("&agrave;");
  trans_uni.push_back("à");
      
  trans_latex.push_back("{\\`e}");
  trans_latex_alt.push_back("\\`{e}");
  trans_html.push_back("&egrave;");
  trans_uni.push_back("è");
      
  trans_latex.push_back("{\\`i}");
  trans_latex_alt.push_back("\\`{i}");
  trans_html.push_back("&igrave;");
  trans_uni.push_back("ì");
      
  trans_latex.push_back("{\\`o}");
  trans_latex_alt.push_back("\\`{o}");
  trans_html.push_back("&ograve;");
  trans_uni.push_back("ò");
      
  trans_latex.push_back("{\\`u}");
  trans_latex_alt.push_back("\\`{u}");
  trans_html.push_back("&ugrave;");
  trans_uni.push_back("ù");
      
  trans_latex.push_back("{\\ua}");
  trans_latex_alt.push_back("\\u{a}");
  trans_html.push_back("&abreve;");
  trans_uni.push_back("ă");
      
  trans_latex.push_back("{\\ue}");
  trans_latex_alt.push_back("\\u{e}");
  trans_html.push_back("&ebreve;");
  trans_uni.push_back("ĕ");
      
  trans_latex.push_back("{\\ug}");
  trans_latex_alt.push_back("\\u{g}");
  trans_html.push_back("&gbreve;");
  trans_uni.push_back("ğ");
      
  trans_latex.push_back("{\\ui}");
  trans_latex_alt.push_back("\\u{i}");
  trans_html.push_back("&ibreve;");
  trans_uni.push_back("ĭ");
      
  trans_latex.push_back("{\\uo}");
  trans_latex_alt.push_back("\\u{o}");
  trans_html.push_back("&obreve;");
  trans_uni.push_back("ŏ");
      
  trans_latex.push_back("{\\uu}");
  trans_latex_alt.push_back("\\u{u}");
  trans_html.push_back("&ubreve;");
  trans_uni.push_back("ŭ");
      
  trans_latex.push_back("{\\\"a}");
  trans_latex_alt.push_back("\\\"{a}");
  trans_html.push_back("&auml;");
  trans_uni.push_back("ä");
      
  trans_latex.push_back("{\\\"e}");
  trans_latex_alt.push_back("\\\"{e}");
  trans_html.push_back("&euml;");
  trans_uni.push_back("ë");
      
  trans_latex.push_back("{\\\"i}");
  trans_latex_alt.push_back("\\\"{i}");
  trans_html.push_back("&iuml;");
  trans_uni.push_back("ï");
      
  trans_latex.push_back("{\\\"o}");
  trans_latex_alt.push_back("\\\"{o}");
  trans_html.push_back("&ouml;");
  trans_uni.push_back("ö");
      
  trans_latex.push_back("{\\\"u}");
  trans_latex_alt.push_back("\\\"{u}");
  trans_html.push_back("&uuml;");
  trans_uni.push_back("ü");
      
  trans_latex.push_back("{\\'A}");
  trans_latex_alt.push_back("\\'{A}");
  trans_html.push_back("&Aacute;");
  trans_uni.push_back("Á");
      
  trans_latex.push_back("{\\'E}");
  trans_latex_alt.push_back("\\'{E}");
  trans_html.push_back("&Eacute;");
  trans_uni.push_back("É");
      
  trans_latex.push_back("{\\'I}");
  trans_latex_alt.push_back("\\'{I}");
  trans_html.push_back("&Iacute;");
  trans_uni.push_back("Í");
      
  trans_latex.push_back("{\\'O}");
  trans_latex_alt.push_back("\\'{O}");
  trans_html.push_back("&Oacute;");
  trans_uni.push_back("Ó");
      
  trans_latex.push_back("{\\'U}");
  trans_latex_alt.push_back("\\'{U}");
  trans_html.push_back("&Uacute;");
  trans_uni.push_back("Ú");
      
  trans_latex.push_back("{\\`A}");
  trans_latex_alt.push_back("\\`{A}");
  trans_html.push_back("&Agrave;");
  trans_uni.push_back("À");
      
  trans_latex.push_back("{\\`E}");
  trans_latex_alt.push_back("\\`{E}");
  trans_html.push_back("&Egrave;");
  trans_uni.push_back("È");
      
  trans_latex.push_back("{\\`I}");
  trans_latex_alt.push_back("\\`{I}");
  trans_html.push_back("&Igrave;");
  trans_uni.push_back("Ì");
      
  trans_latex.push_back("{\\`O}");
  trans_latex_alt.push_back("\\`{O}");
  trans_html.push_back("&Ograve;");
  trans_uni.push_back("Ò");
      
  trans_latex.push_back("{\\`U}");
  trans_latex_alt.push_back("\\`{U}");
  trans_html.push_back("&Ugrave;");
  trans_uni.push_back("Ù");
      
  trans_latex.push_back("{\\uA}");
  trans_latex_alt.push_back("\\u{A}");
  trans_html.push_back("&Abreve;");
  trans_uni.push_back("Ă");
      
  trans_latex.push_back("{\\uE}");
  trans_latex_alt.push_back("\\u{E}");
  trans_html.push_back("&Ebreve;");
  trans_uni.push_back("Ĕ");
      
  trans_latex.push_back("{\\uI}");
  trans_latex_alt.push_back("\\u{I}");
  trans_html.push_back("&Ibreve;");
  trans_uni.push_back("Ĭ");
      
  trans_latex.push_back("{\\uO}");
  trans_latex_alt.push_back("\\u{O}");
  trans_html.push_back("&Obreve;");
  trans_uni.push_back("Ŏ");
      
  trans_latex.push_back("{\\uU}");
  trans_latex_alt.push_back("\\u{U}");
  trans_html.push_back("&Ubreve;");
  trans_uni.push_back("Ŭ");
      
  trans_latex.push_back("{\\\"A}");
  trans_latex_alt.push_back("\\\"{A}");
  trans_html.push_back("&Auml;");
  trans_uni.push_back("Ä");
      
  trans_latex.push_back("{\\\"E}");
  trans_latex_alt.push_back("\\\"{E}");
  trans_html.push_back("&Euml;");
  trans_uni.push_back("Ë");
      
  trans_latex.push_back("{\\\"I}");
  trans_latex_alt.push_back("\\\"{I}");
  trans_html.push_back("&Iuml;");
  trans_uni.push_back("Ï");
      
  trans_latex.push_back("{\\\"O}");
  trans_latex_alt.push_back("\\\"{O}");
  trans_html.push_back("&Ouml;");
  trans_uni.push_back("Ö");
      
  trans_latex.push_back("{\\\"U}");
  trans_latex_alt.push_back("\\\"{U}");
  trans_html.push_back("&Uuml;");
  trans_uni.push_back("Ü");

  spec_chars=sc_allow_all;
      
  remove_fields={"adsnote","date-added","annote","bdsk-url-1",
		 "bdsk-url-2","date-modified","archiveprefix",
		 "primaryclass","abstract"};
  months_long={"January","February","March","April","May","June",
	       "July","August","September","October","November",
	       "December"};
  months_short={"Jan","Feb","Mar","Apr","May","Jun",
		"Jul","Aug","Sep","Oct","Nov",
		"Dec"};
  months_four={"Jan.","Feb.","Mar.","Apr.","May.","June",
	       "July","Aug.","Sep.","Oct.","Nov.",
	       "Dec."};
}

std::string bib_file::lower_string(std::string s) {
  for(size_t i=0;i<s.size();i++) {
    s[i]=std::tolower(s[i]);
  }
  return s;
}

int bib_file::read_journals(std::string fname) {
  if (journals.size()>0) {
    journals.clear();
  }
      
  wordexp_single_file(fname);
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

void bib_file::thin_whitespace(std::string &s) {
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

std::string bib_file::journal_simplify(std::string s) {
  for(size_t i=0;i<s.size();i++) {
    if (!isalpha(s[i])) {
      s=s.substr(0,i)+s.substr(i+1,s.length()-(i+1));
      i=0;
    }
  }
  s=lower_string(s);
  return s;
}

int bib_file::find_abbrev(std::string jour, std::string &abbrev) {
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

int bib_file::find_abbrevs(std::string jour, std::vector<std::string> &list) {
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
    
std::string bib_file::first_page(std::string pages) {
  size_t loc=pages.find('-');
  std::string pages2=pages;
  if (loc!=std::string::npos) {
    pages2=pages.substr(0,loc);
  }
  return pages2;
}

void bib_file::search_keys(std::string pattern,
			   std::vector<std::string> &list) {
  list.clear();
  for(size_t i=0;i<entries.size();i++) {
    bibtex::BibTeXEntry &bt=entries[i];
    if (fnmatch(pattern.c_str(),(*bt.key).c_str(),0)==0) {
      list.push_back(*bt.key);
    }
  }
  return;
}

void bib_file::search_or(std::vector<std::string> &args) {

  if (args.size()==0 || args.size()%2!=0) {
    O2SCL_ERR("Need a set of field and pattern pairs in search_or().",
	      o2scl::exc_einval);
  }
      
  std::vector<bibtex::BibTeXEntry> entries2;
      
  for(size_t i=0;i<entries.size();i++) {

    bool entry_matches=false;
    bibtex::BibTeXEntry &bt=entries[i];

    for(size_t k=0;k<args.size();k+=2) {
      std::string field=lower_string(args[k]);
      std::string pattern=args[k+1];
      if (field==((string)"key")) {
	if (fnmatch(pattern.c_str(),(*bt.key).c_str(),0)==0) {
	  entry_matches=true;
	}
      } else {
	for(size_t j=0;j<bt.fields.size();j++) {
	  std::string tmp=lower_string(bt.fields[j].first);
	  if (tmp==field &&
	      fnmatch(pattern.c_str(),bt.fields[j].second[0].c_str(),0)==0) {
	    entry_matches=true;
	  }
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
    
void bib_file::remove_or(std::vector<std::string> &args) {

  if (args.size()==0 || args.size()%2!=0) {
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
	std::string field=lower_string(args[k]);
	std::string pattern=args[k+1];
	for(size_t j=0;restart_loop==false && j<bt.fields.size();j++) {
	  std::string tmp=lower_string(bt.fields[j].first);
	  if (tmp==field &&
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

void bib_file::search_and(std::vector<std::string> &args) {

  if (args.size()==0 || args.size()%2!=0) {
    O2SCL_ERR("Need a set of field and pattern pairs in search_and().",
	      o2scl::exc_einval);
  }
      
  for(size_t k=0;k<args.size();k+=2) {

    std::string field=lower_string(args[k]);
    std::string pattern=args[k+1];
	
    std::vector<bibtex::BibTeXEntry> entries2;

    for(size_t i=0;i<entries.size();i++) {
      bool entry_matches=false;
      bibtex::BibTeXEntry &bt=entries[i];
      if (field==((string)"key")) {
	if (fnmatch(pattern.c_str(),(*bt.key).c_str(),0)==0) {
	  entry_matches=true;
	}
      } else {
	for(size_t j=0;j<bt.fields.size();j++) {
	  std::string tmp=lower_string(bt.fields[j].first);
	  if (tmp==field &&
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

void bib_file::entry_check_required(bibtex::BibTeXEntry &bt) {
  if (lower_string(bt.tag)==((std::string)"article")) {
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
  } else if (lower_string(bt.tag)==((std::string)"book")) {
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
  } else if (lower_string(bt.tag)==((std::string)"booklet")) {
    if (!is_field_present(bt,"title")) {
      O2SCL_ERR("Booklet missing title field.",
		o2scl::exc_einval);
    }
  } else if (lower_string(bt.tag)==((std::string)"conference")) {
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
  } else if (lower_string(bt.tag)==((std::string)"inbook")) {
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
  } else if (lower_string(bt.tag)==((std::string)"incollection")) {
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
  } else if (lower_string(bt.tag)==((std::string)"inproceedings")) {
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
  } else if (lower_string(bt.tag)==((std::string)"manual")) {
    if (!is_field_present(bt,"title")) {
      O2SCL_ERR("Manual missing title field.",
		o2scl::exc_einval);
    }
  } else if (lower_string(bt.tag)==((std::string)"mastersthesis")) {
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
    //} else if (lower_string(bt.tag)==((std::string)"Misc")) {
    // Misc has no required fields
  } else if (lower_string(bt.tag)==((std::string)"phdthesis")) {
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
  } else if (lower_string(bt.tag)==((std::string)"proceedings")) {
    if (!is_field_present(bt,"title")) {
      O2SCL_ERR("Proceedings missing title field.",
		o2scl::exc_einval);
    }
    if (!is_field_present(bt,"year")) {
      O2SCL_ERR("Proceedings missing year field.",
		o2scl::exc_einval);
    }
  } else if (lower_string(bt.tag)==((std::string)"techreport")) {
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
  } else if (lower_string(bt.tag)==((std::string)"unpublished")) {
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

bool bib_file::entry_add_empty_title(bibtex::BibTeXEntry &bt) {
      
  bool changed=false;
  if (lower_string(bt.tag)==((std::string)"article") ||
      lower_string(bt.tag)==((std::string)"inproceedings")) {
    if (!is_field_present(bt,"title")) {
      std::vector<std::string> val;
      val.push_back(" ");
      bt.fields.push_back(std::make_pair("title",val));
      changed=true;
      if (verbose>1) {
	std::cout << "In entry with key " << *bt.key
		  << " added empty title." << std::endl;
      }
    }
  }
  return changed;
}
    
bool bib_file::entry_autoformat_url(bibtex::BibTeXEntry &bt) {
  bool changed=false;
  if (lower_string(bt.tag)==((std::string)"article")) {
    if (is_field_present(bt,"doi")) {
      if (is_field_present(bt,"url")) {
	std::string &url=get_field(bt,"url");
	if (url.substr(0,15)!=((std::string)"https://doi.org")) {
	  url=((std::string)"https://doi.org/")+
	    get_field(bt,"doi");
	  changed=true;
	  if (verbose>1) {
	    std::cout << "In entry with key " << *bt.key
		      << " reformatted url to " << url << std::endl;
	  }
	}
      } else {
	std::vector<std::string> val;
	val.push_back(((std::string)"https://doi.org/")+
		      get_field(bt,"doi"));
	bt.fields.push_back(std::make_pair("url",val));
	changed=true;
	if (verbose>1) {
	  std::cout << "In entry with key " << *bt.key
		    << " added url field " << val[0] << std::endl;
	}
      }
    }
  } else if (lower_string(bt.tag)==((std::string)"book")) {
    if (is_field_present(bt,"isbn") && !is_field_present(bt,"url")) {
      std::vector<std::string> val;
      val.push_back(((std::string)"http://www.worldcat.org/isbn/")+
		    get_field(bt,"isbn"));
      bt.fields.push_back(std::make_pair("url",val));
      changed=true;
      if (verbose>1) {
	std::cout << "In entry with key " << *bt.key
		  << " added url field " << val[0] << std::endl;
      }
    }
  }
  return changed;
}
    
bool bib_file::entry_remove_vol_letters(bibtex::BibTeXEntry &bt) {
  bool changed=false;
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
      changed=true;
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
      changed=true;
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
      changed=true;
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
      changed=true;
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
  return changed;
}
    
void bib_file::clean(bool prompt) {

  size_t empty_titles_added=0;
  size_t entries_fields_removed=0;
  size_t journals_renamed=0;
  size_t urls_reformatted=0;
  size_t vol_letters_moved=0;
  size_t tags_normalized=0;
  size_t author_fields_notilde=0;
      
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
    std::cout << "remove_author_tildes: " << remove_author_tildes
	      << std::endl;
  }

  if (entries.size()==0) {
    std::cout << "No entries to clean." << std::endl;
  }

  std::vector<bool> entry_changed(entries.size());
      
  // Reformat if necessary. This loop is over each entry
  for(size_t i=0;i<entries.size();i++) {
    entry_changed[i]=false;
	
    bool this_empty_titles_added=false;
    bool this_entries_fields_removed=false;
    bool this_journals_renamed=false;
    bool this_urls_reformatted=false;
    bool this_vol_letters_moved=false;
    bool this_tags_normalized=false;
    bool this_author_fields_notilde=false;

    // Make a copy
    bibtex::BibTeXEntry bt=entries[i];

    if (normalize_tags) {
	  
      std::string old_tag=bt.tag;
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
      if (bt.tag!=old_tag) {
	entry_changed[i]=true;
	this_tags_normalized=true;
      }
    }

    if (remove_author_tildes && is_field_present(bt,"author")) {
      std::string old_auth=get_field(bt,"author"), auth=old_auth;
      tilde_to_space(auth);
      if (auth!=old_auth) {
	set_field_value(bt,"author",auth);
	entry_changed[i]=true;
	this_author_fields_notilde=true;
      }
    }
	
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
	    entry_changed[i]=true;
	  }
	}

	// Remove extra braces from each value
	std::string &valtemp=bt.fields[j].second[0];
	bool removed_verb=false;
	while (valtemp.length()>=4 && valtemp[0]=='{' &&
	       valtemp[1]=='{' && valtemp[valtemp.size()-1]=='}' &&
	       valtemp[valtemp.size()-2]=='}') {
	  valtemp=valtemp.substr(1,valtemp.size()-2);
	  entry_changed[i]=true;
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
	    entry_changed[i]=true;
	  }
	}

	if (field_removed) {
	  this_entries_fields_removed=true;
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
	      std::string old=bt.fields[j].second[k];
	      thin_whitespace(bt.fields[j].second[k]);
	      if (bt.fields[j].second[k]!=old) {
		entry_changed[i]=true;
	      }
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
		// Avoid changing arxiv entries in journal fields
		if (jour!=abbrev && abbrev!=((string)"Arxiv.org")) {
		  if (verbose>1) {
		    std::cout << "Reformatting journal " << jour << " to "
			      << abbrev << std::endl;
		  }
		  bt.fields[j].second[0]=abbrev;
		  this_journals_renamed=true;
		  entry_changed[i]=true;
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
      if (entry_remove_vol_letters(bt)) {
	entry_changed[i]=true;
	this_vol_letters_moved=true;
      }
    }

    // If necessary, create an article URL from the
    // DOI entry
    if (autoformat_urls) {
      if (entry_autoformat_url(bt)) {
	entry_changed[i]=true;
	this_urls_reformatted=true;
      }
    }
		  
    // Add empty title to an article if necessary
    if (add_empty_titles) {
      if (entry_add_empty_title(bt)) {
	entry_changed[i]=true;
	this_empty_titles_added=true;
      }
    }

    // If requested, check that required fields are present
    // for each entry
    if (normalize_tags && lowercase_fields && check_required) {
      entry_check_required(bt);
    }

    if (entry_changed[i]==true) {
      bool accept=false;
      if (prompt) {
	char ch;
	do {
	  std::cout << "\nChanging " << i << " of " << entries.size()
		    << "\n" << std::endl;
	  
	  //bib_output_one(std::cout,entries[i]);
	  //std::cout << "\nto\n" << std::endl;
	  //bib_output_one(std::cout,bt);
	  //std::cout << std::endl;

	  bib_output_twoup(std::cout,entries[i],bt,
			   "Original entry","Proposed new entry");
	  
	  if (this_empty_titles_added) {
	    std::cout << "Empty title added." << std::endl;
	  }
	  if (this_entries_fields_removed) {
	    std::cout << "Some fields removed." << std::endl;
	  }
	  if (this_journals_renamed) {
	    std::cout << "Journal renamed." << std::endl;
	  }
	  if (this_urls_reformatted) {
	    std::cout << "URL reformatted." << std::endl;
	  }
	  if (this_vol_letters_moved) {
	    std::cout << "Volume letter moved." << std::endl;
	  }
	  if (this_tags_normalized) {
	    std::cout << "Tag name recapitalized." << std::endl;
	  }
	  if (this_author_fields_notilde) {
	    std::cout << "Removed tildes from author names." << std::endl;
	  }
	  std::cout << "\nYes (y), no (n), yes to all remaining changes (Y), "
		    << "no to all remaining changes (N), "
		    << "or (s) to stop? ";
	  std::cin >> ch;
	  if (ch=='y') {
	    accept=true;
	  }
	  if (ch=='Y') prompt=false;
	  if (ch=='n' || ch=='N') {
	    entry_changed[i]=false;
	  }
	  if (ch=='N' || ch=='s') {
	    i=entries.size();
	  }
	} while (ch!='n' && ch!='N' && ch!='y' && ch!='Y' && ch!='s');
      } else {
	accept=true;
      }
      if (accept) {
	entries[i]=bt;
	
	//std::cout << "Tag: " << entries[i].tag << std::endl;
	//std::cout << "Author: " << get_field(entries[i],"author")
	//<< std::endl;
	
	if (this_empty_titles_added) {
	  empty_titles_added++;
	}
	if (this_entries_fields_removed) {
	  entries_fields_removed++;
	}
	if (this_journals_renamed) {
	  journals_renamed++;
	}
	if (this_urls_reformatted) {
	  urls_reformatted++;
	}
	if (this_vol_letters_moved) {
	  vol_letters_moved++;
	}
	if (this_tags_normalized) {
	  tags_normalized++;
	}
	if (this_author_fields_notilde) {
	  author_fields_notilde++;
	}
      }
    }
	
    // End of loop over entries
  }

  if (verbose>0) {
    // Count entries that changed
    size_t nch=0;
    for(size_t k=0;k<entries.size();k++) {
      if (entry_changed[k]) nch++;
    }
    std::cout << nch << " entries changed out of " << entries.size()
	      << std::endl;
    if (normalize_tags) {
      std::cout << tags_normalized << " tags normalized."
		<< std::endl;
    }
    if (add_empty_titles) {
      std::cout << empty_titles_added << " emtpy titles added."
		<< std::endl;
    }
    std::cout << entries_fields_removed
	      << " entries with extra fields removed." << std::endl;
    if (reformat_journal) {
      std::cout << journals_renamed << " journal names standardized."
		<< std::endl;
    }
    if (autoformat_urls) {
      std::cout << urls_reformatted << " URLs reformatted."
		<< std::endl;
    }
    if (remove_vol_letters) {
      std::cout << vol_letters_moved << " volume letters moved."
		<< std::endl;
    }
    if (remove_author_tildes) {
      std::cout << author_fields_notilde << " author fields cleaned "
		<< "of tildes." << std::endl;
    }
  }
      
  return;
}

int bib_file::set_field_value(bibtex::BibTeXEntry &bt, std::string field,
			      std::string value) {
  for(size_t j=0;j<bt.fields.size();j++) {
    if (bt.fields[j].first==field) {
      bt.fields[j].second[0]=value;
      return 0;
    }
  }
  std::vector<std::string> list={value};
  // If the field is not found, then add it
  bt.fields.push_back(std::make_pair(field,list));
      
  return 0;
}
    
int bib_file::set_field_value(std::string key, std::string field,
			      std::string value) {
  bibtex::BibTeXEntry &bt=get_entry_by_key(key);
  return set_field_value(bt,field,value);
}
    
void bib_file::parse_bib(std::string fname) {

  // Erase current entries
  if (entries.size()>0) {
    entries.clear();
    sort.clear();
  }
      
  // Main parse call
  if (verbose>1) std::cout << "Main parse call." << std::endl;
  wordexp_single_file(fname);
  std::ifstream in(fname.c_str());
  if (!in) {
    std::cerr << "File open failed. Wrong filename?" << std::endl;
    return;
  }
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
    
void bib_file::refresh_sort() {
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
    
void bib_file::sort_bib() {

  if (entries.size()!=sort.size()) {
    O2SCL_ERR("Cannot sort when two entries have the same key.",
	      o2scl::exc_efailed);
  }
      
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

void bib_file::sort_by_date(bool descending) {

  if (descending) {
    
    std::map<int,size_t,std::greater<int>> sbd;
    std::map<int,size_t,std::greater<int>>::iterator sbdit;
  
    for(size_t i=0;i<entries.size();i++) {
      bibtex::BibTeXEntry &bt=entries[i];
      int year=3000;
      if (is_field_present(bt,"year")) {
	year=std::stoi(get_field(bt,"year"));
      }
      int imonth=13;
      if (is_field_present(bt,"month")) {
	std::string month=get_field(bt,"month");
	if (month.length()==1) {
	  imonth=std::stoi(month);
	} else if (month.length()==2 && month[0]=='1') {
	  imonth=std::stoi(month);
	} else if (month.length()>=3) {
	  if ((month[0]=='j' || month[0]=='J') &&
	      (month[0]=='a' || month[0]=='A') &&
	      (month[0]=='n' || month[0]=='N')) {
	    imonth=1;
	  } else if ((month[0]=='f' || month[0]=='F') &&
		     (month[0]=='e' || month[0]=='E') &&
		     (month[0]=='b' || month[0]=='B')) {
	    imonth=2;
	  } else if ((month[0]=='m' || month[0]=='M') &&
		     (month[0]=='a' || month[0]=='A') &&
		     (month[0]=='r' || month[0]=='R')) {
	    imonth=3;
	  } else if ((month[0]=='a' || month[0]=='A') &&
		     (month[0]=='p' || month[0]=='P') &&
		     (month[0]=='r' || month[0]=='R')) {
	    imonth=4;
	  } else if ((month[0]=='m' || month[0]=='M') &&
		     (month[0]=='a' || month[0]=='A') &&
		     (month[0]=='y' || month[0]=='Y')) {
	    imonth=5;
	  } else if ((month[0]=='j' || month[0]=='J') &&
		     (month[0]=='u' || month[0]=='U') &&
		     (month[0]=='n' || month[0]=='N')) {
	    imonth=6;
	  } else if ((month[0]=='j' || month[0]=='J') &&
		     (month[0]=='u' || month[0]=='U') &&
		     (month[0]=='l' || month[0]=='L')) {
	    imonth=7;
	  } else if ((month[0]=='a' || month[0]=='A') &&
		     (month[0]=='u' || month[0]=='U') &&
		     (month[0]=='g' || month[0]=='G')) {
	    imonth=8;
	  } else if ((month[0]=='s' || month[0]=='S') &&
		     (month[0]=='e' || month[0]=='E') &&
		     (month[0]=='p' || month[0]=='P')) {
	    imonth=9;
	  } else if ((month[0]=='o' || month[0]=='O') &&
		     (month[0]=='c' || month[0]=='C') &&
		     (month[0]=='t' || month[0]=='T')) {
	    imonth=10;
	  } else if ((month[0]=='n' || month[0]=='N') &&
		     (month[0]=='o' || month[0]=='O') &&
		     (month[0]=='v' || month[0]=='V')) {
	    imonth=11;
	  } else if ((month[0]=='d' || month[0]=='D') &&
		     (month[0]=='e' || month[0]=='E') &&
		     (month[0]=='c' || month[0]=='C')) {
	    imonth=12;
	  }
	}
      }
      int date=40;
      if (is_field_present(bt,"date")) {
	date=std::stoi(get_field(bt,"date"));
      }
      int sortable_date=year*10000+imonth*100+date;
      sbd.insert(make_pair(sortable_date,i));
      cout << "Here2: " << sortable_date << " " << i << endl;
    }
  
    std::vector<bibtex::BibTeXEntry> entries2;
    for(sbdit=sbd.begin();sbdit!=sbd.end();sbdit++) {
      entries2.push_back(entries[sbdit->second]);
    }
    
  } else {

    std::map<int,size_t,std::less<int>> sbd;
    std::map<int,size_t,std::less<int>>::iterator sbdit;
  
    for(size_t i=0;i<entries.size();i++) {
      bibtex::BibTeXEntry &bt=entries[i];
      int year=3000;
      if (is_field_present(bt,"year")) {
	year=std::stoi(get_field(bt,"year"));
      }
      int imonth=13;
      if (is_field_present(bt,"month")) {
	std::string month=get_field(bt,"month");
	if (month.length()==1) {
	  imonth=std::stoi(month);
	} else if (month.length()==2 && month[0]=='1') {
	  imonth=std::stoi(month);
	} else if (month.length()>=3) {
	  if ((month[0]=='j' || month[0]=='J') &&
	      (month[0]=='a' || month[0]=='A') &&
	      (month[0]=='n' || month[0]=='N')) {
	    imonth=1;
	  } else if ((month[0]=='f' || month[0]=='F') &&
		     (month[0]=='e' || month[0]=='E') &&
		     (month[0]=='b' || month[0]=='B')) {
	    imonth=2;
	  } else if ((month[0]=='m' || month[0]=='M') &&
		     (month[0]=='a' || month[0]=='A') &&
		     (month[0]=='r' || month[0]=='R')) {
	    imonth=3;
	  } else if ((month[0]=='a' || month[0]=='A') &&
		     (month[0]=='p' || month[0]=='P') &&
		     (month[0]=='r' || month[0]=='R')) {
	    imonth=4;
	  } else if ((month[0]=='m' || month[0]=='M') &&
		     (month[0]=='a' || month[0]=='A') &&
		     (month[0]=='y' || month[0]=='Y')) {
	    imonth=5;
	  } else if ((month[0]=='j' || month[0]=='J') &&
		     (month[0]=='u' || month[0]=='U') &&
		     (month[0]=='n' || month[0]=='N')) {
	    imonth=6;
	  } else if ((month[0]=='j' || month[0]=='J') &&
		     (month[0]=='u' || month[0]=='U') &&
		     (month[0]=='l' || month[0]=='L')) {
	    imonth=7;
	  } else if ((month[0]=='a' || month[0]=='A') &&
		     (month[0]=='u' || month[0]=='U') &&
		     (month[0]=='g' || month[0]=='G')) {
	    imonth=8;
	  } else if ((month[0]=='s' || month[0]=='S') &&
		     (month[0]=='e' || month[0]=='E') &&
		     (month[0]=='p' || month[0]=='P')) {
	    imonth=9;
	  } else if ((month[0]=='o' || month[0]=='O') &&
		     (month[0]=='c' || month[0]=='C') &&
		     (month[0]=='t' || month[0]=='T')) {
	    imonth=10;
	  } else if ((month[0]=='n' || month[0]=='N') &&
		     (month[0]=='o' || month[0]=='O') &&
		     (month[0]=='v' || month[0]=='V')) {
	    imonth=11;
	  } else if ((month[0]=='d' || month[0]=='D') &&
		     (month[0]=='e' || month[0]=='E') &&
		     (month[0]=='c' || month[0]=='C')) {
	    imonth=12;
	  }
	}
      }
      int date=40;
      if (is_field_present(bt,"date")) {
	date=std::stoi(get_field(bt,"date"));
      }
      int sortable_date=year*10000+imonth*100+date;
      sbd.insert(make_pair(sortable_date,i));
      cout << "Here: " << sortable_date << " " << i << endl;
    }
  
    std::vector<bibtex::BibTeXEntry> entries2;
    for(sbdit=sbd.begin();sbdit!=sbd.end();sbdit++) {
      entries2.push_back(entries[sbdit->second]);
    }

    std::swap(entries,entries2);
    refresh_sort();
    
  }
  
  return;
}

void bib_file::reverse_bib() {

  std::vector<bibtex::BibTeXEntry> entries2(entries.size());
  for(size_t j=0;j<entries.size();j++) {
    entries2[entries.size()-1-j]=entries[j];
  }
  std::swap(entries,entries2);
      
  return;
}
    
void bib_file::bib_output_one(std::ostream &outs, bibtex::BibTeXEntry &bt) {

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

void bib_file::fill(std::string &s, size_t len, char ch) {
  for(size_t i=s.length();i<len;i++) {
    s+=ch;
  }
  return;
}

void bib_file::local_wrap(std::vector<std::string> &sv, size_t len) {

  std::vector<std::string> sv2, sv3;
  
  for(std::vector<std::string>::iterator it=sv.begin();it!=sv.end();it++) {
    string stmp=*it;
    //cout << "Here2: " << stmp << " " << stmp.length() << endl;
    if (stmp.length()<=len) {
      sv2.push_back(stmp);
    } else {
      //cout << "Here4: " << stmp << endl;
      bool done=false;
      while (stmp.length()>=len && done==false) {
	bool found=false;
	int k2;
	for(int k=len-1;k>=0;k--) {
	  if (stmp[k]==' ') {
	    found=true;
	    k2=k;
	    k=-1;
	  }
	}
	if (found==false) {
	  done=true;
	} else {
	  sv3.push_back(stmp.substr(0,k2+1));
	  stmp=stmp.substr(k2+1,stmp.length()-k2-1);
	}
      }
      sv3.push_back(stmp);
      //for(size_t j=0;j<sv3.size();j++) {
      //cout << "sv3: " << j << " " << sv3[j] << endl;
      //}
      for(size_t j=0;j<sv3.size();j++) {
	sv2.push_back(sv3[j]);
      }
    }
  }
  sv=sv2;
  
  return;
}

void bib_file::format_and_output(std::string left, std::string right,
				 std::ostream &outs, int highlight,
				 std::string sep, size_t len) {

  // First, take the value and wrap it 
  vector<string> vs_left={left};
  vector<string> vs_right={right};
  local_wrap(vs_left,len-16);
  local_wrap(vs_right,len-16);

  // Pad with spaces to balance left vs. right
  while (vs_left.size()<vs_right.size()) vs_left.push_back(" ");
  while (vs_left.size()>vs_right.size()) vs_right.push_back(" ");

  /*
    if (vs_left.size()>1) {
    cout << "Iere: " << endl;
    for(size_t j=0;j<vs_left.size();j++) {
    cout << "left: " << j << " " << vs_left[j] << endl;
    }
    for(size_t j=0;j<vs_right.size();j++) {
    cout << "right: " << j << " " << vs_right[j] << endl;
    }
    cout << "Iere2: " << endl;
    }
  */

  // For the first line, pad with spaces to fill up to the
  // full line length
  fill(vs_left[0],len);
  fill(vs_right[0],len);
  
  //cout << "2: " << vs_left[0].length() << " "
  //<< vs_right[0].length() << " " << len << endl;

  if (highlight==-1 || highlight==2) {
    vs_left[0]=vt100_cyan_fg()+vs_left[0]+vt100_default();
  }
  if (highlight==1 || highlight==2) {
    vs_right[0]=vt100_cyan_fg()+vs_right[0]+vt100_default();
  }
  outs << vs_left[0] << sep << vs_right[0] << endl;
  /*
    cout << "4: " << vs_left[0].length() << " " << sep.length() << " "
    << vs_right[0].length() << endl;
    for(size_t j=0;j<vs_left[0].length();j++) {
    cout << ((int)vs_left[0][j]) << " ";
    }
    cout << endl;
    for(size_t j=0;j<vs_right[0].length();j++) {
    cout << ((int)vs_right[0][j]) << " ";
    }
    cout << endl;
  */
  
  for(size_t j=1;j<vs_left.size();j++) {

    // For the wrapped lines, pad with spaces up to the
    // full line minus the LHS margin
    fill(vs_left[j],len-16);
    fill(vs_right[j],len-16);
    
    //cout << "3: " << highlight << " " << vs_left[j].length() << " "
    //<< vs_right[j].length() << " " << len-16 << endl;

    // Setup highlighting
    if (highlight==-1 || highlight==2) {
      vs_left[j]=vt100_cyan_fg()+vs_left[j]+vt100_default();
    }
    if (highlight==1 || highlight==2) {
      vs_right[j]=vt100_cyan_fg()+vs_right[j]+vt100_default();
    }

    // Perform the output
    for(size_t k=0;k<16;k++) outs << ' ';
    outs << vs_left[j] << sep;
    for(size_t k=0;k<16;k++) outs << ' ';
    outs << vs_right[j] << endl;
    
    //cout << "3: " << vs_left[j].length() << " "
    //<< vs_right[j].length() << endl;
  }

  return;
}

void bib_file::format_field_value(std::string field, std::string value,
				  std::string &outs) {

  // If the field name is too long, then shorten it
  if (field.length()>=13) {
    field=field.substr(0,9)+"...";
  }

  // Begin the string with the field and equals sign
  outs=((std::string)"  ")+field+" =";
  while (outs.length()<16) outs+=" ";
  
  // Determine if the field value needs extra braces
  bool with_braces=true;
  if (field=="year") {
    with_braces=false;
  }
  if (value[0]=='{' &&
      value[value.size()-1]=='}' &&
      value.find('{',1)==std::string::npos) {
    with_braces=false;
  }
  
  // Don't surround purely numeric values with braces
  // unless they begin with a '0'. 
  if ((field=="pages" || field=="numpages" ||
       field=="volume" || field=="issue" ||
       field=="isbn" || field=="citations" ||
       field=="adscites" || field=="number") &&
      value.size()>0 && value[0]!='0') {
    
    bool has_nonnum=false;
    for(size_t i=0;i<value.size();i++) {
      if (!isdigit(value[i])) {
	has_nonnum=true;
      }
    }
    if (has_nonnum==false) {
      with_braces=false;
    }
  }

  // Iterate through the value, if it has a carriage
  // return, then replace it with a space
  for(size_t k=0;k<value.length();k++) {
    if (value[k]=='\n') {
      string stmp=value.substr(0,k)+' ';
      stmp+=value.substr(k+1,value.length()-k-1);
      value=stmp;
      k=0;
    }
  }

  // Iterate through the value, if it has two consecutive spaces
  // then replace it with one space
  if (value.length()>1) {
    for(size_t k=0;k<value.length()-1;k++) {
      if (value[k]==' ' && value[k+1]==' ') {
	string stmp=value.substr(0,k);
	stmp+=value.substr(k+2,value.length()-k-2);
	value=stmp;
	k=0;
      }
    }
  }
  
  // Output with braces or a comma as necessary
  if (with_braces==false) {
    value=value+",";
  } else {
    value=((string)"{")+value+"},";
  }
  outs+=value;

  return;
}

void bib_file::bib_output_twoup(std::ostream &outs,
				bibtex::BibTeXEntry &bt_left,
				bibtex::BibTeXEntry &bt_right,
				std::string left_header,
				std::string right_header) {

  string stmpl, stmpr;

  bool key_match=false;
  if (bt_left.key && bt_right.key && *bt_left.key==*bt_right.key) {
    key_match=true;
  }
  
  // Print out header
  string stmp=left_header+" ( matching ";
  stmp+=vt100_cyan_fg();
  stmp+="different";
  stmp+=vt100_default();
  stmp+=" )";
  // 78 for LHS and 12 for vt100
  if (stmp.length()>86) stmp=stmp.substr(0,87)+"...";
  while (stmp.length()<86) stmp+=' ';
  stmp+=" | "+right_header;
  // 78 for LHS, 12 for vt100, 3 for separator, and 78 for RHS
  // is a total of 171
  if (stmp.length()>171) stmp=stmp.substr(0,168)+"...";
  cout << stmp << endl;
  
  // Print out line separator
  stmpl='-';
  stmpr='-';
  for(size_t k=0;k<77;k++) {
    stmpl+='-';
    stmpr+='-';
  }
  format_and_output(stmpl,stmpr,std::cout);
  
  // Output tag and key
  stmpl=((string)"@")+bt_left.tag+"{";
  stmpl+=(*bt_left.key);
  stmpl+=',';
  stmpr=((string)"@")+bt_right.tag+"{";
  stmpr+=(*bt_right.key);
  stmpr+=',';
  if (key_match) {
    format_and_output(stmpl,stmpr,outs);
  } else {
    // Highlight if the keys don't match
    format_and_output(stmpl,stmpr,outs,2);
  }

  // Loop through all fields on the LHS
  for(size_t j=0;j<bt_left.fields.size();j++) {

    stmpr="";
    
    bool fields_match=false;

    // If the value in the field is not empty, construct the string
    // stmpl from the value in the field
    if (bt_left.fields[j].second.size()>0) {
      format_field_value(bt_left.fields[j].first,
			 bt_left.fields[j].second[0],stmpl);
    }

    // If this field is present on the RHS
    if (is_field_present(bt_right,bt_left.fields[j].first)) {

      // Get the value
      string rx=get_field(bt_right,bt_left.fields[j].first);

      // If it's not empty, then fill the string stmpr
      if (rx.size()>0) {
	format_field_value(bt_left.fields[j].first,rx,stmpr);
      }

      if (bt_left.fields[j].second[0]==rx) {
	fields_match=true;
      }
      
    }

    if (fields_match) {
      format_and_output(stmpl,stmpr,outs);
    } else {
      format_and_output(stmpl,stmpr,outs,2);
    }
    
  }

  stmpl="";

  // Now loop through all the extra fields in bt_right which are
  // not present on the left
  for(size_t j=0;j<bt_right.fields.size();j++) {
    if (!is_field_present(bt_left,bt_right.fields[j].first)) {

      stmpr=((std::string)"  ")+bt_right.fields[j].first+" =";
      while (stmpr.length()<16) stmpr+=" ";
      
      if (bt_right.fields[j].second.size()>0) {
	format_field_value(bt_right.fields[j].first,
			   bt_right.fields[j].second[0],stmpr);
      }
      
      format_and_output(stmpl,stmpr,outs);
    }
  }

  stmpl="}";
  stmpr="}";
  format_and_output(stmpl,stmpr,outs);

  return;
}

void bib_file::ident_or_addl_fields(bibtex::BibTeXEntry &bt_left,
				    bibtex::BibTeXEntry &bt_right,
				    int &result) {

  // If the keys are not present or different, then presume the
  // entries are different
  if (!bt_left.key || !bt_right.key || *bt_left.key!=*bt_right.key) {
    result=ia_diff;
    return;
  }

  // Presume the entries are identical for now  
  result=ia_ident;
  
  // Loop through all fields on the LHS
  for(size_t j=0;j<bt_left.fields.size();j++) {

    // If this field is present on the RHS, then check it
    if (is_field_present(bt_right,bt_left.fields[j].first)) {
      string rx=get_field(bt_right,bt_left.fields[j].first);
      // If the values are not equal, then exit, indicating they
      // are different
      if (bt_left.fields[j].second[0]!=rx) {
	result=ia_diff;
	return;
      }
    } else {
      // If the field is not present on the RHS
      result=ia_addl_fields;
    }
  }

  
  // Loop through all fields on the RHS
  for(size_t j=0;j<bt_right.fields.size();j++) {

    // If this field not present on the LHS
    if (!is_field_present(bt_left,bt_right.fields[j].first)) {

      // If the field is not present on the RHS
      result=ia_addl_fields;
      
    }
  }

  return;
}

void bib_file::merge_to_left(bibtex::BibTeXEntry &bt_left,
			     bibtex::BibTeXEntry &bt_right) {

  // Loop through all fields on the RHS
  for(size_t j=0;j<bt_right.fields.size();j++) {

    // If this field not present on the LHS, then add it
    if (!is_field_present(bt_left,bt_right.fields[j].first)) {
      set_field_value(bt_left,bt_right.fields[j].first,
		      bt_right.fields[j].second[0]);
    }
  }

  return;
}

int bib_file::possible_duplicate(bibtex::BibTeXEntry &bt,
				 bibtex::BibTeXEntry &bt2) {
  std::string lower_tag1=bt.tag, lower_tag2=bt2.tag;
  std::string lower_key1=*bt.key, lower_key2=*bt2.key;
  for (size_t k=0;k<lower_tag1.size();k++) {
    lower_tag1[k]=std::tolower(lower_tag1[k]);
  }
  for (size_t k=0;k<lower_tag2.size();k++) {
    lower_tag2[k]=std::tolower(lower_tag2[k]);
  }
  for (size_t k=0;k<lower_key1.size();k++) {
    lower_key1[k]=std::tolower(lower_key1[k]);
  }
  for (size_t k=0;k<lower_key2.size();k++) {
    lower_key2[k]=std::tolower(lower_key2[k]);
  }
  if (lower_tag1==lower_tag2 && lower_key1==lower_key2) {
    return 1;
  }
  // First, check to see if tag, journal, volume and first page all match
  if (lower_tag1==lower_tag2 &&
      is_field_present(bt,"volume") &&
      is_field_present(bt,"pages") &&
      is_field_present(bt2,"volume") &&
      is_field_present(bt2,"pages") &&
      get_field(bt,"volume")==get_field(bt2,"volume") &&
      first_page(get_field(bt,"pages"))==
      first_page(get_field(bt2,"pages"))) {
    // Then, check that journal fields are present
    if (is_field_present(bt,"journal") &&
	is_field_present(bt2,"journal")) {
      std::string j1=get_field(bt,"journal");
      std::string j2=get_field(bt2,"journal");
      // If we can, get the standard abbreviation for each
      if (journals.size()>0) {
	find_abbrev(j1,j1);
	find_abbrev(j2,j2);
      }
      // Finally, check journal
      if (j1==j2) {
	return 2;
      }
    }
  }
  return 0;
}

void bib_file::list_possible_duplicates(bibtex::BibTeXEntry &bt,
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
    
void bib_file::text_output_one(std::ostream &outs, bibtex::BibTeXEntry &bt) {
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
    
void bib_file::add_bib(std::string fname) {

  std::vector<bibtex::BibTeXEntry> entries2;

  // Main parse call
  if (verbose>1) std::cout << "Main parse call." << std::endl;
  wordexp_single_file(fname);
  std::ifstream in(fname.c_str());
  bibtex::read(in,entries2); 
  in.close();
  if (verbose>1) std::cout << "Done with main parse call." << std::endl;

  size_t n_orig=entries.size();
  size_t n_new=entries2.size();
  size_t n_add=0;
  size_t n_process=0;
  size_t n_mod=0;
  size_t n_ident=0;
  size_t n_auto=0;

  // Loop over entries
  for(size_t i=0;i<entries2.size();i++) {

    bibtex::BibTeXEntry &bt=entries2[i];

    std::vector<size_t> list;
    list_possible_duplicates(bt,list);

    if (list.size()==0) {
      
      // If the new entry does not have any apparent duplicates,
      // then just add it.
      
      n_add++;
      entries.push_back(bt);
      if (bt.key) {
	cout << "Directly added entry " << *bt.key << endl;
      }
      
    } else {

      bool auto_merge=false;
    
      if (list.size()==1) {    

	// If there is only one possible matching duplicate,
	// see if we can auto merge
	int result;
	ident_or_addl_fields(bt,entries[list[0]],result);
	
	if (result==ia_addl_fields) {
	  cout << "Quietly merging:" << endl;
	  bib_output_twoup(std::cout,entries[list[0]],bt,
			   ((string)"Entry ")+
			   o2scl::szttos(list[0])+" in current list",
			   ((string)"Entry ")+
			   o2scl::szttos(i)+" in "+fname);
	  merge_to_left(bt,entries[list[0]]);
	  n_auto++;
	  auto_merge=true;
	} else if (result==ia_ident) {
	  cout << "Identical:" << endl;
	  bib_output_twoup(std::cout,entries[list[0]],bt,
			   ((string)"Entry ")+
			   o2scl::szttos(list[0])+" in current list",
			   ((string)"Entry ")+
			   o2scl::szttos(i)+" in "+fname);
	  n_ident++;
	  auto_merge=true;
	}
	
      }

      if (auto_merge==false) {
	
	std::cout << "\n" << list.size() << " possible duplicates in the "
		  << "current list were found:\n" << std::endl;
	
	cout << n_orig << " original, " << n_new << " new, "
	     << n_add << " added, " << n_ident << " identical, "
	     << n_auto << " automatically added, " << n_mod
	     << " modified, and " << n_process << " processed." << endl;
      
	for(size_t j=0;j<list.size();j++) {
	  // Print out entry comparison
	  bib_output_twoup(std::cout,entries[list[j]],bt,
			   ((string)"Entry ")+
			   o2scl::szttos(list[j])+" in current list",
			   ((string)"Entry ")+
			   o2scl::szttos(i)+" in "+fname);
	}
      
	std::cout << "\nKeep entry on left (<,), replace with "
		  << "entry on right (>.), add entry enyway (a) "
		  << "or stop add (s)? " << std::endl;
	char ch;
	cin >> ch;
      
	if (ch=='a' || ch=='A') {
	  n_add++;
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
	} else if (list.size()==1 && (ch=='>' || ch=='.')) {
	  std::cout << "Replacing " << *(entries[list[0]].key)
		    << " with " << *bt.key << std::endl;
	  entries[list[0]]=bt;
	  n_mod++;
	} else if (ch=='<' || ch==',') {
	  std::cout << "Keeping old entry." << std::endl;
	} else if (ch=='S' || ch=='s') {
	  i=entries2.size();
	} else if (i>0 && (ch=='u' || ch=='U')) {
	  std::cout << "Creating file of unprocessed entries." << std::endl;
	  std::cout << "Give filename: " << std::flush;
	  string fname2;
	  std::cin >> fname2;
	  std::vector<bibtex::BibTeXEntry>::iterator first=entries2.begin();
	  std::vector<bibtex::BibTeXEntry>::iterator last=first+i;
	  entries2.erase(first,last);
	  cout << "entries2.size(): " << entries2.size() << endl;

	  ofstream fout;
	  fout.open(fname2);
	  for(size_t i=0;i<entries2.size();i++) {
	    bibtex::BibTeXEntry &bt=entries2[i];
	    bib_output_one(fout,bt);
	    if (i+1<entries2.size()) (fout) << endl;
	  }
	  fout.close();
	  
	  i=entries2.size();
	  
	} else {
	  std::cout << "Ignoring " << *bt.key << std::endl;
	}

      }

    }
    
    n_process++;
    // End of loop over entries
  }
      
  /* 
     This test will fail if there are multiple entries with
     the same key. We currently allow this, thus this 
     test must be removed. 
  */
      
  if (verbose>0) {
    cout << n_orig << " original, " << n_new << " new, "
	 << n_add << " added, " << n_ident << " identical, "
	 << n_auto << " automatically added, " << n_mod
	 << " modified, and " << n_process << " processed." << endl;
    
    std::cout << "Read " << entries2.size() << " entries from file \""
	      << fname << "\". Now " << entries.size() 
	      << " total entries with " << sort.size()
	      << " sortable entries." << std::endl;
  }

  return;
}

bool bib_file::is_key_present(std::string key) {
  if (sort.find(key)==sort.end()) return false;
  return true;
}

bibtex::BibTeXEntry &bib_file::get_entry_by_key(std::string key) {
  return entries[sort.find(key)->second];
}

void bib_file::change_key(std::string key1, std::string key2) {
  size_t ix=sort.find(key1)->second;
  if (sort.find(key2)!=sort.end()) {
    O2SCL_ERR("Key 2 already present in change_key().",
	      o2scl::exc_einval);
  }
  bibtex::BibTeXEntry bt=entries[ix];      
  entries.erase(entries.begin()+ix);
  *bt.key=key2;
  entries.push_back(bt);
  refresh_sort();
  return;
}
    
size_t bib_file::get_index_by_key(std::string key) {
  return sort.find(key)->second;
}

std::string bib_file::spec_char_to_latex(std::string s_in) {
  for(size_t i=0;i<trans_latex.size();i++) {
    if (s_in.find(trans_html[i])!=std::string::npos) {
      boost::replace_all(s_in,trans_html[i],trans_latex[i]);
    }
    if (s_in.find(trans_uni[i])!=std::string::npos) {
      boost::replace_all(s_in,trans_uni[i],trans_latex[i]);
    }
    if (s_in.find(trans_latex_alt[i])!=std::string::npos) {
      boost::replace_all(s_in,trans_latex_alt[i],trans_latex[i]);
    }
  }
  return s_in;
}

std::string bib_file::spec_char_to_html(std::string s_in) {
  for(size_t i=0;i<trans_latex.size();i++) {
    if (s_in.find(trans_latex[i])!=std::string::npos) {
      boost::replace_all(s_in,trans_latex[i],trans_html[i]);
    }
    if (s_in.find(trans_uni[i])!=std::string::npos) {
      boost::replace_all(s_in,trans_uni[i],trans_html[i]);
    }
    if (s_in.find(trans_latex_alt[i])!=std::string::npos) {
      boost::replace_all(s_in,trans_latex_alt[i],trans_html[i]);
    }
  }
  return s_in;
}

std::string bib_file::spec_char_to_uni(std::string s_in) {
  for(size_t i=0;i<trans_latex.size();i++) {
    if (s_in.find(trans_latex[i])!=std::string::npos) {
      boost::replace_all(s_in,trans_latex[i],trans_uni[i]);
    }
    if (s_in.find(trans_html[i])!=std::string::npos) {
      boost::replace_all(s_in,trans_html[i],trans_uni[i]);
    }
    if (s_in.find(trans_latex_alt[i])!=std::string::npos) {
      boost::replace_all(s_in,trans_latex_alt[i],trans_uni[i]);
    }
  }
  return s_in;
}

std::string bib_file::spec_char_auto(std::string s_in) {
  if (spec_chars==sc_force_unicode) {
    return spec_char_to_uni(s_in);
  } else if (spec_chars==sc_force_html) {
    return spec_char_to_html(s_in);
  } else if (spec_chars==sc_force_latex) {
    return spec_char_to_latex(s_in);
  }
  return s_in;
}
    
std::string bib_file::short_author(bibtex::BibTeXEntry &bt) {
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
    
std::string bib_file::last_name_first_author(bibtex::BibTeXEntry &bt) {
  std::string auth=get_field(bt,"author");
  std::vector<std::string> firstv, lastv;
  parse_author(auth,firstv,lastv);
  std::string ret=lastv[0];
  if (ret[0]=='{' && ret[ret.length()-1]=='}') {
    ret=ret.substr(1,ret.length()-2);
  }
  return ret;
}
    
void bib_file::parse_author(std::string s_in,
			    std::vector<std::string> &firstv, 
			    std::vector<std::string> &lastv,
			    bool remove_braces) {

  // Look for a comma
  size_t comma_loc=s_in.find(',');
  if (comma_loc!=std::string::npos) {

    // If a comma is found, assume "last, first and" notation
	
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

  } else {

    // Assume "first last and" notation
	
    std::istringstream *is=new std::istringstream(s_in.c_str());
    std::string stmp;
    std::string stmp2;
    (*is) >> stmp;
    firstv.push_back("");
    while ((*is) >> stmp2) {
      if (stmp2==(std::string)"and") {
	lastv.push_back(stmp);
	firstv.push_back("");
	stmp=stmp2;
	(*is) >> stmp2;
      } else {
	if (firstv[firstv.size()-1].length()>0) {
	  firstv[firstv.size()-1]+=((std::string)" ")+stmp;
	} else {
	  firstv[firstv.size()-1]=stmp;
	}
      }
      stmp=stmp2;
    }
    lastv.push_back(stmp);

  }

  // Remove extra curly braces from all last names
  if (remove_braces) {
    for(size_t k=0;k<lastv.size();k++) {
      if (lastv[k][0]=='{' and lastv[k][lastv[k].length()-1]=='}') {
	lastv[k]=lastv[k].substr(1,lastv[k].length()-2);
      }
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

std::string bib_file::author_firstlast(std::string s_in, 
				       bool remove_braces,
				       bool first_initial) {

  std::vector<std::string> firstv, lastv;

  parse_author(s_in,firstv,lastv,remove_braces);

  /*
    for(size_t k=0;k<lastv.size();k++) {
    firstv[k]=spec_char_auto(firstv[k]);
    lastv[k]=spec_char_auto(lastv[k]);
    }
  */

  // Convert first (and middle) names to first initials
  if (first_initial) {

    // Go through all of the first names in the list
    for(size_t k=0;k<firstv.size();k++) {
      // Go through each character in the name
      for(size_t j=0;j<firstv[k].length();j++) {
	// Look for an upper case character
	if (isupper(firstv[k][j])) {

	  // Count lowercase characters immediately after
	  size_t lower_count=0;
	  for(size_t jj=j+1;jj<firstv[k].length();jj++) {
	    if (islower(firstv[k][jj])) {
	      lower_count++;
	    } else {
	      jj=firstv[k].length();
	    }
	  }

	  // If there is at least one, then remove them
	  if (lower_count>0) {
	    std::string temp;
	    // Look for a dot, and add one if not already present
	    if (firstv[k][j+lower_count+1]=='.') {
	      temp=firstv[k].substr(0,j+1)+
		firstv[k].substr(j+1+lower_count,
				 firstv[k].length()-j-1-lower_count);
	    } else {
	      temp=firstv[k].substr(0,j+1)+'.'+
		firstv[k].substr(j+1+lower_count,
				 firstv[k].length()-j-1-lower_count);
	    }
	    firstv[k]=temp;

	    // Start over at the beginning of the first name
	    // string looking for lower case letters to remove
	    j=0;
	  }
	}
      }
    }
  }

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
    
bool bib_file::is_field_present(bibtex::BibTeXEntry &bt, std::string field) {
  for(size_t j=0;j<bt.fields.size();j++) {
    std::string lower=bt.fields[j].first;
    for(size_t k=0;k<lower.size();k++) {
      lower[k]=std::tolower(lower[k]);
    }
    for(size_t k=0;k<field.size();k++) {
      field[k]=std::tolower(field[k]);
    }
    if (lower==field && bt.fields[j].second.size()>0) {
      return true;
    }
  } 
  return false;
}

bool bib_file::is_field_present(bibtex::BibTeXEntry &bt, std::string field1,
				std::string field2) {
  for(size_t j=0;j<bt.fields.size();j++) {
    std::string lower=bt.fields[j].first;
    for(size_t k=0;k<lower.size();k++) {
      lower[k]=std::tolower(lower[k]);
    }
    for(size_t k=0;k<field1.size();k++) {
      field1[k]=std::tolower(field1[k]);
    }
    for(size_t k=0;k<field2.size();k++) {
      field2[k]=std::tolower(field2[k]);
    }
    if ((lower==field1 || lower==field2) && bt.fields[j].second.size()>0) {
      return true;
    }
  }
  return false;
}
  
std::string &bib_file::get_field(bibtex::BibTeXEntry &bt, std::string field) {
  for(size_t j=0;j<bt.fields.size();j++) {
    std::string lower=bt.fields[j].first;
    for(size_t k=0;k<lower.size();k++) {
      lower[k]=std::tolower(lower[k]);
    }
    for(size_t k=0;k<field.size();k++) {
      field[k]=std::tolower(field[k]);
    }
    if (lower==field) {
      if (bt.fields[j].second.size()>0) {
	return bt.fields[j].second[0];
      } else {
	O2SCL_ERR("Field found but value vector was empty.",
		  o2scl::exc_einval);
      }
    }
  }
  if (!bt.key) {
    O2SCL_ERR((((std::string)"Field ")+field+
	       " not found in entry with no key ").c_str(),
	      o2scl::exc_einval);
    return trans_latex[0];
  }
  O2SCL_ERR((((std::string)"Field ")+field+
	     " not found in entry with key "+(*bt.key).c_str()).c_str(),
	    o2scl::exc_einval);
  return trans_latex[0];
}

std::vector<std::string> &bib_file::get_field_list
(bibtex::BibTeXEntry &bt, std::string field) {
  for(size_t j=0;j<bt.fields.size();j++) {
    if (bt.fields[j].first==field) {
      return bt.fields[j].second;
    }
  }
  O2SCL_ERR("Field not found.",o2scl::exc_einval);
  return trans_latex;
}

void bib_file::tilde_to_space(std::string &s) {
  for(size_t i=0;i<s.length();i++) {
    if (s[i]=='~') s[i]=' ';
  }
  return;
}
  
void bib_file::output_html(std::ostream &os, bibtex::BibTeXEntry &bt) {
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

void bib_file::output_latex(std::ostream &os, bibtex::BibTeXEntry &bt) {
  std::string s=get_field(bt,"author");
  std::string s2=author_firstlast(s);
  os << s2 << ", {\\i"
     << get_field(bt,"journal") << "} {\\b "
     << get_field(bt,"volume") << "} ("
     << get_field(bt,"year") << ") "
     << get_field(bt,"pages") << ".";
  return;
}

void bib_file::add_entry(bibtex::BibTeXEntry &bt) {
  entries.push_back(bt);
  if (bt.key) sort.insert(make_pair(*bt.key,entries.size()-1));
  return;
}

