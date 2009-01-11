#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/scoped_ptr.hpp>

#include <shear/make_grammar.hpp>
#include <shear/production.hpp>

namespace sh = shear;
namespace mpl = boost::mpl;

namespace arithmetic_symbols {

class X {};
class PLUS {};

class expression {
  public:
    expression(const X&) : sub_() {}
    expression(const expression& e, const X&) : sub_(new expression(e)) {}

    bool is_x() const { return !sub_; }
    const expression& subexpression() const { return *sub_; }
  private:
    expression(const expression& copy) {
      if (copy.sub_) {
        sub_.reset(new expression(*copy.sub_));
      }
    }
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
      sh::production<expression, sh::discard<PLUS>, X>
    >::type
  >::type grammar_type;
  // Construct the grammar
  grammar_type arithmetic_grammar;
  arithmetic_grammar.check();
#if 0
  // The parser type is similarly horrible, but obtainable from the grammar
  // type so we don't need to express all that stuff again.
  typedef sh::result_of::make_lalr_parser<grammar_type>::type parser_type;
  parser_type arithmetic_parser =
    sh::make_lalr_parser(arithmetic_grammar);
  // Tokens passed in to the parser one by one
  arithmetic_parser.handle_token(X())
  arithmetic_parser.handle_token(PLUS())
  arithmetic_parser.handle_token(X())
  // And then we finalize to indicate the end of input, getting a parse object
  // encapsulating the result
  parser_type::parse_type parse = arithmetic_parser.finalize();
  // The parse object converted to bool yields success / failure
  assert(parse);
  // On success, we can get the root of the parse tree
  expression root = *parse.root();
  // Check it constructed the right things
  assert(!expression.is_x());
  assert(expression.subexpression().is_x());
#endif
}

