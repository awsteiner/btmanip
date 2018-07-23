/*
  -------------------------------------------------------------------

  Copyright (C) 2015-2018, Andrew W. Steiner

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

int bib_file::read_journals(std::string fname="") {
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
