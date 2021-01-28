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
/** \file bib_file.h
    \brief File defining \ref bib_file
*/
#ifndef BTMANIP_BIB_FILE_H
#define BTMANIP_BIB_FILE_H

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

  /** \brief A child bibtex entry object with some
      additional functionality
   */
  class bibtex_entry : public bibtex::BibTeXEntry {

  public:

    bibtex_entry() {
    }

    /** \brief Get field named \c field (case-insensitive)
     */
    std::string &get_field(std::string field) {

      // Convert user-specified field to lowercase
      for(size_t k=0;k<field.size();k++) {
        field[k]=std::tolower(field[k]);
      }
      
      for(size_t j=0;j<fields.size();j++) {

        std::string lower=fields[j].first;
        for(size_t k=0;k<lower.size();k++) {
          lower[k]=std::tolower(lower[k]);
        }
        
        if (lower==field) {
          if (fields[j].second.size()==1) {
            return fields[j].second[0];
          } else if (fields[j].second.size()>1) {
            O2SCL_ERR("Field had multiple entries.",
                      o2scl::exc_esanity);
          } else {
            O2SCL_ERR("Field found but value vector was empty.",
                      o2scl::exc_einval);
          }
        }
      }
      if (!key) {
        O2SCL_ERR((((std::string)"Field ")+field+
                   " not found in entry with no key ").c_str(),
                  o2scl::exc_einval);
      }
      O2SCL_ERR((((std::string)"Field ")+field+
                 " not found in entry with key "+(*key).c_str()).c_str(),
                o2scl::exc_einval);
      
      return fields[0].first;
    }

    /** \brief Return true if field \c field is present (case-insensitive)
     */
    bool is_field_present(std::string field) {

      // Convert user-specified field to lowercase
      for(size_t k=0;k<field.size();k++) {
        field[k]=std::tolower(field[k]);
      }
      
      for(size_t j=0;j<fields.size();j++) {

        std::string lower=fields[j].first;
        for(size_t k=0;k<lower.size();k++) {
          lower[k]=std::tolower(lower[k]);
        }
        
        if (lower==field && fields[j].second.size()>0) {
          return true;
        }
      }
      return false;
    }
    
    /** \brief Return true if field \c field1 or field \c field2 is 
        present (case-insensitive)
     */
    bool is_field_present_or(std::string field1, std::string field2) {
      
      for(size_t k=0;k<field1.size();k++) {
        field1[k]=std::tolower(field1[k]);
      }
      for(size_t k=0;k<field2.size();k++) {
        field2[k]=std::tolower(field2[k]);
      }
      
      for(size_t j=0;j<fields.size();j++) {
        std::string lower=fields[j].first;
        for(size_t k=0;k<lower.size();k++) {
          lower[k]=std::tolower(lower[k]);
        }
        if ((lower==field1 || lower==field2) && fields[j].second.size()>0) {
          return true;
        }
      }
      return false;
    }
    
  };
  
  /** \brief Manipulate BibTeX files using bibtex-spirit
   */
  class bib_file {

  protected:

    /** \brief Fill a string with chararacter \c ch to ensure
	its length is \c len
     */
    void fill(std::string &s, size_t len=78, char ch=' ');
    
    /** \brief Rewrap a list of strings into a new list with line
	breaks
     */
    void local_wrap(std::vector<std::string> &sv, size_t len=78);

    /** \brief Format the left and right strings into tabular
	output with optional highlighting
    */
    void format_and_output(std::string left, std::string right,
			   std::ostream &outs, bool bright=false, 
			   std::string sep=" | ", size_t len=78);

    /// \name Result codes for \ref ident_or_addl_fields()
    //@{
    /// Undetermined or different
    static const int ia_diff=0;
    /// Identical
    static const int ia_ident=1;
    /// One of the entries has additional fields not in the other
    static const int ia_addl_fields=2;
    //@}
    
    /// Return true if the entries are the identical
    void ident_or_addl_fields(bibtex::BibTeXEntry &bt_left,
			      bibtex::BibTeXEntry &bt_right, int &result);

    /// Merge all fields from RHS not in LHS to the LHS
    void merge_to_left(bibtex::BibTeXEntry &bt_left,
		       bibtex::BibTeXEntry &bt_right);
    
    /** \brief Format the field and value into one string for
	the \ref bib_output_twoup() function
     */
    void format_field_value(std::string field, std::string value,
			    std::string &outs);
    
    /*q* \brief LaTeX strings for translation
     */
    std::vector<std::string> trans_latex;

    /** \brief LaTeX strings for translation
     */
    std::vector<std::string> trans_latex_alt;

    /** \brief HTML strings for translation
     */
    std::vector<std::string> trans_html;

    /** \brief Unicode strings for translation
     */
    std::vector<std::string> trans_uni;

    /** \brief Type for journal name list iterator
     */
    typedef std::map<std::string,std::vector<std::string>,
      std::greater<std::string> >::iterator journal_it;

  public:
    
    /** \brief Output two entries in a tabular format
     */
    void bib_output_twoup(std::ostream &outs,
			  bibtex::BibTeXEntry &bt_left,
			  bibtex::BibTeXEntry &bt_right,
			  std::string left_header,
			  std::string right_header);
    
    /** \brief List of journal synonyms
     */
    std::map<std::string,std::vector<std::string>,
      std::greater<std::string> > journals;
    
    /** \brief Fields automatically removed by parse()
     */
    std::vector<std::string> remove_fields;
    
    /** \brief The full list of BibTeX entries
     */
    std::vector<bibtex::BibTeXEntry> entries;

    /** \brief A sorted list of keys and indexes
     */
    std::map<std::string,size_t,std::less<std::string> > sort;

    /// \name Special character handling (default is <tt>sc_allow_all</tt>)
    //@{
    int spec_chars;
    static const int sc_allow_all=0;
    static const int sc_force_unicode=1;
    static const int sc_force_html=2;
    static const int sc_force_latex=3;
    //@}
    
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
    /** \brief If true, remove LaTeX tildes from author names 
	(default true)
    */
    bool remove_author_tildes;
    /** \brief Verbosity parameter
     */
    int verbose;

    /** \brief Month names
     */
    std::vector<std::string> months_long;

    /** \brief Short month names
     */
    std::vector<std::string> months_short;

    /** \brief Short month names
     */
    std::vector<std::string> months_four;

    /** \brief Short month names
     */
    //@{
    int month_format;
    static const int month_format_long=0;
    static const int month_format_four=1;
    static const int month_format_short=2;
    static const int month_format_short_dot=3;
    static const int month_format_num=4;
    //@}

    /** \brief Create a ``bib_file`` object
     */
    bib_file();

    /** \brief Convert all characters in a string to lower case
     */
    std::string lower_string(std::string s);
    
    /** \brief Read a journal name list from file \c fname

	\note If a list was read previously, that list is
	deleted before reading the new list.

	If \ref verbose is greater than 0, then this function outputs
	the total number of journal lists read after reading the full
	list. If \ref verbose is greater than 1, then every list of
	synonyms is output to the screen.
    */
    int read_journals(std::string fname="");
    
    /** \brief Remove extra whitespace by parsing through a 
	``stringstream``
	
	This function ensures that each word is separated by one and
	only one space, removing all other whitespace.
    */
    void thin_whitespace(std::string &s);

    /** \brief Remove all whitespace and punctuation and
	convert to lower case
    */
    std::string journal_simplify(std::string s);

    /** \brief Find the standard abbrevation for a journal with
	name \c jour

	If an abbreviation is found, then this function returns 0,
	otherwise this function return 1. If no journal list has been
	loaded, then this function calls the error handler.
    */
    int find_abbrev(std::string jour, std::string &abbrev);
    
    /** \brief Find all synonyms for a journal with
	name \c jour

	If the journal is found in the list, then this function fills
	``list`` with all the synonyms and returns 0. If the journal
	is not found in the list, this function return 1. If no
	journal list has been loaded, then this function calls the
	error handler.
    */
    int find_abbrevs(std::string jour, std::vector<std::string> &list);
    
    /** \brief Given a pages field, return only the first page

	This function just looks for the first hyphen and returns all
	characters before it.
    */
    std::string first_page(std::string pages);

    /** \brief Search for a pattern, setting ``list`` equal
	to the set of keys which match
    */
    void search_keys(std::string pattern,
		     std::vector<std::string> &list);
    
    /** \brief Search for entries using 'or'

	If the number of arguments to this function is zero or
	an odd number, then the error handler is called.
    */
    void search_or(std::vector<std::string> &args);
    
    /** \brief Remove matching entries using 'or'

	If the number of arguments to this function is zero or
	an odd number, then the error handler is called.
    */
    void remove_or(std::vector<std::string> &args);
    
    /** \brief Search for entries using 'and'

	If the number of arguments to this function is zero or
	an odd number, then the error handler is called.
    */
    void search_and(std::vector<std::string> &args);
    
    /** \brief Check entry for required fields
     */
    void entry_check_required(bibtex::BibTeXEntry &bt);

    /** \brief If an 'article' or 'inproceedings' has no
	title, set the title equal to one space
    */
    bool entry_add_empty_title(bibtex::BibTeXEntry &bt);
    
    /** \brief If DOI number is present, ensure URL matches

	This function returns true if any change has been
	made to the entry.
    */
    bool entry_autoformat_url(bibtex::BibTeXEntry &bt);
    
    /** \brief Remove volume letters and move to journal names
	for some journals
    */
    bool entry_remove_vol_letters(bibtex::BibTeXEntry &bt);
    
    /** \brief Clean the current BibTeX entries
     */
    void clean(bool prompt=true);
    
    /** \brief In entry \c bt, set the value of \c field
	equal to \c value
    */
    int set_field_value(bibtex::BibTeXEntry &bt, std::string field,
			std::string value);
    
    /** \brief In entry with key \c key, set the value of \c field
	equal to \c value
    */
    int set_field_value(std::string key, std::string field,
			std::string value);
    
    /** \brief Parse a BibTeX file and perform some extra reformatting
     */
    void parse_bib(std::string fname);
    
    /** \brief Refresh the \ref sort object which contains a set
	of keys an indexes for the \ref entries array
    */
    void refresh_sort();
    
    /** \brief Sort the bibliography by key

	Because the indexes for a sorted list are always maintained in
	\ref sort, this is a \f${\cal O}(N)\f$ operation.

	\note This will call the error handler if more than one
	entry has the same key
    */
    void sort_bib();

    /** \brief Sort by date
     */
    void sort_by_date(bool descending=false);
    
    /** \brief Reverse the bibliography
     */
    void reverse_bib();
    
    /** \brief Output one entry \c bt to stream \c outs in 
	.bib format
    */
    void bib_output_one(std::ostream &outs, bibtex::BibTeXEntry &bt);
    
    /** \brief Return a positive number if \c bt and \c bt2 are possible
	duplicates

	This function returns 1 if the tags and keys are identical
	(except for capitalization) and 2 if the tags are the same and
	the keys are different, but the volume pages, and journal are
	the same. This function returns zero otherwise.
    */
    int possible_duplicate(bibtex::BibTeXEntry &bt,
			   bibtex::BibTeXEntry &bt2);
    
    /** \brief Create a list of possible duplicates of \c bt
	in the current set of BibTeX entries
    */
    void list_possible_duplicates(bibtex::BibTeXEntry &bt,
				  std::vector<size_t> &list);

    /** \brief Output one entry \c bt to stream \c outs in 
	plain text
    */
    void text_output_one(std::ostream &outs, bibtex::BibTeXEntry &bt);
    
    /** \brief Add entries in a specified BibTeX file to the current
	list, checking for duplicates and prompting if they're found
    */
    void add_bib(std::string fname);
    
    /** \brief Get entry by key name
     */
    bool is_key_present(std::string key);

    /** \brief Get entry by key name
     */
    bibtex::BibTeXEntry &get_entry_by_key(std::string key);

    /** \brief Change an entry's key
     */
    void change_key(std::string key1, std::string key2);    

    /** \brief Get index of entry by key name
     */
    size_t get_index_by_key(std::string key);
    
    /** \brief Reformat special characters to latex
     */
    std::string spec_char_to_latex(std::string s_in);
    
    /** \brief Reformat special characters to html
     */
    std::string spec_char_to_html(std::string s_in);
    
    /** \brief Reformat special characters to unicode

	This function is particularly useful in generating
	rst output.
     */
    std::string spec_char_to_uni(std::string s_in);

    /** \brief Reformat special characters based on <tt>spec_chars</tt>
     */
    std::string spec_char_auto(std::string s_in);
    
    /** \brief Return the last name of the first author,
	and "et al." if there is more than one author
    */
    std::string short_author(bibtex::BibTeXEntry &bt);
    
    /** \brief Return the last name of the first author
     */
    std::string last_name_first_author(bibtex::BibTeXEntry &bt);
    
    /** \brief Reformat author string into first and last names

	Note that by default this doesn't automatically remove curly
	braces from the author's last name. This option is put there
	because many BibTeX entries put braces around by default, even
	if they are not needed. Curly braces are, however, sometimes
	required if the actual "author" of the entry is not a person
	but an organization with the word "and" in the name of the
	organization. The best plan for curly braces is probably just
	to leave them as is in the bibtex entries, and naively remove
	them for text-based formats and phase out this 'remove_braces'
	option (since it only works for authors and we need something
	that works for titles also). 
     */
    void parse_author(std::string s_in,
		      std::vector<std::string> &firstv, 
		      std::vector<std::string> &lastv,
		      bool remove_braces=false);
    
    /** \brief Reformat author string into a list with commas and
	the word <tt>"and"</tt> before the last author
    */
    std::string author_firstlast(std::string s_in, 
				 bool remove_braces=true,
				 bool first_initial=true);
    
    /** \brief Return true if field named \c field (ignoring
	differences in field name capitalization) is present in entry
	\c bt
    */
    bool is_field_present(bibtex::BibTeXEntry &bt, std::string field);

    /** \brief Count the number of times that field \c field occurs
	in the entry
    */
    size_t count_field_occur(bibtex::BibTeXEntry &bt, std::string field);
    
    /** \brief Return true if field named \c field1 or field named \c
	field2 is present in entry \c bt
    */
    bool is_field_present(bibtex::BibTeXEntry &bt, std::string field1,
			  std::string field2);
    
    /** \brief Get field named \c field from entry \c bt (assuming
	the field occurs only once)
     */
    std::string &get_field(bibtex::BibTeXEntry &bt, std::string field);
    
    /** \brief Get all values for field named \c field from entry \c bt
     */
    void get_field_all(bibtex::BibTeXEntry &bt, std::string field,
		      std::vector<std::string> &list);
    
    /** \brief Get field named \c field from entry \c bt
     */
    std::vector<std::string> &get_field_list
      (bibtex::BibTeXEntry &bt, std::string field);

    /** \brief Convert tildes to spaces
     */
    void tilde_to_space(std::string &s);
  
    /** \brief Output an entry in HTML format
     */
    void output_html(std::ostream &os, bibtex::BibTeXEntry &bt);
    
    /** \brief Output an entry in LaTeX format
     */
    void output_latex(std::ostream &os, bibtex::BibTeXEntry &bt);

    /** \brief Add an entry to the list
     */
    void add_entry(bibtex::BibTeXEntry &bt);

  };

}
 
#endif
