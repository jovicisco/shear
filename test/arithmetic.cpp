#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <boost/scoped_ptr.hpp>

#include <shear/make_grammar.hpp>
#include <shear/discard.hpp>
#include <shear/production.hpp>
#include <shear/lalr/parser_from_grammar.hpp>

namespace sh = shear;
namespace mpl = boost::mpl;

namespace arithmetic_symbols {

class X {};
class PLUS {};

class expression {
  public:
    expression(const X&) : sub_() {}
    expression(const expression& e, const X&) : sub_(new expression(e)) {}
    expression(const expression& copy) {
      if (copy.sub_) {
        sub_.reset(new expression(*copy.sub_));
      }
    }

    bool is_x() const { return !sub_; }
    const expression& subexpression() const { return *sub_; }
  private:
    boost::scoped_ptr<expression> sub_;
};

}

using namespace arithmetic_symbols;

BOOST_AUTO_TEST_CASE(arithmetic_grammar)
{
  // The type of the grammar incorporates pretty much everything required for
  // it (an unfortunate almost-necessity; runtime polymorphism opens far too
  // many worm cans.  With the aid of decltype we could
  // provide an alternate more BNF-ish syntax a la spirit, but this works with
  // C++03)
  typedef sh::make_grammar<
    // root symbol
    expression,
    // tokens
    mpl::vector<X, PLUS>::type,
    // productions
    mpl::vector<
      sh::production<expression, X>,
      sh::production<expression, expression, sh::discard<PLUS>, X>
    >::type
  >::type Grammar;
  // Construct the grammar
  Grammar grammar;
  try {
    grammar.check();
  } catch (sh::grammar_exception& e) {
    BOOST_ERROR(std::string("grammar exception: ") + e.what());
  }
  std::cout << "productions:\n";
  grammar.dump_productions(std::cout);
  // The parser type is similarly horrible, but obtainable from the grammar
  // type so we don't need to express all that stuff again.
  typedef sh::lalr::parser_from_grammar<Grammar> Parser;
  Parser parser(grammar, true);
  std::cout << "transition array:\n";
  parser.dump_transition_array(std::cout);
  // Tokens passed in to the parser one by one
  parser.handle_token(X());
  parser.handle_token(PLUS());
  parser.handle_token(X());
  // And then we finalize to indicate the end of input, getting a parse object
  // encapsulating the result
  Parser::parse_type parse = parser.finalize();
  // The parse object converted to bool yields success / failure
  if (!parse) {
    std::copy(
        parse.syntax_errors().begin(), parse.syntax_errors().end(),
        std::ostream_iterator<sh::syntax_error>(std::cerr, "\n")
      );
    BOOST_ERROR("parse suffered syntax error");
  }
  // On success, we can get the root of the parse tree
  expression root = *parse.root();
  // Check it constructed the right things
  assert(!root.is_x());
  assert(root.subexpression().is_x());
}

