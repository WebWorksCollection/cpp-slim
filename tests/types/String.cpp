#include <boost/test/unit_test.hpp>
#include "expression/Parser.hpp"
#include "expression/Ast.hpp"
#include "expression/Lexer.hpp"
#include "expression/Scope.hpp"
#include "types/Array.hpp"
#include "types/Number.hpp"
#include "types/Proc.hpp"
#include "types/String.hpp"
#include "Error.hpp"

#include "../TestAccumulator.hpp"

using namespace slim;
using namespace slim::expr;
BOOST_AUTO_TEST_SUITE(TestString)

std::string eval(Ptr<ViewModel> model , const std::string &str)
{
    Scope scope(model);
    Lexer lexer(str);
    expr::LocalVarNames vars;
    Parser parser(vars, lexer);
    auto expr = parser.full_expression();
    auto result = expr->eval(scope);
    return result->inspect();
}
std::string eval(const std::string &str)
{
    return eval(create_view_model(), str);
}

//Encoding/unicode
BOOST_AUTO_TEST_CASE(ascii_only)
{
    BOOST_CHECK_EQUAL("true", eval("'abcd'.ascii_only?"));
    BOOST_CHECK_EQUAL("false", eval("'abcd�'.ascii_only?"));
}
BOOST_AUTO_TEST_CASE(bytes)
{
    BOOST_CHECK_EQUAL("[]", eval("''.bytes"));
    BOOST_CHECK_EQUAL("[97]", eval("'a'.bytes"));
    BOOST_CHECK_EQUAL("[194, 163]", eval("'\xC2\xA3'.bytes")); //GBP � U+00A3

    auto model = create_view_model();
    auto data = create_object<TestAccumulator>();
    model->set_attr("data", data);

    eval(model, "''.each_byte{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[]", data->check());

    eval(model, "'\xC2\xA3'.each_byte{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[194, 163]", data->check());

    eval(model, "'\xC2\xA3'.each_byte.each{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[194, 163]", data->check());
}
BOOST_AUTO_TEST_CASE(byteslice)
{
    //range
    BOOST_CHECK_EQUAL("\"es\"", eval("'test'.byteslice 1..2"));
    BOOST_CHECK_EQUAL("\"t\"", eval("'test'.byteslice 3...5"));
    BOOST_CHECK_EQUAL("\"es\"", eval("'test'.byteslice -3...-1"));
    BOOST_CHECK_EQUAL("\"\"", eval("'test'.byteslice 4...6"));
    BOOST_CHECK_EQUAL("nil", eval("'test'.byteslice -5...-3"));
    BOOST_CHECK_EQUAL("nil", eval("'test'.byteslice 5...7"));
    BOOST_CHECK_EQUAL("\"\"", eval("'test'.byteslice 2...2"));
    BOOST_CHECK_EQUAL("\"s\"", eval("'test'.byteslice 2...-1"));
    //offset, offset len
    BOOST_CHECK_EQUAL("\"s\"", eval("'test'.byteslice 2"));
    BOOST_CHECK_EQUAL("\"st\"", eval("'test'.byteslice 2, 2"));
    BOOST_CHECK_EQUAL("nil", eval("'test'.byteslice -5"));
    BOOST_CHECK_EQUAL("nil", eval("'test'.byteslice 5"));
    BOOST_CHECK_EQUAL("\"t\"", eval("'test'.byteslice 3, 2"));
    BOOST_CHECK_EQUAL("nil", eval("'test'.byteslice 5, 2"));
    BOOST_CHECK_EQUAL("\"\xC2\"", eval("'\xC2\xA3'.byteslice 0")); //GBP � U+00A3
}
BOOST_AUTO_TEST_CASE(chars)
{
    BOOST_CHECK_EQUAL("[]", eval("''.chars"));
    BOOST_CHECK_EQUAL("[\"a\"]", eval("'a'.chars"));
    BOOST_CHECK_EQUAL("[\"\xC2\xA3\"]", eval("'\xC2\xA3'.chars"));
    BOOST_CHECK_EQUAL("[\"\xC2\xA3\", \"a\"]", eval("'\xC2\xA3""a'.chars"));

    auto model = create_view_model();
    auto data = create_object<TestAccumulator>();
    model->set_attr("data", data);

    eval(model, "''.each_char{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[]", data->check());

    eval(model, "'\xC2\xA3'.each_char{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[\"\xC2\xA3\"]", data->check());

    eval(model, "'\xC2\xA3'.each_char.each{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[\"\xC2\xA3\"]", data->check());
}
BOOST_AUTO_TEST_CASE(chop)
{
    BOOST_CHECK_EQUAL("\"\"", eval("''.chop"));
    BOOST_CHECK_EQUAL("\"\"", eval("'a'.chop"));
    BOOST_CHECK_EQUAL("\"tes\"", eval("'test'.chop"));
    BOOST_CHECK_EQUAL("\"test\"", eval("'test\xC2\xA3'.chop"));
}
BOOST_AUTO_TEST_CASE(chr)
{
    BOOST_CHECK_EQUAL("\"\"", eval("''.chr"));
    BOOST_CHECK_EQUAL("\"a\"", eval("'a'.chr"));
    BOOST_CHECK_EQUAL("\"t\"", eval("'test'.chr"));
    BOOST_CHECK_EQUAL("\"\xC2\xA3\"", eval("'\xC2\xA3test'.chr"));
}
BOOST_AUTO_TEST_CASE(codepoints)
{
    BOOST_CHECK_EQUAL("[]", eval("''.codepoints"));
    BOOST_CHECK_EQUAL("[97]", eval("'a'.codepoints"));
    BOOST_CHECK_EQUAL("[163]", eval("'\xC2\xA3'.codepoints"));
    BOOST_CHECK_EQUAL("[163, 97]", eval("'\xC2\xA3""a'.codepoints"));

    auto model = create_view_model();
    auto data = create_object<TestAccumulator>();
    model->set_attr("data", data);

    eval(model, "''.each_codepoint{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[]", data->check());

    eval(model, "'\xC2\xA3'.each_codepoint{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[163]", data->check());

    eval(model, "'\xC2\xA3'.each_codepoint.each{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[163]", data->check());
}
BOOST_AUTO_TEST_CASE(getbyte)
{
    BOOST_CHECK_EQUAL("nil", eval("''.getbyte 0"));
    BOOST_CHECK_EQUAL("97", eval("'a'.getbyte 0"));
    BOOST_CHECK_EQUAL("97", eval("'a'.getbyte -1"));
    BOOST_CHECK_EQUAL("nil", eval("'a'.getbyte 1"));
    BOOST_CHECK_EQUAL("nil", eval("'a'.getbyte -2"));

    BOOST_CHECK_EQUAL("194", eval("'\xC2\xA3'.getbyte 0"));
    BOOST_CHECK_EQUAL("163", eval("'\xC2\xA3'.getbyte 1"));
    BOOST_CHECK_EQUAL("97", eval("'\xC2\xA3""a'.getbyte 2"));
}
BOOST_AUTO_TEST_CASE(scrub)
{
    BOOST_CHECK_EQUAL("\"\"", eval("''.scrub"));
    BOOST_CHECK_EQUAL("\"a\"", eval("'a'.scrub"));
    BOOST_CHECK_EQUAL("\"a\xEF\xBF\xBD\"", eval("'a\xC2'.scrub"));
    BOOST_CHECK_EQUAL("\"a\xEF\xBF\xBD\xEF\xBF\xBD\"", eval("'a\xC2\xC2'.scrub"));
    BOOST_CHECK_EQUAL("\"a??\"", eval("'a\xC2\xC2'.scrub '?'"));
}



BOOST_AUTO_TEST_CASE(inspect_escape)
{
    std::string escaped = "\"\\\\ \\' \\\" \\r \\n \\t\"";
    BOOST_CHECK_EQUAL(escaped, eval(escaped));
}

BOOST_AUTO_TEST_CASE(cmp)
{
    BOOST_CHECK_EQUAL("true", eval("'abcd' == 'abcd'"));
    BOOST_CHECK_EQUAL("false", eval("'abcd' == 'abcD'"));
    BOOST_CHECK_EQUAL("true", eval("'5' == '5'"));
    BOOST_CHECK_EQUAL("false", eval("'5' == 5"));

    BOOST_CHECK_EQUAL("0", eval("'abcd' <=> 'abcd'"));
    BOOST_CHECK_EQUAL("-1", eval("'aacd' <=> 'abcd'"));
    BOOST_CHECK_EQUAL("-1", eval("'abcd' <=> 'abcde'"));
    BOOST_CHECK_EQUAL("-1", eval("'Abcd' <=> 'abcd'"));
    BOOST_CHECK_EQUAL("1", eval("'abcde' <=> 'abcd'"));
    BOOST_CHECK_EQUAL("1", eval("'abcd' <=> 'aacd'"));
    BOOST_CHECK_EQUAL("1", eval("'abcd' <=> 'Abcd'"));


    BOOST_CHECK_EQUAL("0", eval("'abcd'.casecmp 'abcd'"));
    BOOST_CHECK_EQUAL("0", eval("'Abcd'.casecmp 'abcd'"));
    BOOST_CHECK_EQUAL("0", eval("'abcd'.casecmp 'Abcd'"));
    BOOST_CHECK_EQUAL("-1", eval("'aacd'.casecmp 'Abcd'"));
    BOOST_CHECK_EQUAL("1", eval("'accd'.casecmp 'Abcd'"));
    BOOST_CHECK_EQUAL("-1", eval("'Abcd'.casecmp 'abcda'"));
    BOOST_CHECK_EQUAL("1", eval("'abcd'.casecmp 'Abc'"));


    BOOST_CHECK_EQUAL("true", eval("'abcd test'.start_with? 'abcd'"));
    BOOST_CHECK_EQUAL("false", eval("'abcd test'.start_with? 'test'"));
    BOOST_CHECK_EQUAL("false", eval("'abcd test'.start_with? 'cd', 'st'"));
    BOOST_CHECK_EQUAL("true", eval("'abcd test'.start_with? 'cd', 'ab'"));

    BOOST_CHECK_EQUAL("false", eval("'abcd test'.end_with? 'abcd'"));
    BOOST_CHECK_EQUAL("true", eval("'abcd test'.end_with? 'test'"));
    BOOST_CHECK_EQUAL("true", eval("'abcd test'.end_with? 'cd', 'st'"));
    BOOST_CHECK_EQUAL("false", eval("'abcd test'.end_with? 'cd', 'ab'"));

    BOOST_CHECK_EQUAL("true", eval("'abcd test'.include? 'abcd'"));
    BOOST_CHECK_EQUAL("true", eval("'abcd test'.include? 'test'"));
    BOOST_CHECK_EQUAL("false", eval("'abcd test'.include? 'tex'"));
}

BOOST_AUTO_TEST_CASE(convert)
{
    BOOST_CHECK_EQUAL("\"Hello woRLD\"", eval("'hello woRLD'.capitalize"));
    BOOST_CHECK_EQUAL("\"Hello woRLD\"", eval("'Hello woRLD'.capitalize"));
    BOOST_CHECK_EQUAL("\"hello world\"", eval("'hello woRLD'.downcase"));
    BOOST_CHECK_EQUAL("\"HELLO WORLD\"", eval("'hello woRLD'.upcase"));

    BOOST_CHECK_EQUAL("0", eval("'test'.hex"));
    BOOST_CHECK_EQUAL("0", eval("'0'.hex"));
    BOOST_CHECK_EQUAL("0", eval("'0x'.hex"));
    BOOST_CHECK_EQUAL("15", eval("'0xf'.hex"));
    BOOST_CHECK_EQUAL("10", eval("'0x0a'.hex"));
    BOOST_CHECK_EQUAL("26", eval("'0x01A bar'.hex"));
    BOOST_CHECK_EQUAL("-36", eval("'-24'.hex"));
}

BOOST_AUTO_TEST_CASE(size)
{
    BOOST_CHECK_EQUAL("0", eval("''.size"));
    BOOST_CHECK_EQUAL("0", eval("''.length"));
    BOOST_CHECK_EQUAL("0", eval("''.bytesize"));

    BOOST_CHECK_EQUAL("4", eval("'test'.size"));
    BOOST_CHECK_EQUAL("4", eval("'test'.length"));
    BOOST_CHECK_EQUAL("4", eval("'test'.bytesize"));

    BOOST_CHECK_EQUAL("2", eval("'\xC2\xA3'.size"));
    BOOST_CHECK_EQUAL("2", eval("'\xC2\xA3'.length"));
    BOOST_CHECK_EQUAL("2", eval("'\xC2\xA3'.bytesize")); //GBP � U+00A3

    BOOST_CHECK_EQUAL("true", eval("''.empty?"));
    BOOST_CHECK_EQUAL("false", eval("'test'.empty?"));
}
BOOST_AUTO_TEST_CASE(slice)
{
    //slice index
    BOOST_CHECK_EQUAL("\"s\"", eval("'test'.slice 2"));
    BOOST_CHECK_EQUAL("\"s\"", eval("'test'[2]"));
    BOOST_CHECK_EQUAL("\"t\"", eval("'test'[-1]"));
    BOOST_CHECK_EQUAL("\"\"", eval("'test'[4]"));
    BOOST_CHECK_EQUAL("nil", eval("'test'[-5]"));
    BOOST_CHECK_EQUAL("nil", eval("'test'[5]"));
    //slice start, length
    BOOST_CHECK_EQUAL("\"st\"", eval("'test'.slice 2, 2"));
    BOOST_CHECK_EQUAL("\"st\"", eval("'test'[2, 2]"));
    BOOST_CHECK_EQUAL("\"t\"", eval("'test'[3, 2]"));
    BOOST_CHECK_EQUAL("\"es\"", eval("'test'[-3, 2]"));
    BOOST_CHECK_EQUAL("\"\"", eval("'test'[4, 2]"));
    BOOST_CHECK_EQUAL("nil", eval("'test'[-5, 2]"));
    BOOST_CHECK_EQUAL("nil", eval("'test'[5, 2]"));
    BOOST_CHECK_EQUAL("\"\"", eval("'test'[2, 0]"));
    BOOST_CHECK_EQUAL("nil", eval("'test'[2, -1]"));
    //slice range
    BOOST_CHECK_EQUAL("\"es\"", eval("'test'.slice 1..2"));
    BOOST_CHECK_EQUAL("\"es\"", eval("'test'.slice 1...3"));
    BOOST_CHECK_EQUAL("\"es\"", eval("'test'[1..2]"));
    BOOST_CHECK_EQUAL("\"t\"", eval("'test'[3...5]"));
    BOOST_CHECK_EQUAL("\"es\"", eval("'test'[-3...-1]"));
    BOOST_CHECK_EQUAL("\"\"", eval("'test'[4...6]"));
    BOOST_CHECK_EQUAL("nil", eval("'test'[-5...-3]"));
    BOOST_CHECK_EQUAL("nil", eval("'test'[5...7]"));
    BOOST_CHECK_EQUAL("\"\"", eval("'test'[2...2]"));
    BOOST_CHECK_EQUAL("\"s\"", eval("'test'[2...-1]"));
    //slice match_str
    BOOST_CHECK_EQUAL("\"es\"", eval("'test'.slice 'es'"));
    BOOST_CHECK_EQUAL("\"es\"", eval("'test'['es']"));
    BOOST_CHECK_EQUAL("nil", eval("'test'['esx']"));
    //slice regex
    BOOST_CHECK_EQUAL("\"world\"", eval("'hello world'[/world/]"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'[/test/]"));
    BOOST_CHECK_EQUAL("\"world\"", eval("'hello world'[/w(orl)d/, 0]"));
    BOOST_CHECK_EQUAL("\"orl\"", eval("'hello world'[/w(orl)d/, 1]"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'[/w(orl)d/, 2]"));
    BOOST_CHECK_EQUAL("nil", eval("'hello'[/w(orl)d/, 2]"));


    BOOST_CHECK_THROW(eval("'test'[true]"), ScriptError);
    BOOST_CHECK_THROW(eval("'test'[5, true]"), ScriptError);
    BOOST_CHECK_THROW(eval("'test'[true, 5]"), ScriptError);
    BOOST_CHECK_THROW(eval("'test'[1, 2, 3]"), ScriptError);
}

BOOST_AUTO_TEST_CASE(justify)
{
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.center 0"));
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.center 4"));
    BOOST_CHECK_EQUAL("\"test \"", eval("'test'.center 5"));
    BOOST_CHECK_EQUAL("\" test \"", eval("'test'.center 6"));
    BOOST_CHECK_EQUAL("\"   test   \"", eval("'test'.center 10"));
    BOOST_CHECK_EQUAL("\"---test---\"", eval("'test'.center 10, '-'"));
    BOOST_CHECK_EQUAL("\"-=-test-=-\"", eval("'test'.center 10, '-='"));


    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.ljust 4"));
    BOOST_CHECK_EQUAL("\"test \"", eval("'test'.ljust 5"));
    BOOST_CHECK_EQUAL("\"test-=-\"", eval("'test'.ljust 7, '-='"));

    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.rjust 4"));
    BOOST_CHECK_EQUAL("\" test\"", eval("'test'.rjust 5"));
    BOOST_CHECK_EQUAL("\"-=-test\"", eval("'test'.rjust 7, '-='"));
}

BOOST_AUTO_TEST_CASE(chomp)
{
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.chomp"));
    BOOST_CHECK_EQUAL("\"test\\nx\"", eval("'test\nx'.chomp"));
    
    BOOST_CHECK_EQUAL("\"test\"", eval("'test\\r'.chomp"));
    BOOST_CHECK_EQUAL("\"test\"", eval("'test\\n'.chomp"));
    BOOST_CHECK_EQUAL("\"test\"", eval("'test\\r\\n'.chomp"));
    BOOST_CHECK_EQUAL("\"test\\n\"", eval("'test\\n\\r'.chomp"));
    BOOST_CHECK_EQUAL("\"test\\n\"", eval("'test\\n\\n'.chomp"));

    BOOST_CHECK_EQUAL("\"test\\r\"", eval("'test\\r'.chomp '\\n'"));
    BOOST_CHECK_EQUAL("\"test\"", eval("'test\\n'.chomp '\\n'"));
    BOOST_CHECK_EQUAL("\"test\\n\"", eval("'test\\n\\n'.chomp '\\n'"));

    BOOST_CHECK_EQUAL("\"test\"", eval("'test\\n\\n'.chomp ''"));
    BOOST_CHECK_EQUAL("\"test\"", eval("'test\\n\\n\\r'.chomp ''"));

    BOOST_CHECK_THROW(eval("''.chomp 1, 2"), ArgumentError);
}

BOOST_AUTO_TEST_CASE(strip)
{
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.strip"));
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.lstrip"));
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.rstrip"));

    BOOST_CHECK_EQUAL("\"\"", eval("' '.strip"));
    BOOST_CHECK_EQUAL("\"\"", eval("'  '.lstrip"));
    BOOST_CHECK_EQUAL("\"\"", eval("' \r'.rstrip"));

    BOOST_CHECK_EQUAL("\"test\"", eval("'\\r\\n\\t \ntest\\r\\n\\t \\n'.strip"));
    BOOST_CHECK_EQUAL("\"test\\r\\n\\t \\n\"", eval("'\\r\\n\\t \\ntest\\r\\n\\t \\n'.lstrip"));
    BOOST_CHECK_EQUAL("\"\\r\\n\\t \\ntest\"", eval("'\\r\\n\\t \\ntest\\r\\n\\t \\n'.rstrip"));
}

BOOST_AUTO_TEST_CASE(ord)
{
    BOOST_CHECK_THROW(eval("''.ord"), ScriptError);
    BOOST_CHECK_EQUAL("97", eval("'a'.ord"));
    BOOST_CHECK_EQUAL("97", eval("'ax'.ord"));
}

BOOST_AUTO_TEST_CASE(reverse)
{
    BOOST_CHECK_EQUAL("\"dlrow olleh\"", eval("'hello world'.reverse"));
}

BOOST_AUTO_TEST_CASE(lines)
{
    BOOST_CHECK_EQUAL("[\"hello world\"]", eval("'hello world'.lines"));
    BOOST_CHECK_EQUAL("[\"hello\\n\", \"world\"]", eval("'hello\nworld'.lines"));

    BOOST_CHECK_EQUAL("[\"hello\\n\", \"world\\n\", \"\\n\", \"lines\"]", eval("'hello\\nworld\\n\\nlines'.lines"));
    BOOST_CHECK_EQUAL("[\"hello\\n\", \"world\\n\", \"\\n\", \"lines\\n\"]", eval("'hello\\nworld\\n\\nlines\\n'.lines"));

    BOOST_CHECK_EQUAL("[\"hello \", \"world\\n\\nlines\\n\"]", eval("'hello world\\n\\nlines\\n'.lines ' '"));

    BOOST_CHECK_EQUAL("[\"hello\\nworld\\n\\n\", \"lines\\n\"]", eval("'hello\\nworld\\n\\nlines\\n'.lines ''"));

    BOOST_CHECK_EQUAL("[\"hello\\nworld\\n\\n\\n\", \"lines\\n\"]", eval("'hello\\nworld\\n\\n\\nlines\\n'.lines ''"));
}

BOOST_AUTO_TEST_CASE(index)
{
    BOOST_CHECK_EQUAL("2", eval("'hello world'.index 'l'"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'.index 'x'"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'.index 'llx'"));
    BOOST_CHECK_EQUAL("2", eval("'hello world'.index 'l', 2"));
    BOOST_CHECK_EQUAL("2", eval("'hello world'.index 'll', 2"));
    BOOST_CHECK_EQUAL("3", eval("'hello world'.index 'l', 3"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'.index 'll', 3"));
    BOOST_CHECK_EQUAL("9", eval("'hello world'.index 'l', 4"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'.index 'l', 10"));
    BOOST_CHECK_EQUAL("4", eval("'test'.index '', 4"));

    BOOST_CHECK_EQUAL("9", eval("'hello world'.rindex 'l'"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'.rindex 'x'"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'.rindex 'llx'"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'.rindex 'l', 1"));
    BOOST_CHECK_EQUAL("2", eval("'hello world'.rindex 'l', 2"));
    BOOST_CHECK_EQUAL("2", eval("'hello world'.rindex 'll', 2"));
    BOOST_CHECK_EQUAL("3", eval("'hello world'.rindex 'l', 3"));
    BOOST_CHECK_EQUAL("2", eval("'hello world'.rindex 'll', 3"));
    BOOST_CHECK_EQUAL("9", eval("'hello world'.rindex 'l', 9"));
    BOOST_CHECK_EQUAL("9", eval("'hello world'.rindex 'l', 10"));
    BOOST_CHECK_EQUAL("4", eval("'test'.rindex '', 4"));

    BOOST_CHECK_EQUAL("9", eval("'hello world'.rindex 'l', 20"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'.rindex 'l', -20"));
    //regex
    BOOST_CHECK_EQUAL("2", eval("'hello world'.index(/l/)"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'.index 'x'"));
    BOOST_CHECK_EQUAL("2", eval("'hello world'.index(/l/, 2)"));
    BOOST_CHECK_EQUAL("4", eval("'test'.index(//, 4)"));

    BOOST_CHECK_EQUAL("9", eval("'hello world'.rindex(/l/)"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'.rindex 'x'"));
    BOOST_CHECK_EQUAL("2", eval("'hello world'.rindex(/l/, 2)"));
    BOOST_CHECK_EQUAL("4", eval("'test'.rindex(//, 4)"));
    //invalid
    BOOST_CHECK_THROW(eval("''.index 5"), ScriptError);
    BOOST_CHECK_THROW(eval("''.rindex 5"), ScriptError);
}

BOOST_AUTO_TEST_CASE(each_line)
{
    auto model = create_view_model();
    auto data = create_object<TestAccumulator>();
    model->set_attr("data", data);

    eval(model, "'test'.each_line{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[\"test\"]", data->check());

    eval(model, "'test\\nline'.each_line{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[\"test\\n\", \"line\"]", data->check());

    eval(model, "'test\\nline'.each_line ',' {|x| @data.store x}");
    BOOST_CHECK_EQUAL("[\"test\\nline\"]", data->check());

    eval(model, "'test\\nline'.each_line '' {|x| @data.store x}");
    BOOST_CHECK_EQUAL("[\"test\\nline\"]", data->check());

    eval(model, "'test\\n\\n\\nline'.each_line '' {|x| @data.store x}");
    BOOST_CHECK_EQUAL("[\"test\\n\\n\\n\", \"line\"]", data->check());

    eval(model, "'test,line'.each_line ',' {|x| @data.store x}");
    BOOST_CHECK_EQUAL("[\"test,\", \"line\"]", data->check());

    //enumerator test
    eval(model, "'test'.each_line.each{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[\"test\"]", data->check());

    eval(model, "'test,csv'.each_line(',').each{|x| @data.store x}");
    BOOST_CHECK_EQUAL("[\"test,\", \"csv\"]", data->check());

    eval(model, "'test,csv'.each_line.each ',' {|x| @data.store x}");
    BOOST_CHECK_EQUAL("[\"test,\", \"csv\"]", data->check());

    BOOST_CHECK_THROW(eval(model, "'test,csv'.each_line('\\n').each ',' {|x| @data.store x}"), ArgumentCountError);
}

BOOST_AUTO_TEST_CASE(match)
{
    BOOST_CHECK_EQUAL("\"world\"", eval("'hello world'.match('world').to_s"));
    BOOST_CHECK_EQUAL("\"world\"", eval("'hello world'.match(/world/).to_s"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'.match('test')"));

    BOOST_CHECK_EQUAL("\"world\"", eval("'hello world'.match('world', 6).to_s"));
    BOOST_CHECK_EQUAL("\"world\"", eval("'hello world'.match(/world/, 6).to_s"));
    BOOST_CHECK_EQUAL("nil", eval("'hello world'.match('world', 7)"));
}

BOOST_AUTO_TEST_CASE(partition)
{
    BOOST_CHECK_EQUAL("[\"hello\", \" \", \"world\"]", eval("'hello world'.partition ' '"));
    BOOST_CHECK_EQUAL("[\"hello world\", \"\", \"\"]", eval("'hello world'.partition 'x'"));

    BOOST_CHECK_EQUAL("[\"hello\", \" \", \"world test\"]", eval("'hello world test'.partition ' '"));
    BOOST_CHECK_EQUAL("[\"hello world\", \" \", \"test\"]", eval("'hello world test'.rpartition ' '"));
    BOOST_CHECK_EQUAL("[\"\", \"\", \"hello world\"]", eval("'hello world'.rpartition 'x'"));

    BOOST_CHECK_EQUAL("[\"hello\", \" \", \"world\"]", eval("'hello world'.partition(/ /)"));
    BOOST_CHECK_EQUAL("[\"hello test\\nworld\", \" \", \"test\"]", eval("'hello test\\nworld test'.rpartition(/ /)"));
    BOOST_CHECK_EQUAL("[\"\", \"\", \"hello test\\nworld test\"]", eval("'hello test\\nworld test'.rpartition(/^ /)"));
    BOOST_CHECK_EQUAL("[\"hello test\\n\", \"test\", \" world\"]", eval("'hello test\\ntest world'.rpartition(/^test/)"));
}

BOOST_AUTO_TEST_CASE(split)
{
    //empty
    BOOST_CHECK_EQUAL("[\"t\", \"e\", \"s\", \"t\"]", eval("'test'.split('')"));
    BOOST_CHECK_EQUAL("[\"t\", \"e\", \"s\", \"t\"]", eval("'test'.split(//)"));
    BOOST_CHECK_EQUAL("[\"t\", \"e\", \"st\"]", eval("'test'.split('', 3)"));
    BOOST_CHECK_EQUAL("[\"t\", \"e\", \"st\"]", eval("'test'.split(//, 3)"));
    //strings
    BOOST_CHECK_EQUAL("[\"unit\", \"test\", \"string\", \"split\"]", eval("'unit  test\\nstring\\tsplit'.split"));
    BOOST_CHECK_EQUAL("[\"unit\", \"test\", \"string\", \"split\"]", eval("'unit  test\\nstring\\tsplit'.split ' '"));
    BOOST_CHECK_EQUAL("[\"\", \"es\"]", eval("'test'.split 't'"));
    BOOST_CHECK_EQUAL("[\"1\", \"2\", \"3\", \"\", \"5\"]", eval("'1,2,3,,5,'.split ','"));
    BOOST_CHECK_EQUAL("[\"\", \"es\", \"\"]", eval("'test'.split 't', -1"));
    BOOST_CHECK_EQUAL("[\"test\"]", eval("'test'.split 't', 1"));
    BOOST_CHECK_EQUAL("[\"test\"]", eval("'test'.split '', 1"));
    BOOST_CHECK_EQUAL("[\"1\", \"2\", \"3\", \"\", \"5\", \"\"]", eval("'1,2,3,,5,'.split ',', -1"));
    BOOST_CHECK_EQUAL("[\"1\", \"2\", \"3,,5,\"]", eval("'1,2,3,,5,'.split ',', 3"));
    BOOST_CHECK_EQUAL("[\"1\", \"2\", \"3\", \"\", \"5,\"]", eval("'1,2,3,,5,'.split ',', 5"));
    BOOST_CHECK_EQUAL("[\"1\", \"2\", \"3\", \"\", \"5\"]", eval("'1,2,3,,5,'.split ',', 10"));
    //regex
    BOOST_CHECK_EQUAL("[\"test\"]", eval("'test'.split(/ /)"));
    BOOST_CHECK_EQUAL("[]", eval("'test'.split(/.*/)"));
    BOOST_CHECK_EQUAL("[\"test\", \"test\"]", eval("'test test'.split(/ /)"));
    BOOST_CHECK_EQUAL("[\"\", \"test\", \"test\"]", eval("' test test'.split(/ /)"));

    BOOST_CHECK_EQUAL("[\"\", \"test\", \"test\"]", eval("' test test'.split(/ /, 0)"));
    BOOST_CHECK_EQUAL("[\" test test\"]", eval("' test test'.split(/ / , 1)"));
    BOOST_CHECK_EQUAL("[\"\", \"test test\"]", eval("' test test'.split(/ / , 2)"));
    //regex captures
    BOOST_CHECK_EQUAL("[\"\", \"test\"]", eval("'test'.split(/(.*)/)"));
    BOOST_CHECK_EQUAL("[\"\", \"test\", \"\"]", eval("'test'.split(/(.*)/, -1)"));
    BOOST_CHECK_EQUAL("[\"test\"]", eval("'test'.split(/(.)(.)/, 1)"));
    BOOST_CHECK_EQUAL("[\"\", \"t\", \"e\", \"st\"]", eval("'test'.split(/(.)(.)/, 2)"));
    //invalid
    BOOST_CHECK_THROW(eval("''.split 5"), ScriptError);

}

BOOST_AUTO_TEST_CASE(sub)
{
    //string with string
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.sub 'x', '5'"));
    BOOST_CHECK_EQUAL("\"test 5 x\"", eval("'test x x'.sub 'x', '5'"));
    BOOST_CHECK_EQUAL("\"test -x-- x\"", eval("'test x x'.sub 'x', '-\\\\0-\\\\1-'"));
    //string with hash
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.sub 'x', {'x' => 10}"));
    BOOST_CHECK_EQUAL("\"test 10\"", eval("'test x'.sub 'x', {'x' => 10}"));
    BOOST_CHECK_EQUAL("\"test \"", eval("'test x'.sub 'x', {'y' => 10}"));
    //string with block
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.sub 'x' {|str| \"-#{str}-\"}"));
    BOOST_CHECK_EQUAL("\"test -x- x\"", eval("'test x x'.sub 'x' {|str| \"-#{str}-\"}"));
    //regex with string
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.sub(/[0-9]+/, '5')"));
    BOOST_CHECK_EQUAL("\"test 5 100\"", eval("'test 70 100'.sub(/[0-9]+/, '5')"));
    BOOST_CHECK_EQUAL("\"test -70- 100\"", eval("'test 70 100'.sub(/[0-9]+/, '-\\\\0-')"));
    BOOST_CHECK_EQUAL("\"test -7:0- 100\"", eval("'test 70 100'.sub(/([0-9])([0-9]*)/, '-\\\\1:\\\\2-')"));
    //regex with hash
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.sub(/x/, {'x' => 55})"));
    BOOST_CHECK_EQUAL("\"test 55 x\"", eval("'test x x'.sub(/x/, {'x' => 55})"));
    //regex with block
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.sub(/x/){|str| \"-#{str}-\"}"));
    BOOST_CHECK_EQUAL("\"test -x- x\"", eval("'test x x'.sub(/x/){|str| \"-#{str}-\"}"));
    //invalid args
    BOOST_CHECK_THROW(eval("'test x'.sub 'x', 5"), ScriptError);
    BOOST_CHECK_THROW(eval("'test x'.sub 5, '5'"), ScriptError);
}
BOOST_AUTO_TEST_CASE(gsub)
{
    //string with string
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.gsub 'x', '5'"));
    BOOST_CHECK_EQUAL("\"test -x-- -x--\"", eval("'test x x'.gsub 'x', '-\\\\0-\\\\1-'"));
    //string with hash
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.gsub 'x', {'x' => 10}"));
    BOOST_CHECK_EQUAL("\"test 10 10\"", eval("'test x x'.gsub 'x', {'x' => 10}"));
    BOOST_CHECK_EQUAL("\"test  \"", eval("'test x x'.gsub 'x', {'y' => 10}"));
    //string with block
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.gsub 'x' {|str| \"-#{str}-\"}"));
    BOOST_CHECK_EQUAL("\"test -x- -x-\"", eval("'test x x'.gsub 'x' {|str| \"-#{str}-\"}"));
    //regex with string
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.gsub(/[0-9]+/, '5')"));
    BOOST_CHECK_EQUAL("\"test 5 5\"", eval("'test 70 100'.gsub(/[0-9]+/, '5')"));
    BOOST_CHECK_EQUAL("\"test -70- -100-\"", eval("'test 70 100'.gsub(/[0-9]+/, '-\\\\0-')"));
    BOOST_CHECK_EQUAL("\"test -7:0- -1:00-\"", eval("'test 70 100'.gsub(/([0-9])([0-9]*)/, '-\\\\1:\\\\2-')"));
    //regex with hash
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.gsub(/x/, {'x' => 55})"));
    BOOST_CHECK_EQUAL("\"test 55 55\"", eval("'test x x'.gsub(/x/, {'x' => 55})"));
    //regex with block
    BOOST_CHECK_EQUAL("\"test\"", eval("'test'.gsub(/x/){|str| \"-#{str}-\"}"));
    BOOST_CHECK_EQUAL("\"test -x- -x-\"", eval("'test x x'.gsub(/x/){|str| \"-#{str}-\"}"));
    //invalid args
    BOOST_CHECK_THROW(eval("'test x'.gsub 'x', 5"), ScriptError);
    BOOST_CHECK_THROW(eval("'test x'.gsub 5, '5'"), ScriptError);
}

BOOST_AUTO_TEST_SUITE_END()

