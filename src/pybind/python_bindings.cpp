//
// Created by matthew on 5/24/25.
//


#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "pybind/pybind_buffer.h"
#include "pybind/pybind_convert.h"

#include "interpreter/interpreter.h"
#include "interpreter/memory.h"
#include "parser/parser.h"
#include "tokenizer/tokenizer.h"

namespace py = pybind11;


class InterpreterWrapper {
    std::unique_ptr<PyBytesIOBuf> ibuf_;
    std::unique_ptr<PyBytesIOBuf> obuf_;
    std::shared_ptr<std::istream> istream_;
    std::unique_ptr<std::ostream> ostream_;
    std::unique_ptr<Interpreter> obj_;

public:
    InterpreterWrapper(const py::object& istream, const py::object& ostream) :
        ibuf_(std::make_unique<PyBytesIOBuf>(istream)),
        obuf_(std::make_unique<PyBytesIOBuf>(ostream)),
        istream_(std::make_shared<std::istream>(ibuf_.get())),
        ostream_(std::make_unique<std::ostream>(obuf_.get())),
        obj_(std::make_unique<Interpreter>(*istream_, *ostream_)) {}

    void initProgram(const MemLayout& layout) const { obj_->initProgram(layout); }
    void step() const { obj_->step(); }
    [[nodiscard]] int interpret(const MemLayout& layout) const { return obj_->interpret(layout); }
};


PYBIND11_MODULE(pymasm, m) {
    m.doc() = "MIPS Assembly Interpreter Library";

    const py::module_ tokenizer_module = m.def_submodule("tokenizer", "Masm Tokenizer");
    const py::module_ parser_module = m.def_submodule("parser", "Masm Parser");
    const py::module_ interpreter_module = m.def_submodule("interpreter", "Masm Interpreter");

    // Tokenizer Bindings //

    // Binding for the TokenType enum
    py::enum_<TokenType>(tokenizer_module, "TokenType")
            .value("UNKNOWN", TokenType::UNKNOWN)
            .value("SEC_DIRECTIVE", TokenType::SEC_DIRECTIVE)
            .value("ALLOC_DIRECTIVE", TokenType::ALLOC_DIRECTIVE)
            .value("META_DIRECTIVE", TokenType::META_DIRECTIVE)
            .value("LABEL_DEF", TokenType::LABEL_DEF)
            .value("LABEL_REF", TokenType::LABEL_REF)
            .value("INSTRUCTION", TokenType::INSTRUCTION)
            .value("REGISTER", TokenType::REGISTER)
            .value("IMMEDIATE", TokenType::IMMEDIATE)
            .value("SEPERATOR", TokenType::SEPERATOR)
            .value("OPEN_PAREN", TokenType::OPEN_PAREN)
            .value("CLOSE_PAREN", TokenType::CLOSE_PAREN)
            .value("STRING", TokenType::STRING)
            .value("MACRO_PARAM", TokenType::MACRO_PARAM);

    // Binding for the RawFile struct
    py::class_<RawFile>(tokenizer_module, "RawFile")
            .def(py::init<>())
            .def(py::init<const std::string&, const std::vector<std::string>&>(), py::arg("name"),
                 py::arg("lines"))
            .def_readwrite("name", &RawFile::name)
            .def_readwrite("lines", &RawFile::lines)
            .def("__repr__", [](const RawFile& rf) {
                return "<RawFile(name='" + rf.name + "', lines=" + std::to_string(rf.lines.size()) +
                       ")>";
            });

    // Binding for the Token struct
    py::class_<Token>(tokenizer_module, "Token")
            .def(py::init<>())
            .def(py::init<TokenType, const std::string&>(), py::arg("type"), py::arg("value"))
            .def_readwrite("type", &Token::type)
            .def_readwrite("value", &Token::value)
            .def("__repr__",
                 [](const Token& t) {
                     return "<Token(type=" + tokenTypeToString(t.type) + ", value='" + t.value +
                            "')>";
                 })
            .def("__str__", [](const Token& t) { return t.value; })
            .def(py::self == py::self) // Bind the equality operator
            .def(py::self != py::self); // Bind the inequality operator

    // Binding for the SourceLine struct
    py::class_<SourceLine>(tokenizer_module, "SourceLine")
            .def(py::init<>())
            .def(py::init<size_t, const std::vector<Token>&>(), py::arg("lineno"),
                 py::arg("tokens"))
            .def_readwrite("lineno", &SourceLine::lineno)
            .def_readwrite("tokens", &SourceLine::tokens)
            .def("__repr__", [](const SourceLine& sl) {
                std::string concatLine;
                for (const Token& token : sl.tokens)
                    concatLine += token.value + " ";

                // Remove trailing space
                if (!concatLine.empty())
                    concatLine.pop_back();

                return "<SourceLine(lineno=" + std::to_string(sl.lineno) + ", tokens='" +
                       concatLine + "')>";
            });

    // Binding for the Tokenizer class
    py::class_<Tokenizer>(tokenizer_module, "Tokenizer")
            // Constructors
            .def(py::init<>())
            .def_static("tokenize_file", &Tokenizer::tokenizeFile, py::arg("raw_file"),
                        "Tokenizes the given file and returns a vector of SourceLine objects")
            .def_static("tokenize", &Tokenizer::tokenize, py::arg("raw_files"),
                        "Tokenizes the given raw files and returns a vector of SourceLine objects");

    // Parser Bindings //

    // Binding for the MemSection enum
    py::enum_<MemSection>(parser_module, "MemSection")
            .value("TEXT", MemSection::TEXT)
            .value("DATA", MemSection::DATA)
            .value("HEAP", MemSection::HEAP)
            .value("KTEXT", MemSection::KTEXT)
            .value("KDATA", MemSection::KDATA)
            .value("MMIO", MemSection::MMIO);

    py::bind_map<MemLayout>(parser_module, "MemLayout", "Memory layout mapping");

    // Binding for the Parser Class
    py::class_<Parser>(parser_module, "Parser")
            .def(py::init<>())
            .def("parse", &Parser::parse, py::arg("program"),
                 "Parses the given program and returns a MemLayout object");

    // Interpreter Bindings //

    // Bindings for the Interpreter class
    py::class_<InterpreterWrapper>(interpreter_module, "Interpreter")
            // Constructor that accepts Python file-like objects
            .def(py::init<py::object, py::object>())
            .def("step", &InterpreterWrapper::step, "Executes a single instruction")
            .def("interpret", &InterpreterWrapper::interpret, py::arg("layout"),
                 "Interprets the given memory layout and returns an exit code");
}
