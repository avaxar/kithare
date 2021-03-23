/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/parser/ast.cpp
* Defines include/parser/ast.hpp.
*/

#include "utility/string.hpp"

#include "parser/ast.hpp"


kh::AstModule::~AstModule() {
    for (kh::AstImport* import_ : this->imports)
        if (import_) delete import_;

    for (kh::AstFunction* function : this->functions)
        if (function) delete function;

    for (kh::AstClass* class_ : this->classes)
        if (class_) delete class_;

    for (kh::AstStruct* struct_ : this->structs)
        if (struct_) delete struct_;

    for (kh::AstEnum* enum_ : this->enums)
        if (enum_) delete enum_;

    for (kh::AstDeclarationExpression* variable : this->variables)
        if (variable) delete variable;
}

kh::AstFunction::~AstFunction() {
    if (this->return_type)
        delete this->return_type;

    for (kh::AstDeclarationExpression* argument : this->arguments)
        if (argument) delete argument;

    for (kh::AstFunction* function : this->nested_functions)
        if (function) delete function;

    for (kh::AstBody* part : this->body)
        if (part) delete part;
}

kh::AstClass::~AstClass() {
    if (this->base)
        delete this->base;

    for (kh::AstDeclarationExpression* member : this->members)
        if (member) delete member;

    for (kh::AstFunction* method : this->methods)
        if (method) delete method;
}

kh::AstStruct::~AstStruct() {
    if (this->base)
        delete this->base;

    for (kh::AstDeclarationExpression* member : this->members)
        if (member) delete member;
}

kh::AstEnum::~AstEnum() {
    if (this->base)
        delete this->base;
}

std::u32string kh::repr(const kh::AstModule& module_ast, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str = U"MODULE:";

    for (kh::AstImport* import_ : module_ast.imports)
        if (import_)
            str += U"\n\t" + ind + kh::repr(*import_, indent + 1);

    for (kh::AstFunction* function : module_ast.functions)
        if (function)
            str += U"\n\t" + ind + kh::repr(*function, indent + 1);

    for (kh::AstClass* class_ : module_ast.classes)
        if (class_)
            str += U"\n\t" + ind + kh::repr(*class_, indent + 1);

    for (kh::AstStruct* struct_ : module_ast.structs)
        if (struct_)
            str += U"\n\t" + ind + kh::repr(*struct_, indent + 1);

    for (kh::AstEnum* enum_ : module_ast.enums)
        if (enum_)
            str += U"\n\t" + ind + kh::repr(*enum_, indent + 1);

    for (kh::AstDeclarationExpression* variable : module_ast.variables)
        if (variable)
            str += U"\n\t" + ind + kh::repr(*variable, indent + 1);

    return str;
}

std::u32string kh::repr(const kh::AstImport& import_ast, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str = U"IMPORT:";

    str += U"\n\t" + ind + U"PATH:";
    for (const std::u32string& dir : import_ast.path)
        str += U"\n\t\t" + ind + dir;

    str += U"\n\t" + ind + U"IDENTIFIER: " + import_ast.identifier;

    return str;
}

std::u32string kh::repr(const kh::AstFunction& function_ast, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str = U"FUNCTION:";

    str += U"\n\t" + ind + U"STATIC: " + (function_ast.is_static ? U"STATIC" : U"NON-STATIC");
    str += U"\n\t" + ind + U"ACCESS: " + (function_ast.is_public ? U"PUBLIC" : U"PRIVATE");

    str += U"\n\t" + ind + U"IDENTIFIERS:";
    for (const std::u32string& identifier : function_ast.identifiers)
        str += U"\n\t\t" + ind + identifier;

    str += U"\n\t" + ind + U"TEMPLATES:";
    for (const std::u32string& template_ : function_ast.templates)
        str += U"\n\t\t" + ind + template_;

    str += U"\n\t" + ind + U"RETURN TYPE REF DEPTH: " + kh::repr((uint64_t)function_ast.return_ref_depth);
    str += U"\n\t" + ind + U"RETURN TYPE:\n\t\t" + ind + kh::repr(*function_ast.return_type, indent + 2);

    str += U"\n\t" + ind + U"ARGUMENTS:";
    for (kh::AstDeclarationExpression* arg : function_ast.arguments)
        if (arg)
            str += U"\n\t\t" + ind + kh::repr(*arg, indent + 2);

    str += U"\n\t" + ind + U"NESTED FUNCTIONS:";
    for (kh::AstFunction* function : function_ast.nested_functions)
        if (function)
            str += U"\n\t\t" + ind + kh::repr(*function, indent + 2);

    str += U"\n\t" + ind + U"BODY:";
    for (kh::AstBody* part : function_ast.body)
        if (part)
            str += U"\n\t\t" + ind + kh::repr(*part, indent + 2);

    return str;
}

std::u32string kh::repr(const kh::AstClass& class_ast, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str = U"CLASS:";
    str += U"\n\t" + ind + U"NAME: " + class_ast.name;

    if (class_ast.base)
        str += U"\n\t" + ind + U"BASE:\n\t\t" + ind + kh::repr(*class_ast.base);

    if (!class_ast.templates.empty()) {
        str += U"\n\t" + ind + U"TEMPLATES:";
        for (const std::u32string& template_ : class_ast.templates)
            str += U"\n\t\t" + ind + template_;
    }

    str += U"\n\t" + ind + U"MEMBERS:";
    for (kh::AstDeclarationExpression* member : class_ast.members)
        str += U"\n\t" + ind + kh::repr(*member);

    str += U"\n\t" + ind + U"METHODS:";
    for (kh::AstFunction* method : class_ast.methods)
        str += U"\n\t" + ind + kh::repr(*method);

    return str;
}

std::u32string kh::repr(const kh::AstStruct& struct_ast, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str = U"STRUCT:";
    str += U"\n\t" + ind + U"NAME: " + struct_ast.name;

    if (struct_ast.base)
        str += U"\n\t" + ind + U"BASE:\n\t\t" + ind + kh::repr(*struct_ast.base);

    str += U"\n\t" + ind + U"MEMBERS:";
    for (kh::AstDeclarationExpression* member : struct_ast.members)
        str += U"\n\t" + ind + kh::repr(*member);

    return str;
}

std::u32string kh::repr(const kh::AstEnum& enum_ast, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str = U"ENUM:";
    str += U"\n\t" + ind + U"NAME: " + enum_ast.name;

    if (enum_ast.base)
        str += U"\n\t" + ind + U"BASE:\n\t\t" + ind + kh::repr(*enum_ast.base);

    str += U"\n\t" + ind + U"MEMBERS:";
    for (const std::u32string& member : enum_ast.members)
        str += U"\n\t" + ind + member;

    return str;
}

std::u32string kh::repr(const kh::AstBody& ast, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str;

    switch (ast.type) {
    case kh::AstBody::Type::EXPRESSION: {
        str += kh::repr(*(kh::AstExpression*)&ast, indent);
        break;
    }

    case kh::AstBody::Type::IF: {
        const kh::AstIf& ast_if = *(kh::AstIf*)&ast;
        str += U"IF:";

        for (size_t clause = 0; clause < ast_if.conditions.size(); clause++) {
            str += U"\n\t" + ind + U"IF CLAUSE:";

            if (ast_if.conditions[clause])
                str += U"\n\t\t" + ind + U"CONDITION:\n\t\t\t" + kh::repr(*ast_if.conditions[clause], indent + 3);

            if (!ast_if.bodies[clause].empty()) {
                str += U"\n\t\t" + ind + U"BODY:";
                for (kh::AstBody* part : ast_if.bodies[clause])
                    if (part)
                        str += U"\n\t\t" + ind + kh::repr(*part, indent + 3);
            }
        }

        if (!ast_if.else_body.empty()) {
            str += U"\n\t" + ind + U"ELSE CLAUSE:\n";
            for (kh::AstBody* part : ast_if.else_body)
                if (part)
                    str += U"\n\t\t" + ind + kh::repr(*part, indent + 2);
        }

        break;
    }

    case kh::AstBody::Type::WHILE: {
        const kh::AstWhile& ast_while = *(kh::AstWhile*)&ast;
        str += U"WHILE:";

        if (ast_while.condition) {
            str += U"\n\t" + ind + U"CONDITION:\n\t\t" + kh::repr(*ast_while.condition, indent + 2);
        }

        if (!ast_while.body.empty()) {
            str += U"\n\t" + ind + U"BODY:";
            for (kh::AstBody* part : ast_while.body)
                if (part)
                    str += U"\n\t\t" + ind + kh::repr(*part, indent + 2);
        }

        break;
    }

    case kh::AstBody::Type::DO_WHILE: {
        const kh::AstDoWhile& ast_do_while = *(kh::AstDoWhile*)&ast;
        str += U"DO WHILE:";

        if (ast_do_while.condition)
            str += U"\n\t" + ind + U"CONDITION:\n\t\t" + kh::repr(*ast_do_while.condition, indent + 2);

        if (!ast_do_while.body.empty()) {
            str += U"\n\t" + ind + U"BODY:";
            for (kh::AstBody* part : ast_do_while.body)
                if (part)
                    str += U"\n\t\t" + ind + kh::repr(*part, indent + 2);
        }

        break;
    }

    case kh::AstBody::Type::FOR: {
        const kh::AstFor& ast_for = *(kh::AstFor*)&ast;
        str += U"FOR:";

        if (!ast_for.targets.empty()) {
            str += U"\n\t" + ind + U"TARGETS:";
            for (kh::AstExpression* target : ast_for.targets)
                if (target)
                    str += U"\n\t\t" + ind + kh::repr(*target, indent + 2);
        }

        if (ast_for.iterator)
            str += U"\n\t" + ind + kh::repr(*ast_for.iterator, indent + 1);

        if (!ast_for.body.empty()) {
            str += U"\n\t" + ind + U"BODY:";
            for (kh::AstBody* part : ast_for.body)
                if (part)
                    str += U"\n\t\t" + ind + kh::repr(*part, indent + 2);
        }

        break;
    }

    case kh::AstBody::Type::STATEMENT: {
        const kh::AstStatement& ast_statement = *(kh::AstStatement*)&ast;
        str += U"STATEMENT:";

        str += U"\n\t" + ind + U"TYPE: ";
        switch (ast_statement.statement_type) {
        case kh::AstStatement::Type::CONTINUE:
            str += U"CONTINUE"; break;
        case kh::AstStatement::Type::BREAK:
            str += U"BREAK"; break;
        case kh::AstStatement::Type::RETURN:
            str += U"RETURN"; break;
        default:
            str += U"UNKNOWN"; break;
        }

        if (ast_statement.expression)
            str += U"\n\t" + ind + U"EXPRESSION:\n\t\t" + ind + kh::repr(*ast_statement.expression, indent + 2);

        break;
    }

    case kh::AstBody::Type::INSTRUCTION: {
        const kh::AstInstruction& ast_inst = *(kh::AstInstruction*)&ast;
        str += U"INSTRUCTION:";

        str += U"\n\t" + ind + U"OP NAME: " + ast_inst.op_name;

        for (kh::AstExpression* op_arg : ast_inst.op_arguments)
            if (op_arg)
                str += str += U"\n\t" + ind + U"OP ARGUMENT:\n\t\t" + ind + kh::repr(*op_arg, indent + 2);

        break;
    }

    default:
        str += U"[UNKNOWN BODY]";
    }

    return str;
}

std::u32string kh::repr(const kh::AstExpression& expr, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str;

    switch (expr.expression_type) {
    case kh::AstExpression::ExType::IDENTIFIER: {
        const kh::AstIdentifierExpression& expr_id = *(kh::AstIdentifierExpression*)&expr;
        str += U"IDENTIFIER EXPR:";

        if (!expr_id.identifiers.empty()) {
            str += U"\n\t" + ind + U"IDENTIFIERS:";
            for (const std::u32string& identifier : expr_id.identifiers)
                str += U"\n\t\t" + ind + identifier;
        }

        if (!expr_id.templates.empty()) {
            str += U"\n\t" + ind + U"TEMPLATES:";
            for (kh::AstIdentifierExpression* template_ : expr_id.templates)
                if (template_)
                    str += U"\n\t\t" + ind + kh::repr(*template_, indent + 2);
        }

        break;
    }

    case kh::AstExpression::ExType::UNARY: {
        const kh::AstUnaryExpression& expr_unary = *(kh::AstUnaryExpression*)&expr;
        str += U"UNARY EXPR:";

        str += U"\n\t" + ind + U"OPERATOR: " + kh::repr(expr_unary.operation);

        if (expr_unary.rvalue)
            str += U"\n\t" + ind + U"RVALUE:\n\t\t" + ind + kh::repr(*expr_unary.rvalue, indent + 2);

        break;
    }

    case kh::AstExpression::ExType::BINARY: {
        const kh::AstBinaryExpression& expr_binary = *(kh::AstBinaryExpression*)&expr;
        str += U"BINARY EXPR:";

        str += U"\n\t" + ind + U"OPERATOR: " + kh::repr(expr_binary.operation);

        if (expr_binary.lvalue)
            str += U"\n\t" + ind + U"LVALUE:\n\t\t" + ind + kh::repr(*expr_binary.lvalue, indent + 2);
        if (expr_binary.rvalue)
            str += U"\n\t" + ind + U"RVALUE:\n\t\t" + ind + kh::repr(*expr_binary.rvalue, indent + 2);

        break;
    }

    case kh::AstExpression::ExType::TERNARY: {
        const kh::AstTernaryExpression& expr_ternary = *(kh::AstTernaryExpression*)&expr;
        str += U"TERNARY EXPR:";

        if (expr_ternary.condition)
            str += U"\n\t" + ind + U"CONDITION:\n\t\t" + ind + kh::repr(*expr_ternary.condition, indent + 2);
        if (expr_ternary.value)
            str += U"\n\t" + ind + U"VALUE:\n\t\t" + ind + kh::repr(*expr_ternary.value, indent + 2);
        if (expr_ternary.otherwise)
            str += U"\n\t" + ind + U"OTHERWISE:\n\t\t" + ind + kh::repr(*expr_ternary.otherwise, indent + 2);

        break;
    }

    case kh::AstExpression::ExType::SUBSCRIPT: {
        const kh::AstSubscriptExpression& expr_subscript = *(kh::AstSubscriptExpression*)&expr;
        str += U"SUBSCRIPT:";

        if (expr_subscript.expression)
            str += U"\n\t" + ind + U"EXPRESSION:\n\t\t" + ind + kh::repr(*expr_subscript.expression, indent + 2);

        for (kh::AstExpression* argument : expr_subscript.arguments)
            if (argument)
                str += U"\n\t" + ind + U"ARGUMENT:\n\t\t" + ind + kh::repr(*argument, indent + 2);

        break;
    }

    case kh::AstExpression::ExType::CALL: {
        const kh::AstCallExpression & expr_call = *(kh::AstCallExpression*)&expr;
        str += U"CALL:";

        if (expr_call.expression)
            str += U"\n\t" + ind + U"EXPRESSION:\n\t\t" + ind + kh::repr(*expr_call.expression, indent + 2);

        if (!expr_call.arguments.empty()) {
            str += U"\n\t" + ind + U"ARGUMENTS:";
            for (kh::AstExpression* argument : expr_call.arguments)
                if (argument)
                    str += U"\n\t\t"+ ind + kh::repr(*argument, indent + 2);
        }

        break;
    }

    case kh::AstExpression::ExType::DECLARE: {
        const kh::AstDeclarationExpression& expr_declare = *(kh::AstDeclarationExpression*)&expr;
        str += U"DECLARE:";

        if (expr_declare.var_type)
            str += U"\n\t" + ind + U"TYPE:\n\t\t" + ind + kh::repr(*expr_declare.var_type, indent + 2);

        str += U"\n\t" + ind + U"NAME: " + ind + expr_declare.var_name;

        if (expr_declare.expression)
            str += U"\n\t" + ind + U"EXPRESSION:\n\t\t" + ind + kh::repr(*expr_declare.expression, indent + 2);

        str += U"\n\t" + ind + U"REF DEPTH: " + kh::repr((uint64_t)expr_declare.ref_depth);
        str += U"\n\t" + ind + U"STATIC: " + (expr_declare.is_static ? U"STATIC" : U"NON-STATIC");
        str += U"\n\t" + ind + U"ACCESS: " + (expr_declare.is_public ? U"PUBLIC" : U"PRIVATE");

        break;
    }

    case kh::AstExpression::ExType::SCOPE: {
        const kh::AstScopeExpression& expr_scope = *(kh::AstScopeExpression*)&expr;
        str += U"SCOPE:";

        if (expr_scope.expression)
            str += U"\n\t" + ind + U"EXPRESSION:\n\t\t" + ind + kh::repr(*expr_scope.expression, indent + 2);

        if (!expr_scope.identifiers.empty()) {
            str += U"\n\t" + ind + U"IDENTIFIERS:";
            for (const std::u32string& identifier : expr_scope.identifiers)
                str += U"\n\t\t" + ind + identifier;
        }

        break;
    }

    case kh::AstExpression::ExType::CONSTANT: {
        const kh::AstConstValue& expr_const = *(kh::AstConstValue*)&expr;
        str += U"CONSTANT:";

        switch (expr_const.value_type) {
        case kh::AstConstValue::ValueType::CHARACTER:
            str += U"\n\t" + ind + U"TYPE: CHARACTER";
            str += U"\n\t" + ind + U"VALUE: " + kh::quote(std::u32string() + expr_const.character);
            break;

        case kh::AstConstValue::ValueType::UINTEGER:
            str += U"\n\t" + ind + U"TYPE: UINTEGER";
            str += U"\n\t" + ind + U"VALUE: " + kh::repr(expr_const.uinteger);
            break;

        case kh::AstConstValue::ValueType::INTEGER:
            str += U"\n\t" + ind + U"TYPE: INTEGER";
            str += U"\n\t" + ind + U"VALUE: " + kh::repr(expr_const.integer);
            break;

        case kh::AstConstValue::ValueType::FLOATING:
            str += U"\n\t" + ind + U"TYPE: FLOATING";
            str += U"\n\t" + ind + U"VALUE: " + kh::repr(expr_const.floating);
            break;

        case kh::AstConstValue::ValueType::COMPLEX:
            str += U"\n\t" + ind + U"TYPE: COMPLEX";
            str += U"\n\t" + ind + U"VALUE: " + kh::repr(expr_const.complex);
            break;

        case kh::AstConstValue::ValueType::BUFFER:
            str += U"\n\t" + ind + U"TYPE: BUFFER";
            str += U"\n\t" + ind + U"VALUE: " + kh::quote(expr_const.buffer);
            break;

        case kh::AstConstValue::ValueType::STRING:
            str += U"\n\t" + ind + U"TYPE: STRING";
            str += U"\n\t" + ind + U"VALUE: " + kh::quote(expr_const.string);
            break;

        default:
            str += U"\n\t" + ind + U"TYPE: UNKNOWN";
        }

        break;
    }

    case kh::AstExpression::ExType::TUPLE: {
        const kh::AstTupleExpression& expr_const = *(kh::AstTupleExpression*)&expr;
        str += U"TUPLE:";

        for (kh::AstExpression* element : expr_const.elements)
            if (element)
                str += U"\n\t" + ind + kh::repr(*element, indent + 1);

        break;
    }

    default:
        str += U"[UNKNOWN EXPRESSION]";
    }

    return str;
}
