#pragma once

#include "utility/std.hpp"
#include "utility/string.hpp"


namespace kh {
    /* AST expression types for labelling the inheritance type */
    enum class AstExpressionType {
        NOTYPE, IDENTIFIER,
        INTEGER, UNSIGNED_INTEGER, FLOATING, COMPLEX, CHARACTER, STRING, BUFFER, TUPLE, ARRAY,
        CALL, TEMPLATIZE, DECLARE, FUNCTION,
        UNARY, BINARY, TERNARY,
        IF, WHILE, DO_WHILE, FOR, CASE, DEFAULT, SWITCH,
        IMPORT, INCLUDE, CLASS, STRUCT, ENUM
    };

    enum class AstUnaryOperationType {
        UPOS, USUB,
        BIT_NOT, NOT,
        INCREMENT, DECREMENT, SIZEOF
    };

    enum class AstBinaryOperationType {
        ADD, SUB, MUL, DIV, MOD, POW,
        IADD, ISUB, IMUL, IDIV, IMOD, IPOW,
        BIT_AND, BIT_OR, BIT_LSHIFT, BIT_RSHIFT,
        EQUAL, NOT_EQUAL, LESS, MORE, LESS_EQUAL, MORE_EQUAL,
        AND, OR,
        ASSIGN, SUBSCRIPT, MEMBER_SCOPE, SCOPE
    };

    enum class Access {
        PUBLIC, PRIVATE, PROTECTED
    };

    enum class Condition {
        NONE, CONSTANT, REF
    };

    /* Base AST expression type */
    class AstExpression {
    public:
        kh::AstExpressionType type = kh::AstExpressionType::NOTYPE;

        AstExpression() {}
        virtual ~AstExpression() {}
    };

    class AstIdentifier : public kh::AstExpression {
    public:
        kh::String identifier;

        AstIdentifier(const kh::String& _identifier) :
            identifier(_identifier) {
            this->type = kh::AstExpressionType::IDENTIFIER;
        }
        virtual ~AstIdentifier() {}
    };

    /* Constant expression values */

    class AstInteger : public kh::AstExpression {
    public:
        int64 integer = 0;

        AstInteger(const int64 _integer) :
            integer(_integer) {
            this->type = kh::AstExpressionType::INTEGER;
        }
        virtual ~AstInteger() {}
    };

    class AstUnsignedInteger : public kh::AstExpression {
    public:
        uint64 unsigned_integer = 0;

        AstUnsignedInteger(const uint64 _unsigned_integer) :
            unsigned_integer(_unsigned_integer) {
            this->type = kh::AstExpressionType::UNSIGNED_INTEGER;
        }
        virtual ~AstUnsignedInteger() {}
    };

    class AstFloating : public kh::AstExpression {
    public:
        double floating = 0.0;

        AstFloating(const double _floating) :
            floating(_floating) {
            this->type = kh::AstExpressionType::FLOATING;
        }
        virtual ~AstFloating() {}
    };

    class AstComplex : public kh::AstExpression {
    public:
        complex floating_complex = 0.0;

        AstComplex(const complex _floating_complex) :
            floating_complex(_floating_complex) {
            this->type = kh::AstExpressionType::COMPLEX;
        }
        virtual ~AstComplex() {}
    };

    class AstCharacter : public kh::AstExpression {
    public:
        uint32 character = 0;

        AstCharacter(const uint32 _character) :
            character(_character) {
            this->type = kh::AstExpressionType::CHARACTER;
        }
        virtual ~AstCharacter() {}
    };

    class AstString : public kh::AstExpression {
    public:
        kh::String string;

        AstString(const kh::String& _string) :
            string(_string) {
            this->type = kh::AstExpressionType::STRING;
        }
        virtual ~AstString() {}
    };

    class AstBuffer : public kh::AstExpression {
    public:
        std::vector<uint8> buffer;

        AstBuffer(const std::vector<uint8>& _buffer) :
            buffer(_buffer) {
            this->type = kh::AstExpressionType::BUFFER;
        }
        virtual ~AstBuffer() {}
    };

    class AstTuple : public kh::AstExpression {
    public:
        std::vector<kh::AstExpression*> elements;

        AstTuple(std::vector<kh::AstExpression*>& _elements) :
            elements(_elements) {
            this->type = kh::AstExpressionType::TUPLE;
        }
        virtual ~AstTuple() {
            for (kh::AstExpression* element : this->elements)
                delete element;
        }
    };

    class AstArray : public kh::AstExpression {
    public:
        std::vector<kh::AstExpression*> elements;

        AstArray(std::vector<kh::AstExpression*>& _elements) :
            elements(_elements) {
            this->type = kh::AstExpressionType::ARRAY;
        }
        virtual ~AstArray() {
            for (kh::AstExpression* element : this->elements)
                delete element;
        }
    };

    /* Function/method calls, templatizing, and declaring  */

    class AstCall : public kh::AstExpression {
    public:
        kh::AstExpression* function;
        kh::AstTuple* tuple;

        AstCall(kh::AstExpression* _function, kh::AstTuple* _tuple) :
            function(_function), tuple(_tuple) {
            this->type = kh::AstExpressionType::CALL;
        }
        virtual ~AstCall() {
            delete this->function;
            delete this->tuple;
        }
    };

    class AstTemplatize : public kh::AstExpression {
    public:
        kh::AstExpression* type_class;
        kh::AstTuple* tuple;

        AstTemplatize(kh::AstExpression* _type_class, kh::AstTuple* _tuple) :
            type_class(_type_class), tuple(_tuple) {
            this->type = kh::AstExpressionType::TEMPLATIZE;
        }
        virtual ~AstTemplatize() {
            delete this->type_class;
            delete this->tuple;
        }
    };

    class AstDeclare : public kh::AstExpression {
    public:
        kh::Access access;
        kh::Condition condition;
        bool is_static;

        kh::AstExpression* type_class;
        kh::String variable_name;
        kh::AstExpression* expression;

        AstDeclare(kh::AstExpression* _type_class, const kh::String& _variable_name, kh::AstExpression* _expression,
                const kh::Access _access, const kh::Condition _condition, const bool _is_static) :
            type_class(_type_class), variable_name(_variable_name), expression(_expression),
            access(_access), condition(_condition), is_static(_is_static)
            {
            this->type = kh::AstExpressionType::DECLARE;
        }
        virtual ~AstDeclare() {
            delete this->type_class;
            delete this->expression;
        }
    };

    /* Unary and binary operations */

    class AstUnaryOp : public kh::AstExpression {
    public:
        kh::AstUnaryOperationType op_type;
        kh::AstExpression* a;
        kh::AstExpression* b;

        AstUnaryOp(const kh::AstUnaryOperationType _op_type, kh::AstExpression* _a, kh::AstExpression* _b) :
            op_type(_op_type), a(_a), b(_b) {
            this->type = kh::AstExpressionType::UNARY;
        }
        virtual ~AstUnaryOp() {
            delete this->a;
            delete this->b;
        }
    };

    class AstBinaryOp : public kh::AstExpression {
    public:
        kh::AstBinaryOperationType op_type;
        kh::AstExpression* a;
        kh::AstExpression* b;

        AstBinaryOp(const kh::AstBinaryOperationType _op_type, kh::AstExpression* _a, kh::AstExpression* _b) :
            op_type(_op_type), a(_a), b(_b) {
            this->type = kh::AstExpressionType::BINARY;
        }
        virtual ~AstBinaryOp() {
            delete this->a;
            delete this->b;
        }
    };

    /* Ternary operator */
    class AstTernary : public kh::AstExpression {
    public:
        kh::AstExpression* condition;
        kh::AstExpression* action;
        kh::AstExpression* otherwise;

        AstTernary(kh::AstExpression* _condition, kh::AstExpression* _action, kh::AstExpression* _otherwise) :
            condition(_condition), action(_action), otherwise(_otherwise) {
            this->type = kh::AstExpressionType::TERNARY;
        }
        virtual ~AstTernary() {
            delete this->condition;
            delete this->action;
            delete this->otherwise;
        }
    };

    /* Logic flow structures */

    class AstIf : public kh::AstExpression {
    public:
        kh::AstExpression* condition;
        std::vector<kh::AstExpression*> body;
        std::vector<kh::AstExpression*> else_body;

        AstIf(kh::AstExpression* _condition, std::vector<kh::AstExpression*>& _body, std::vector<kh::AstExpression*>& _else_body) :
            condition(_condition), body(_body), else_body(_else_body) {
            this->type = kh::AstExpressionType::IF;
        }
        virtual ~AstIf() {
            delete this->condition;
            for (kh::AstExpression* expression : this->body)
                delete expression;
            for (kh::AstExpression* expression : this->else_body)
                delete expression;
        }
    };

    class AstWhile : public kh::AstExpression {
    public:
        kh::AstExpression* condition;
        std::vector<kh::AstExpression*> body;

        AstWhile(kh::AstExpression* _condition, std::vector<kh::AstExpression*>& _body) :
            condition(_condition), body(_body) {
            this->type = kh::AstExpressionType::WHILE;
        }
        virtual ~AstWhile() {
            delete this->condition;
            for (kh::AstExpression* expression : this->body)
                delete expression;
        }
    };

    class AstDoWhile : public kh::AstExpression {
    public:
        kh::AstExpression* condition;
        std::vector<kh::AstExpression*> body;

        AstDoWhile(kh::AstExpression* _condition, std::vector<kh::AstExpression*>& _body) :
            condition(_condition), body(_body) {
            this->type = kh::AstExpressionType::DO_WHILE;
        }
        virtual ~AstDoWhile() {
            delete this->condition;
            for (kh::AstExpression* expression : this->body)
                delete expression;
        }
    };

    class AstFor : public kh::AstExpression {
    public:
        kh::AstTuple* tuple;
        std::vector<kh::AstExpression*> body;

        AstFor(kh::AstTuple* _tuple, std::vector<kh::AstExpression*>& _body) :
            tuple(_tuple), body(_body) {
            this->type = kh::AstExpressionType::FOR;
        }
        virtual ~AstFor() {
            delete this->tuple;
            for (kh::AstExpression* expression : this->body)
                delete expression;
        }
    };
    
    class AstCase : public kh::AstExpression {
    public:
        kh::AstExpression* expression;
        std::vector<kh::AstExpression*> body;

        AstCase(kh::AstExpression* _expression, std::vector<kh::AstExpression*>& _body) :
            expression(_expression), body(_body) {
            this->type = kh::AstExpressionType::CASE;
        }
        virtual ~AstCase() {
            delete this->expression;
            for (kh::AstExpression* expression : this->body)
                delete expression;
        }
    };

    class AstDefault : public kh::AstExpression {
    public:
        std::vector<kh::AstExpression*> body;

        AstDefault(std::vector<kh::AstExpression*>& _body) :
            body(_body) {
            this->type = kh::AstExpressionType::DEFAULT;
        }
        virtual ~AstDefault() {
            for (kh::AstExpression* expression : this->body)
                delete expression;
        }
    };

    class AstSwitch : public kh::AstExpression {
    public:
        kh::AstExpression* expression;
        kh::AstDefault* default;
        std::vector<kh::AstCase*> cases;

        AstSwitch(kh::AstExpression* _expression, std::vector<kh::AstCase*>& _cases, kh::AstDefault* _default) :
            expression(_expression), cases(_cases), default(_default) {
            this->type = kh::AstExpressionType::SWITCH;
        }
        virtual ~AstSwitch() {
            delete this->expression;
            delete this->default;
            for (kh::AstCase* _case : this->cases)
                delete _case;
        }
    };

    /* Imports, functions, classes, structs, enums */

    class AstImport : public kh::AstExpression {
    public:
        kh::String path;
        kh::String identifier;
        bool relative;

        AstImport(const kh::String& _path, const kh::String& _identifier, const bool _relative) :
            path(_path), identifier(_identifier), relative(_relative) {
            this->type = kh::AstExpressionType::IMPORT;
        }
        virtual ~AstImport() {}
    };

    class AstInclude : public kh::AstExpression {
        kh::String path;
        bool relative;

        AstInclude(const kh::String& _path,  const bool _relative) :
            path(_path), relative(_relative) {
            this->type = kh::AstExpressionType::INCLUDE;
        }
        virtual ~AstInclude() {}
    };

    class AstFunction : public kh::AstExpression {
    public:
        kh::AstExpression* return_type;
        kh::String identifier;
        kh::AstTuple* arguments;
        std::vector<kh::AstExpression*> body;

        AstFunction(kh::AstExpression* _return_type, const kh::String& _identifier, kh::AstTuple* _arguments, std::vector<kh::AstExpression*>& _body) :
            return_type(_return_type), identifier(_identifier), arguments(_arguments), body(_body) {
            this->type = kh::AstExpressionType::FUNCTION;
        }
        virtual ~AstFunction() {
            delete this->return_type;
            delete this->arguments;
            for (kh::AstExpression* expression : this->body)
                delete expression;
        }
    };

    class AstClass : public kh::AstExpression {
    public:
        kh::String identifier;
        kh::AstTuple* templates;
        std::vector<kh::AstExpression*> body;

        AstClass(const kh::String& _identifier, kh::AstTuple* _templates, std::vector<kh::AstExpression*>& _body) :
            identifier(_identifier), templates(_templates), body(_body) {
            this->type = kh::AstExpressionType::CLASS;
        }
        virtual ~AstClass() {
            delete this->templates;
            for (kh::AstExpression* expression : this->body)
                delete expression;
        }
    };

    class AstStruct : public kh::AstExpression {
    public:
        kh::String identifier;
        std::vector<kh::AstExpression*> body;

        AstStruct(const kh::String& _identifier, std::vector<kh::AstExpression*>& _body) :
            identifier(_identifier), body(_body) {
            this->type = kh::AstExpressionType::STRUCT;
        }
        virtual ~AstStruct() {
            for (kh::AstExpression* expression : this->body)
                delete expression;
        }
    };

    class AstEnum : public kh::AstExpression {
    public:
        kh::String identifier;
        std::vector<kh::String> members;

        AstEnum(const kh::String& _identifier, std::vector<kh::String>& _members) :
            identifier(_identifier), members(_members) {
            this->type = kh::AstExpressionType::ENUM;
        }
        virtual ~AstEnum() {}
    };
}
