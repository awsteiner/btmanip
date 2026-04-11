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

#ifndef BIBTEXREADER_HPP
#define BIBTEXREADER_HPP

#pragma once

#include <istream>
#include <numeric>
#include <string>

#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/optional.hpp>
#include <boost/range.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/utility/enable_if.hpp>

#include "bt_entry.h"

namespace bibtex {

namespace x3 = boost::spirit::x3;

//------------------------------------------------------------------
// Semantic action functors (replace Boost.Phoenix expressions)
//------------------------------------------------------------------

namespace detail {

/// @brief Concatenates a vector of strings into @p _val.
struct AccumulateStrings
{
    /// @param ctx X3 parse context carrying a vector<string> attr.
    template<typename Context>
    void operator()(Context& ctx) const
    {
        x3::_val(ctx) = std::accumulate(
            x3::_attr(ctx).begin(),
            x3::_attr(ctx).end(),
            std::string());
    }
};

/// @brief Assigns the tag string to @c BibTeXEntry::tag.
struct AssignTag
{
    /// @param ctx X3 parse context.
    template<typename Context>
    void operator()(Context& ctx) const
    {
        x3::_val(ctx).tag = x3::_attr(ctx);
    }
};

/// @brief Assigns parsed body (key + fields) to @c BibTeXEntry.
struct AssignBody
{
    /// @param ctx X3 parse context.
    template<typename Context>
    void operator()(Context& ctx) const
    {
        typedef boost::fusion::vector<
            boost::optional<std::string>,
            KeyValueVector> BodyAttr;
        BodyAttr const& b = x3::_attr(ctx);
        x3::_val(ctx).key    = boost::fusion::at_c<0>(b)
                                   ? *boost::fusion::at_c<0>(b)
                                   : std::string();
        x3::_val(ctx).fields = boost::fusion::at_c<1>(b);
    }
};

/// @brief Assigns a single @c KeyValue as the sole field.
struct AssignSingleField
{
    /// @param ctx X3 parse context carrying a @c KeyValue attr.
    template<typename Context>
    void operator()(Context& ctx) const
    {
        x3::_val(ctx).fields.assign(1, x3::_attr(ctx));
    }
};

/// @brief Wraps a @c ValueVector in a @c KeyValue with an empty key,
///        then assigns it as the sole field.
struct AssignSimpleValues
{
    /// @param ctx X3 parse context carrying a @c ValueVector attr.
    template<typename Context>
    void operator()(Context& ctx) const
    {
        x3::_val(ctx).fields.assign(
            1,
            KeyValue(KeyValue::first_type(), x3::_attr(ctx)));
    }
};

//------------------------------------------------------------------
// Rule tag types
//------------------------------------------------------------------

struct TagId          {};
struct EntryKeyId     {};
struct KeyId          {};
struct QuotedId       {};
struct ValueId        {};
struct ValuesId       {};
struct FieldId        {};
struct FieldsId       {};
struct BodyId         {};
struct GenericId      {};
struct StringEntryId  {};
struct SimpleId       {};
struct EntryId        {};
struct JunkId         {};
struct StartId        {};

struct EscapedTextId    {};
struct QuoteTextId      {};
struct InnerBraceTextId {};
struct InnerQuoteTextId {};

//------------------------------------------------------------------
// Rule type aliases
//------------------------------------------------------------------

/// @brief Matches a BibTeX entry tag (e.g. "article").
typedef x3::rule<TagId, std::string>       TagRule;

/// @brief Matches a BibTeX citation key.
typedef x3::rule<EntryKeyId, std::string>  EntryKeyRule;

/// @brief Matches a field key (left-hand side of @c key=value).
typedef x3::rule<KeyId, std::string>       KeyRule;

/// @brief Matches brace-escaped text outside braces.
typedef x3::rule<EscapedTextId, std::string>    EscapedTextRule;

/// @brief Matches text inside double-quoted values.
typedef x3::rule<QuoteTextId, std::string>      QuoteTextRule;

/// @brief Matches nested brace content inside brace values.
typedef x3::rule<InnerBraceTextId, std::string> InnerBraceTextRule;

/// @brief Matches nested brace content inside quoted values.
typedef x3::rule<InnerQuoteTextId, std::string> InnerQuoteTextRule;

/// @brief Matches a complete quoted or braced field value.
typedef x3::rule<QuotedId, std::string>    QuotedRule;

/// @brief Matches a single field value component.
typedef x3::rule<ValueId, std::string>     ValueRule;

/// @brief Matches a @c '#'-concatenated list of values.
typedef x3::rule<ValuesId, ValueVector>    ValuesRule;

/// @brief Matches a single @c key=values pair.
typedef x3::rule<FieldId, KeyValue>        FieldRule;

/// @brief Matches the comma-separated list of fields.
typedef x3::rule<FieldsId, KeyValueVector> FieldsRule;

/// @brief Matches the body of a generic entry (key and fields).
///
/// The attribute is a Fusion vector of (optional<string>, KeyValueVector).
typedef x3::rule<
    BodyId,
    boost::fusion::vector<
        boost::optional<std::string>,
        KeyValueVector> >
    BodyRule;

/// @brief Matches a generic @c \@type{key, ...} entry.
typedef x3::rule<GenericId,     BibTeXEntry> GenericRule;

/// @brief Matches a @c \@string{...} entry.
typedef x3::rule<StringEntryId, BibTeXEntry> StringEntryRule;

/// @brief Matches @c \@comment, @c \@include, or @c \@preamble.
typedef x3::rule<SimpleId,      BibTeXEntry> SimpleRule;

/// @brief Matches any recognised BibTeX entry type.
typedef x3::rule<EntryId,       BibTeXEntry> EntryRule;

/// @brief Skips junk preceding an entry.
typedef x3::rule<JunkId>                     JunkRule;

/// @brief Top-level rule: junk then entry.
typedef x3::rule<StartId,       BibTeXEntry> StartRule;

//------------------------------------------------------------------
// Rule objects
//
// Named without trailing underscores so BOOST_SPIRIT_DEFINE(name)
// correctly locates the matching name_def variable.  A trailing
// underscore would cause the macro to look for name__def (double
// underscore), which does not exist.
//------------------------------------------------------------------

TagRule           const tag          = "tag";
EntryKeyRule      const entryKey     = "entry key";
KeyRule           const key          = "key";
EscapedTextRule   const escapedText  = "escaped text";
QuoteTextRule     const quoteText    = "quote text";
InnerBraceTextRule const innerBraceText = "inner brace text";
InnerQuoteTextRule const innerQuoteText = "inner quote text";
QuotedRule        const quoted       = "quoted value";
ValueRule         const value        = "value";
ValuesRule        const values       = "values";
FieldRule         const field        = "field";
FieldsRule        const fields       = "fields";
BodyRule          const body         = "body";
GenericRule       const generic      = "generic entry";
StringEntryRule   const stringEntry  = "string entry";
SimpleRule        const simple       = "simple entry";
EntryRule         const entry        = "entry";
JunkRule          const junk         = "junk";
StartRule         const start        = "start";

//------------------------------------------------------------------
// Symbol tables for escape sequences
//------------------------------------------------------------------

struct EscapedBraces_ : x3::symbols<char>
{
    EscapedBraces_()
    {
        add("\\{", '{')
           ("\\}", '}');
    }
} const escapedBrace;

struct EscapedQuotes_ : x3::symbols<char>
{
    EscapedQuotes_()
    {
        add("\\\"", '"');
    }
} const escapedQuote;

//------------------------------------------------------------------
// Rule definitions
//
// Note on 'auto': Spirit X3 definition expressions are composed
// via operator overloading and produce deeply nested, anonymous
// template types that cannot be spelled out.  'auto' is the only
// practical choice for _def variables.  Its use is confined to
// these internal definition variables; all rule types and public
// API signatures are fully spelled out above.
//------------------------------------------------------------------

/// @brief One or more alphanumeric characters.
auto const tag_def =
    +x3::ascii::alnum;

/// @brief Non-comma, non-space characters (citation key).
auto const entryKey_def =
    x3::lexeme[+(~x3::char_(',') - x3::ascii::space)];

/// @brief Characters excluding @c =,}) and whitespace (field key).
auto const key_def =
    x3::lexeme[+(~x3::char_("=,})") - x3::ascii::space)];

/// @brief Text not starting with @c { and containing no bare braces.
auto const escapedText_def =
    !x3::lit('{')
    >> +(escapedBrace | ~x3::char_("{}"));

/// @brief Text inside double quotes (no bare quotes or braces).
auto const quoteText_def =
    +(escapedQuote | ~x3::char_("\"{}"));

/// @brief Recursive brace-delimited text or escaped text.
auto const innerBraceText_def =
    x3::as_string[
        x3::char_('{')
        >> *(innerBraceText | escapedText)
        >> x3::char_('}')
    ]
    | escapedText;

/// @brief Recursive brace-delimited text inside a quoted value.
auto const innerQuoteText_def =
    x3::as_string[
        x3::char_('{')
        >> *(innerQuoteText | escapedText)
        >> x3::char_('}')
    ]
    | quoteText;

/// @brief Quoted (double-quote or brace-delimited) field value.
auto const quoted_def =
    x3::lexeme[
        ('"' >> *innerQuoteText >> '"')
        | ('{' >> *innerBraceText >> '}')
    ][AccumulateStrings()];

/// @brief A single value: quoted string or bare non-delimiter text.
auto const value_def =
    quoted
    | +~x3::char_(",})#");

/// @brief A @c '#'-separated sequence of values.
auto const values_def =
    value % '#';

/// @brief A field: @c key = values.
auto const field_def =
    key >> '=' >> values;

/// @brief Zero or more fields separated by commas.
auto const fields_def =
    -(field % ',');

/// @brief Entry body: optional key, comma, fields, optional comma.
auto const body_def =
    -entryKey >> ','
    >> fields
    >> -x3::lit(',');

/// @brief Generic BibTeX entry (@type{key, fields}).
auto const generic_def =
    '@' >> tag[AssignTag()]
    >> (
        ('{' >> body[AssignBody()] >> '}')
        | ('(' >> body[AssignBody()] >> ')')
       );

/// @brief @string entry (@string{key=value}).
auto const stringEntry_def =
    '@' >> x3::ascii::no_case[x3::string("string")][AssignTag()]
    >> (
        ('{' >> field[AssignSingleField()] >> '}')
        | ('(' >> field[AssignSingleField()] >> ')')
       );

/// @brief Simple entries: @comment, @include, @preamble.
auto const simple_def =
    '@' >> x3::ascii::no_case[
        (x3::string("comment")
         | x3::string("include")
         | x3::string("preamble"))
        [AssignTag()]
    ]
    >> (
        ('{' >> values[AssignSimpleValues()] >> '}')
        | ('(' >> values[AssignSimpleValues()] >> ')')
       );

/// @brief Dispatches to stringEntry, simple, or generic.
auto const entry_def =
    stringEntry | simple | generic;

/// @brief Skips all characters before the first @c \@.
auto const junk_def =
    *~x3::lit('@');

/// @brief Top-level: skip junk, then parse one entry.
auto const start_def =
    junk >> entry;

BOOST_SPIRIT_DEFINE(
    tag,
    entryKey,
    key,
    escapedText,
    quoteText,
    innerBraceText,
    innerQuoteText,
    quoted,
    value,
    values,
    field,
    fields,
    body,
    generic,
    stringEntry,
    simple,
    entry,
    junk,
    start
)

} // namespace detail

//------------------------------------------------------------------
// Public read() interface
//------------------------------------------------------------------

/**
 * @brief Parse a single BibTeX entry from an iterator range.
 *
 * @tparam ForwardIterator  Iterator satisfying ForwardIterator.
 * @tparam Skipper          Skipper parser type.
 * @param  first            Begin of input range.
 * @param  last             End of input range.
 * @param  skipper          Skipper instance.
 * @param  entry            Output: the parsed entry.
 * @return @c true on success.
 */
template<class ForwardIterator, class Skipper>
inline bool read(ForwardIterator first,
                 ForwardIterator last,
                 Skipper& skipper,
                 BibTeXEntry& entry)
{
    return x3::phrase_parse(
        first, last, detail::start, skipper, entry);
}

/**
 * @brief Parse all BibTeX entries from an iterator range into a
 *        container.
 *
 * @tparam ForwardIterator  Iterator satisfying ForwardIterator.
 * @tparam Skipper          Skipper parser type.
 * @tparam Container        Container whose @c value_type is
 *                          @c BibTeXEntry.
 * @param  first            Begin of input range.
 * @param  last             End of input range.
 * @param  skipper          Skipper instance.
 * @param  entries          Output container.
 * @return @c true on success.
 */
template<class ForwardIterator, class Skipper, class Container>
inline bool read(
    ForwardIterator first,
    ForwardIterator last,
    Skipper& skipper,
    Container& entries,
    typename boost::enable_if<
        boost::is_same<
            typename Container::value_type,
            BibTeXEntry> >::type* /*dummy*/ = NULL)
{
    return x3::phrase_parse(
        first, last, *detail::start, skipper, entries);
}

/**
 * @brief Parse a single BibTeX entry from a range.
 *
 * @tparam ForwardRange  Range satisfying ForwardRange.
 * @tparam Skipper       Skipper parser type.
 * @param  range         Input range.
 * @param  skipper       Skipper instance.
 * @param  entry         Output: the parsed entry.
 * @return @c true on success.
 */
template<class ForwardRange, class Skipper>
inline bool read(const ForwardRange& range,
                 Skipper& skipper,
                 BibTeXEntry& entry)
{
    return read(
        boost::const_begin(range),
        boost::const_end(range),
        skipper, entry);
}

/**
 * @brief Parse all BibTeX entries from a range into a container.
 *
 * @tparam ForwardRange  Range satisfying ForwardRange.
 * @tparam Skipper       Skipper parser type.
 * @tparam Container     Container whose @c value_type is
 *                       @c BibTeXEntry.
 * @param  range         Input range.
 * @param  skipper       Skipper instance.
 * @param  entries       Output container.
 * @return @c true on success.
 */
template<class ForwardRange, class Skipper, class Container>
inline bool read(
    const ForwardRange& range,
    Skipper& skipper,
    Container& entries,
    typename boost::enable_if<
        boost::is_same<
            typename Container::value_type,
            BibTeXEntry> >::type* /*dummy*/ = NULL)
{
    return read(
        boost::const_begin(range),
        boost::const_end(range),
        skipper, entries);
}

/**
 * @brief Parse a single BibTeX entry using the default skipper.
 *
 * @tparam ForwardIterator  Iterator satisfying ForwardIterator.
 * @param  first            Begin of input range.
 * @param  last             End of input range.
 * @param  entry            Output: the parsed entry.
 * @return @c true on success.
 */
template<class ForwardIterator>
inline bool read(ForwardIterator first,
                 ForwardIterator last,
                 BibTeXEntry& entry)
{
    return read(first, last, bibtex::space, entry);
}

/**
 * @brief Parse all BibTeX entries using the default skipper.
 *
 * @tparam ForwardIterator  Iterator satisfying ForwardIterator.
 * @tparam Container        Container whose @c value_type is
 *                          @c BibTeXEntry.
 * @param  first            Begin of input range.
 * @param  last             End of input range.
 * @param  entries          Output container.
 * @return @c true on success.
 */
template<class ForwardIterator, class Container>
inline bool read(
    ForwardIterator first,
    ForwardIterator last,
    Container& entries,
    typename boost::enable_if<
        boost::is_same<
            typename Container::value_type,
            BibTeXEntry> >::type* /*dummy*/ = NULL)
{
    return read(first, last, bibtex::space, entries);
}

/**
 * @brief Parse a single BibTeX entry from a range using the
 *        default skipper.
 *
 * @tparam ForwardRange  Range satisfying ForwardRange.
 * @param  range         Input range.
 * @param  entry         Output: the parsed entry.
 * @return @c true on success.
 */
template<class ForwardRange>
inline bool read(
    const ForwardRange& range,
    BibTeXEntry& entry,
    typename boost::enable_if<
        boost::has_range_iterator<ForwardRange> >::type* /*dummy*/
        = NULL)
{
    return read(range, bibtex::space, entry);
}

/**
 * @brief Parse all BibTeX entries from a range into a container
 *        using the default skipper.
 *
 * @tparam ForwardRange  Range satisfying ForwardRange.
 * @tparam Container     Container whose @c value_type is
 *                       @c BibTeXEntry.
 * @param  range         Input range.
 * @param  entries       Output container.
 * @return @c true on success.
 */
template<class ForwardRange, class Container>
inline bool read(
    const ForwardRange& range,
    Container& entries,
    typename boost::enable_if<
        boost::has_range_iterator<ForwardRange> >::type* /*dummy*/
        = NULL)
{
    return read(range, bibtex::space, entries);
}

/**
 * @brief Parse a single BibTeX entry from an input stream.
 *
 * @tparam E   Character type.
 * @tparam T   Character traits type.
 * @param  in  Input stream.
 * @param  e   Output: the parsed entry.
 * @return @c true on success.
 */
template<class E, class T>
inline bool read(std::basic_istream<E, T>& in, BibTeXEntry& e)
{
    boost::io::ios_flags_saver saver(in);
    in.unsetf(std::ios_base::skipws);

    typedef boost::spirit::basic_istream_iterator<E>
        istream_iterator;
    return read(istream_iterator(in), istream_iterator(), e);
}

/**
 * @brief Parse all BibTeX entries from an input stream into a
 *        container.
 *
 * @tparam E        Character type.
 * @tparam T        Character traits type.
 * @tparam Container Container whose @c value_type is @c BibTeXEntry.
 * @param  in       Input stream.
 * @param  entries  Output container.
 * @return @c true on success.
 */
template<class E, class T, class Container>
inline bool read(std::basic_istream<E, T>& in, Container& entries)
{
    boost::io::ios_flags_saver saver(in);
    in.unsetf(std::ios_base::skipws);

    typedef boost::spirit::basic_istream_iterator<E>
        istream_iterator;
    return read(istream_iterator(in), istream_iterator(), entries);
}

/**
 * @brief Stream extraction operator for a single BibTeX entry.
 *
 * @tparam E     Character type.
 * @tparam T     Character traits type.
 * @param  in    Input stream.
 * @param  entry Output: the parsed entry.
 * @return Reference to @p in.
 */
template<class E, class T>
inline std::basic_istream<E, T>&
operator>>(std::basic_istream<E, T>& in, BibTeXEntry& entry)
{
    read(in, entry);
    return in;
}

/**
 * @brief Stream extraction operator for a container of BibTeX
 *        entries.
 *
 * Enabled only when @c Range::value_type is @c BibTeXEntry.
 *
 * @tparam E       Character type.
 * @tparam T       Character traits type.
 * @tparam Range   Container satisfying ForwardRange with
 *                 @c value_type == @c BibTeXEntry.
 * @param  in      Input stream.
 * @param  entries Output container.
 * @return Reference to @p in.
 */
template<class E, class T, class Range>
inline typename boost::enable_if<
    boost::mpl::and_<
        boost::has_range_iterator<Range>,
        boost::is_same<
            typename Range::value_type,
            BibTeXEntry>
        >,
    std::basic_istream<E, T>
>::type&
operator>>(std::basic_istream<E, T>& in, Range& entries)
{
    read(in, entries);
    return in;
}

} // namespace bibtex

#endif // BIBTEXREADER_HPP
