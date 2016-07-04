#pragma once
#include "Ast.hpp"
#include "Function.hpp"
namespace slim
{
    namespace expr
    {
        //Some misc nodes
        class Literal : public ExpressionNode
        {
        public:
            Literal(ObjectPtr value) : value(value) {}
            virtual std::string to_string()const override;
            virtual ObjectPtr eval(Scope &scope)const override;
            ObjectPtr value;
        };
        class Variable : public ExpressionNode
        {
        public:
            Variable(const std::string &name) : name(name) {}
            virtual std::string to_string()const override { return name; }
            virtual ObjectPtr eval(Scope &scope)const override;
            std::string name;
        };
        class FuncCall : public ExpressionNode
        {
        public:
            typedef std::vector<std::unique_ptr<ExpressionNode>> Args;
            FuncCall(Args &&args) : args(std::move(args)) {}
            virtual std::string to_string()const override;
            FunctionArgs eval_args(Scope &scope)const;

            Args args;
        };
        class GlobalFuncCall : public FuncCall
        {
        public:
            GlobalFuncCall(const Function &function, Args &&args) : FuncCall(std::move(args)), function(function) {}
            virtual std::string to_string()const override;
            virtual ObjectPtr eval(Scope &scope)const override;

            const Function &function;
        };
        class MemberFuncCall : public FuncCall
        {
        public:
            MemberFuncCall(ExpressionNodePtr &&lhs, const std::string &name, Args &&args)
                : FuncCall(std::move(args)), lhs(std::move(lhs)), name(name)
            {}
            virtual std::string to_string()const override;
            virtual ObjectPtr eval(Scope &scope)const override;

            ExpressionNodePtr lhs;
            std::string name;
        };
        class SafeNavMemberFuncCall : public FuncCall
        {
        public:
            SafeNavMemberFuncCall(ExpressionNodePtr &&lhs, const std::string &name, Args &&args)
                : FuncCall(std::move(args)), lhs(std::move(lhs)), name(name)
            {}
            virtual std::string to_string()const override;
            virtual ObjectPtr eval(Scope &scope)const override;

            ExpressionNodePtr lhs;
            std::string name;
        };
        /**[] operator */
        class ElementRefOp : public FuncCall
        {
        public:
            ElementRefOp(ExpressionNodePtr &&lhs, Args &&args)
                : FuncCall(std::move(args)), lhs(std::move(lhs))
            {}
            virtual std::string to_string()const override;
            virtual ObjectPtr eval(Scope &scope)const override;

            ExpressionNodePtr lhs;
        };
        class ArrayLiteral : public FuncCall
        {
        public:
            ArrayLiteral(Args &&args) : FuncCall(std::move(args)) {}
            virtual std::string to_string()const override;
            virtual ObjectPtr eval(Scope &scope)const override;
        };
        class HashLiteral : public FuncCall
        {
        public:
            HashLiteral(Args &&args) : FuncCall(std::move(args)) {}
            virtual std::string to_string()const override;
            virtual ObjectPtr eval(Scope &scope)const override;
        };

        class Block : public ExpressionNode
        {
        public:
            Block(std::vector<std::string> &&param_names, std::unique_ptr<ExpressionNode> &&code)
                : param_names(std::move(param_names)), code(std::move(code))
            {}
            virtual std::string to_string()const override;
            virtual ObjectPtr eval(Scope &scope)const override;
        private:
            std::vector<std::string> param_names;
            std::unique_ptr<ExpressionNode> code;
        };
    }
}
