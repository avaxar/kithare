#pragma once

#include "utility/std.hpp"
#include "utility/string.hpp"


namespace kh {
    /* AST expression types for labelling the inheritance type */
    enum class AstExpressionType {
        NOTYPE, IDENTIFIER,
        INTEGER, UNSIGNED_INTEGER, FLOATING, COMPLEX, CHARACTER, STRING, BUFFER, TUPLE,
        CALL, INITIATE, TEMPLATIZE, INLINE_DECLARE, DECLARE,
        UNARY, BINARY,
        IF, WHILE, DO_WHILE, FOR, CASE, SWITCH
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
        ASSIGN, MEMBER_SCOPE, SCOPE
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

    /* Function/method calls, instantiating,  */

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

    class AstInitiate : public kh::AstExpression {
    public:
        kh::AstExpression* type_class;
        kh::AstTuple* tuple;

        AstInitiate(kh::AstExpression* _type_class, kh::AstTuple* _tuple) :
            type_class(_type_class), tuple(_tuple) {
            this->type = kh::AstExpressionType::INITIATE;
        }
        virtual ~AstInitiate() {
            delete this->type_class;
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

    class AstInlineDeclare : public kh::AstExpression {
        kh::AstExpression* type_class;
        kh::String variable_name;
        kh::AstTuple* tuple;

        AstInlineDeclare(kh::AstExpression* _type_class, const kh::String& _variable_name, kh::AstTuple* _tuple) :
            type_class(_type_class), variable_name(_variable_name), tuple(_tuple) {
            this->type = kh::AstExpressionType::INLINE_DECLARE;
        }
        virtual ~AstInlineDeclare() {
            delete this->type_class;
            delete this->tuple;
        }
    };

    class AstDeclare : public kh::AstExpression {
        kh::AstExpression* type_class;
        kh::String variable_name;
        kh::AstExpression* expression;

        AstDeclare(kh::AstExpression* _type_class, const kh::String& _variable_name, kh::AstExpression* _expression) :
            type_class(_type_class), variable_name(_variable_name), expression(_expression) {
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

    class AstSwitch : public kh::AstExpression {
    public:
        kh::AstExpression* expression;
        std::vector<kh::AstCase*> cases;
        std::vector<kh::AstExpression*> default_body;

        AstSwitch(kh::AstExpression* _expression, std::vector<kh::AstCase*>& _cases, std::vector<kh::AstExpression*> _default_body) :
            expression(_expression), cases(_cases), default_body(_default_body) {
            this->type = kh::AstExpressionType::SWITCH;
        }
        virtual ~AstSwitch() {
            delete this->expression;
            for (kh::AstCase* _case : this->cases)
                delete _case;
            for (kh::AstExpression* expression : this->default_body)
                delete expression;
        }
    };
}
