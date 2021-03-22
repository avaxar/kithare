/*
* This file is a part of the Kithare programming language source code.
* The source code for Kithare programming language is distributed under the MIT license.
* Copyright (C) 2021 Avaxar (AvaxarXapaxa)
*
* src/parser/ast.cpp
* Defines include/parser/ast.hpp.
*/

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
