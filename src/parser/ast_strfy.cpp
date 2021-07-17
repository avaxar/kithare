/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/ast.hpp>
#include <kithare/utf8.hpp>


using namespace kh;

#define REPR_ALL_IN(var)   \
    for (auto& _var : var) \
    str += U"\n\t" + ind + strfy(_var, indent + 1)

#define BODY_HEADER()                   \
    std::u32string ind;                 \
    ind.reserve(indent);                \
    for (size_t i = 0; i < indent; i++) \
        ind += '\t';                    \
                                        \
    std::u32string str;

std::u32string kh::strfy(const AstModule& module_ast, size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++) {
        ind += '\t';
    }

    std::u32string str = U"ast:";

    REPR_ALL_IN(module_ast.imports);
    REPR_ALL_IN(module_ast.functions);
    REPR_ALL_IN(module_ast.user_types);
    REPR_ALL_IN(module_ast.enums);
    REPR_ALL_IN(module_ast.variables);
    return str;
}

std::u32string kh::strfy(const AstImport& import_ast, size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++) {
        ind += '\t';
    }

    std::u32string str = import_ast.is_include ? U"include:" : U"import:";
    str += U"\n\t" + ind + U"type: " + (import_ast.is_relative ? U"relative" : U"absolute");
    str += U"\n\t" + ind + U"access: " + (import_ast.is_public ? U"public" : U"private");

    str += U"\n\t" + ind + U"path: ";
    for (const std::string& dir : import_ast.path) {
        str += decodeUtf8(dir) + (&dir == &import_ast.path.back() ? U"" : U".");
    }

    if (!import_ast.is_include) {
        str += U"\n\t" + ind + U"identifier: " + decodeUtf8(import_ast.identifier);
    }

    return str;
}

std::u32string kh::strfy(const AstUserType& type_ast, size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++) {
        ind += '\t';
    }

    std::u32string str = (type_ast.is_class ? U"class:\n\t" : U"struct:\n\t") + ind + U"name: ";
    for (const std::string& identifier : type_ast.identifiers) {
        str += decodeUtf8(identifier) + (&identifier == &type_ast.identifiers.back() ? U"" : U".");
    }

    str += U"\n\t" + ind + U"access: " + (type_ast.is_public ? U"public" : U"private");

    if (type_ast.base) {
        str += U"\n\t" + ind + U"base " + (type_ast.is_class ? U"class:" : U"struct:");
        str += U"\n\t\t" + ind + kh::strfy(*type_ast.base, indent + 3);
    }

    if (!type_ast.generic_args.empty()) {
        str += U"\n\t" + ind + U"generic argument(s): ";
        for (const std::string& generic_ : type_ast.generic_args) {
            str += decodeUtf8(generic_) + (&generic_ == &type_ast.generic_args.back() ? U"" : U", ");
        }
    }

    if (!type_ast.members.empty()) {
        str += U"\n\t" + ind + U"member(s):";
        for (auto& member : type_ast.members) {
            str += U"\n\t\t" + ind + strfy(member, indent + 2);
        }
    }

    if (!type_ast.methods.empty()) {
        str += U"\n\t" + ind + U"method(s):";
        for (auto& method : type_ast.methods) {
            str += U"\n\t\t" + ind + strfy(method, indent + 2);
        }
    }

    return str;
}

std::u32string kh::strfy(const AstEnumType& enum_ast, size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++) {
        ind += '\t';
    }

    std::u32string str = U"enum:\n\t" + ind + U"name: ";
    for (const std::string& identifier : enum_ast.identifiers) {
        str += decodeUtf8(identifier) + (&identifier == &enum_ast.identifiers.back() ? U"" : U".");
    }

    str += U"\n\t" + ind + U"access: " + (enum_ast.is_public ? U"public" : U"private");

    str += U"\n\t" + ind + U"member(s):";
    for (size_t member = 0; member < enum_ast.members.size(); member++) {
        str += U"\n\t\t" + ind + decodeUtf8(enum_ast.members[member]) + U": " +
               strfy(enum_ast.values[member]);
    }

    return str;
}

std::u32string kh::strfy(const AstBody& body_ast, size_t indent) {
    return body_ast.strfy(indent);
}

std::u32string kh::AstBody::strfy(size_t indent) const {
    return U"[unknown body]";
}

std::u32string kh::AstExpression::strfy(size_t indent) const {
    return U"[unknown expression]";
}

std::u32string kh::AstIdentifiers::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"identifier(s): ";

    for (const std::string& identifier : this->identifiers) {
        str += decodeUtf8(identifier) + (&identifier == &this->identifiers.back() ? U"" : U".");
    }

    bool is_function = this->identifiers.size() == 1 && this->identifiers[0] == "func";

    if (!this->generics.empty()) {
        str += U"!(";
        for (size_t i = 0; i < this->generics.size(); i++) {
            for (size_t refs = 0; refs < this->generics_refs[i]; refs++) {
                str += U"ref ";
            }

            str += kh::strfy(this->generics[i], indent);

            for (size_t d = 0; d < this->generics_array[i].size(); d++) {
                str += U"[" + kh::strfy(this->generics_array[i][d]) + U"]";
            }

            if (is_function && i == 0) {
                str += U"(";
            }
            else if (i != this->generics.size() - 1) {
                str += U", ";
            }
        }
        str += is_function ? U"))" : U")";
    }

    return str;
}

std::u32string kh::AstUnaryOperation::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"unary expression:";

    str += U"\n\t" + ind + U"operator: " + kh::strfy(this->operation);

    if (this->rvalue) {
        str += U"\n\t" + ind + U"rvalue:\n\t\t" + ind + kh::strfy(*this->rvalue, indent + 2);
    }

    return str;
}

std::u32string kh::AstRevUnaryOperation::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"reverse unary expression:";

    str += U"\n\t" + ind + U"operator: " + kh::strfy(this->operation);

    if (this->rvalue) {
        str += U"\n\t" + ind + U"rvalue:\n\t\t" + ind + kh::strfy(*this->rvalue, indent + 2);
    }

    return str;
}

std::u32string kh::AstBinaryOperation::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"binary expression:";

    str += U"\n\t" + ind + U"operator: " + kh::strfy(this->operation);

    if (this->lvalue) {
        str += U"\n\t" + ind + U"lvalue:\n\t\t" + ind + kh::strfy(*this->lvalue, indent + 2);
    }

    if (this->rvalue) {
        str += U"\n\t" + ind + U"rvalue:\n\t\t" + ind + kh::strfy(*this->rvalue, indent + 2);
    }

    return str;
}

std::u32string kh::AstTernaryOperation::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"ternary expression:";

    if (this->condition) {
        str += U"\n\t" + ind + U"condition:\n\t\t" + ind + kh::strfy(*this->condition, indent + 2);
    }

    if (this->value) {
        str += U"\n\t" + ind + U"value:\n\t\t" + ind + kh::strfy(*this->value, indent + 2);
    }

    if (this->otherwise) {
        str += U"\n\t" + ind + U"otherwise:\n\t\t" + ind + kh::strfy(*this->otherwise, indent + 2);
    }

    return str;
}

std::u32string kh::AstComparisonExpression::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"comparison expression:\n\t" + ind + U"operation(s): ";

    for (Operator operation : this->operations) {
        str += kh::strfy(operation) + (&operation == &this->operations.back() ? U"" : U",");
    }

    str += U"\n\t" + ind + U"value(s):";
    for (auto& value : this->values) {
        if (value) {
            str += U"\n\t\t" + ind + kh::strfy(*value);
        }
    }

    return str;
}

std::u32string kh::AstSubscriptExpression::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"subscript:";

    if (this->expression) {
        str += U"\n\t" + ind + U"expression:\n\t\t" + ind + kh::strfy(*this->expression, indent + 2);
    }

    if (!this->arguments.empty()) {
        str += U"\n\t" + ind + U"argument(s):";
        for (auto& argument : this->arguments) {
            if (argument) {
                str += U"\n\t\t" + ind + kh::strfy(*argument, indent + 2);
            }
        }
    }

    return str;
}

std::u32string kh::AstCallExpression::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"call:";

    if (this->expression) {
        str += U"\n\t" + ind + U"expression:\n\t\t" + ind + kh::strfy(*this->expression, indent + 2);
    }

    if (!this->arguments.empty()) {
        str += U"\n\t" + ind + U"argument(s):";
        for (auto& argument : this->arguments) {
            if (argument) {
                str += U"\n\t\t" + ind + kh::strfy(*argument, indent + 2);
            }
        }
    }

    return str;
}

std::u32string kh::AstDeclaration::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"declare:";

    str += U"\n\t" + ind + U"access: " + (this->is_static ? U"static " : U"") +
           (this->is_public ? U"public" : U"private");

    str += U"\n\t" + ind + U"type: ";
    for (size_t refs = 0; refs < this->refs; refs++) {
        str += U"ref ";
    }
    str += kh::strfy(this->var_type, indent + 1);

    for (uint64_t dimension : this->var_array) {
        str += U'[' + strfy(dimension) + U']';
    }

    str += U"\n\t" + ind + U"name: " + decodeUtf8(this->var_name);

    if (this->expression)
        str += U"\n\t" + ind + U"initializer expression:\n\t\t" + ind +
               kh::strfy(*this->expression, indent + 2);

    return str;
}

std::u32string kh::AstFunction::strfy(size_t indent) const {
    BODY_HEADER();
    str = this->is_conditional ? U"conditional function:" : U"function:";

    str += U"\n\t" + ind + U"access: " + (this->is_static ? U"static " : U"") +
           (this->is_public ? U"public" : U"private");

    if (this->identifiers.empty()) {
        str += U"\n\t" + ind + U"name: (lambda)";
    }
    else {
        str += U"\n\t" + ind + U"name: ";
        for (const std::string& identifier : this->identifiers) {
            str += decodeUtf8(identifier) + (&identifier == &this->identifiers.back() ? U"" : U".");
        }

        if (!this->generic_args.empty()) {
            str += U"\n\t" + ind + U"generic argument(s): ";
            for (const std::string& generic_ : this->generic_args) {
                str += decodeUtf8(generic_) + (&generic_ == &this->generic_args.back() ? U"" : U", ");
            }
        }

        if (!this->id_array.empty()) {
            str += U"\n\t" + ind + U"array type dimension: ";
            for (uint64_t size : this->id_array) {
                str += U'[' + strfy(size) + U']';
            }
        }
    }

    str += U"\n\t" + ind + U"return type: ";

    for (size_t refs = 0; refs < this->return_refs; refs++) {
        str += U"ref ";
    }
    str += kh::strfy(this->return_type, indent + 1);

    for (uint64_t dimension : this->return_array) {
        str += U'[' + strfy(dimension) + U']';
    }

    str += U"\n\t" + ind + U"argument(s):";
    if (this->arguments.empty()) {
        str += U" [none]";
    }
    for (auto& arg : this->arguments) {
        str += U"\n\t\t" + ind + kh::strfy(arg, indent + 2);
    }

    str += U"\n\t" + ind + U"body:";
    for (auto& part : this->body) {
        if (part) {
            str += U"\n\t\t" + ind + kh::strfy(*part, indent + 2);
        }
    }

    return str;
}

std::u32string kh::AstScoping::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"scoping (";

    if (!this->identifiers.empty()) {
        for (const std::string& identifier : this->identifiers) {
            str += (&this->identifiers.back() == &identifier ? U"" : U".") + decodeUtf8(identifier);
        }
    }

    str += U"):"; /* sad face */

    if (this->expression) {
        str += U"\n\t" + ind + kh::strfy(*this->expression, indent + 1);
    }

    return str;
}

std::u32string kh::AstValue::strfy(size_t indent) const {
    BODY_HEADER();

    switch (this->value_type) {
        case AstValue::ValueType::CHARACTER:
            str = U"character: " + kh::strfy(this->character);
            break;

        case AstValue::ValueType::UINTEGER:
            str = U"unsigned integer: " + kh::strfy(this->uinteger);
            break;

        case AstValue::ValueType::INTEGER:
            str = U"integer: " + kh::strfy(this->integer);
            break;

        case AstValue::ValueType::FLOATING:
            str = U"floating: " + kh::strfy(this->floating);
            break;

        case AstValue::ValueType::IMAGINARY:
            str = U"imaginary: " + kh::strfy(this->imaginary) + U"i";
            break;

        case AstValue::ValueType::BUFFER:
            str = U"buffer: " + quote(this->buffer);
            break;

        case AstValue::ValueType::STRING:
            str = U"string: " + quote(this->string);
            break;

        default:
            str = U"[unknown constant]";
    }

    return str;
}

std::u32string kh::AstTuple::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"tuple:";

    if (this->elements.empty()) {
        str += U" [no elements]";
    }
    else {
        for (auto& element : this->elements) {
            if (element) {
                str += U"\n\t" + ind + kh::strfy(*element, indent + 1);
            }
        }
    }

    return str;
}

std::u32string kh::AstList::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"list:";

    if (this->elements.empty()) {
        str += U" [no elements]";
    }
    else {
        for (auto& element : this->elements) {
            if (element) {
                str += U"\n\t" + ind + kh::strfy(*element, indent + 1);
            }
        }
    }

    return str;
}

std::u32string kh::AstDict::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"dict:";

    if (this->keys.empty()) {
        str += U" [no pairs]";
    }
    else {
        for (size_t i = 0; i < this->keys.size(); i++) {
            str += U"\n\t" + ind + U"pair:";
            if (this->keys[i]) {
                str += U"\n\t\t" + ind + kh::strfy(*this->keys[i], indent + 2);
            }
            if (this->items[i]) {
                str += U"\n\t\t" + ind + kh::strfy(*this->items[i], indent + 2);
            }
        }
    }

    return str;
}

std::u32string kh::AstIf::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"if:";

    for (size_t clause = 0; clause < this->conditions.size(); clause++) {
        str += U"\n\t" + ind + U"if clause:";

        if (this->conditions[clause])
            str += U"\n\t\t" + ind + U"condition:\n\t\t\t" + ind +
                   kh::strfy(*this->conditions[clause], indent + 3);

        if (!this->bodies[clause].empty()) {
            str += U"\n\t\t" + ind + U"body:";
            for (auto& part : this->bodies[clause])
                if (part) {
                    str += U"\n\t\t\t" + ind + kh::strfy(*part, indent + 3);
                }
        }
    }

    if (!this->else_body.empty()) {
        str += U"\n\t" + ind + U"else body:";
        for (auto& part : this->else_body)
            if (part) {
                str += U"\n\t\t" + ind + kh::strfy(*part, indent + 2);
            }
    }

    return str;
}

std::u32string kh::AstWhile::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"while:";

    if (this->condition) {
        str += U"\n\t" + ind + U"condition:\n\t\t" + ind + kh::strfy(*this->condition, indent + 2);
    }

    if (!this->body.empty()) {
        str += U"\n\t" + ind + U"body:";
        for (auto& part : this->body)
            if (part) {
                str += U"\n\t\t" + ind + kh::strfy(*part, indent + 2);
            }
    }

    return str;
}

std::u32string kh::AstDoWhile::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"do while:";

    if (this->condition) {
        str += U"\n\t" + ind + U"condition:\n\t\t" + ind + kh::strfy(*this->condition, indent + 2);
    }

    if (!this->body.empty()) {
        str += U"\n\t" + ind + U"body:";
        for (auto& part : this->body)
            if (part) {
                str += U"\n\t\t" + ind + kh::strfy(*part, indent + 2);
            }
    }

    return str;
}

std::u32string kh::AstFor::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"for:";

    if (this->initialize) {
        str += U"\n\t" + ind + U"initializer:\n\t\t" + ind + kh::strfy(*this->initialize, indent + 2);
    }

    if (this->condition) {
        str += U"\n\t" + ind + U"condition:\n\t\t" + ind + kh::strfy(*this->condition, indent + 2);
    }

    if (this->step) {
        str += U"\n\t" + ind + U"step:\n\t\t" + ind + kh::strfy(*this->step, indent + 2);
    }

    if (!this->body.empty()) {
        str += U"\n\t" + ind + U"body:";
        for (auto& part : this->body)
            if (part) {
                str += U"\n\t\t" + ind + kh::strfy(*part, indent + 2);
            }
    }

    return str;
}

std::u32string kh::AstForEach::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"foreach:";

    if (this->target) {
        str += U"\n\t" + ind + U"target:\n\t\t" + ind + kh::strfy(*this->target, indent + 2);
    }

    if (this->iterator) {
        str += U"\n\t" + ind + U"iterator:\n\t\t" + ind + kh::strfy(*this->iterator, indent + 2);
    }

    if (!this->body.empty()) {
        str += U"\n\t" + ind + U"body:";
        for (auto& part : this->body)
            if (part) {
                str += U"\n\t\t" + ind + kh::strfy(*part, indent + 2);
            }
    }

    return str;
}

std::u32string kh::AstStatement::strfy(size_t indent) const {
    BODY_HEADER();
    str = U"statement: ";

    switch (this->statement_type) {
        case AstStatement::Type::CONTINUE:
            str += U"continue";
            break;
        case AstStatement::Type::BREAK:
            str += U"break";
            break;
        case AstStatement::Type::RETURN:
            str += U"return";
            break;
        default:
            str += U"unknown";
            break;
    }

    if (this->statement_type == AstStatement::Type::RETURN) {
        if (this->expression) {
            str += U"\n\t" + ind + kh::strfy(*this->expression, indent + 1);
        }
    }
    else {
        str += U" " + strfy((uint64_t)this->loop_count);
    }
    return str;
}
