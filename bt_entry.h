// Copyright (c) 2012 Sergiu Dotenco
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef BIBTEXENTRY_HPP
#define BIBTEXENTRY_HPP

#pragma once

#include <string>
#include <utility>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <boost/spirit/home/x3.hpp>

namespace bibtex {

  namespace x3 = boost::spirit::x3;

  //------------------------------------------------------------------
  // Common types
  //------------------------------------------------------------------

  /// @brief A list of values associated with a single BibTeX field.
  typedef std::vector<std::string> ValueVector;

  /// @brief A BibTeX field: a key string paired with its values.
  typedef std::pair<std::string, ValueVector> KeyValue;

  /// @brief An ordered list of BibTeX fields.
  typedef std::vector<KeyValue> KeyValueVector;

  //------------------------------------------------------------------
  // Skipper
  //
  // Matches standard ASCII whitespace or a BibTeX line comment
  // introduced by '%' and running to (and including) end-of-line.
  //
  // Note on 'auto': Spirit X3 rule definition expressions have
  // deeply nested, unspellable template types produced by operator
  // overloading.  The only practical way to name them is with
  // 'auto'.  We confine this to the single _def variable inside
  // the detail namespace; all public API types are spelled out.
  //------------------------------------------------------------------

  namespace detail {

    /// @brief Tag type for the BibTeX comment/whitespace skipper.
    struct SkipId {};

    /// @brief Concrete rule type for the skipper.
    ///
    /// No attribute (void), so phrase_parse uses it purely for
    /// whitespace/comment consumption.
    typedef x3::rule<SkipId> SkipRule;

    /// @brief The skipper rule object.
    ///
    /// Named @c skip (no trailing underscore) so that
    /// BOOST_SPIRIT_DEFINE(skip) correctly generates @c skip_def
    /// and not @c skip__def.
    SkipRule const skip = "bibtex skipper";

    // 'auto' is unavoidable here: the type of a composed X3 parser
    // expression is an unspellable recursive template instantiation.
    // It is confined to this one internal definition variable.
    auto const skip_def =
        x3::ascii::space
        | ('%' >> *(x3::char_ - x3::eol) >> x3::eol);

    BOOST_SPIRIT_DEFINE(skip)

  } // namespace detail

  /// @brief Convenience typedef: the type of the BibTeX skipper.
  typedef detail::SkipRule Space;

  /// @brief The BibTeX skipper instance (whitespace + % comments).
  detail::SkipRule const& space = detail::skip;

  //------------------------------------------------------------------
  // BibTeXEntry
  //------------------------------------------------------------------

  /**
   * @brief Represents a single BibTeX entry.
   *
   * An entry consists of a tag (entry type), an optional citation
   * key, and an ordered list of key/value field pairs.
   */
  struct BibTeXEntry
    : boost::equality_comparable<BibTeXEntry>
  {
    /// Entry type tag, e.g. @c article or @c book.
    std::string tag;
    /// Optional citation key, absent for @c \@comment etc.
    boost::optional<std::string> key;
    /// Ordered list of field key/value pairs.
    KeyValueVector fields;
  };

  /**
   * @brief Equality comparison for BibTeX entries.
   *
   * Two entries are equal when their tag, key, and fields all match.
   *
   * @param lhs Left-hand entry.
   * @param rhs Right-hand entry.
   * @return @c true if all three members compare equal.
   */
  inline bool operator==(const BibTeXEntry& lhs,
                         const BibTeXEntry& rhs)
  {
    return lhs.tag    == rhs.tag
        && lhs.key    == rhs.key
        && lhs.fields == rhs.fields;
  }

} // namespace bibtex

BOOST_FUSION_ADAPT_STRUCT(
  bibtex::BibTeXEntry,
  (std::string, tag)
  (boost::optional<std::string>, key)
  (bibtex::KeyValueVector, fields)
)

#endif // BIBTEXENTRY_HPP
