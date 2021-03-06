#include "expression/AstOp.hpp"
#include "expression/Scope.hpp"
#include "types/Object.hpp"
#include "types/Array.hpp"
#include "types/Hash.hpp"
#include "types/HtmlSafeString.hpp"
#include "types/Proc.hpp"
#include "types/Regexp.hpp"
#include "types/Range.hpp"
#include "types/String.hpp"
#include "template/Template.hpp"
#include <sstream>
namespace slim
{
    namespace expr
    {
        std::string Literal::to_string() const
        {
            return value->inspect();
        }
        ObjectPtr Literal::eval(Scope & scope) const
        {
            return value;
        }

        ObjectPtr Variable::eval(Scope & scope) const
        {
            return scope.get(name);
        }

        std::string Assignment::to_string()const
        {
            return name->to_string() + " = " + expr->to_string();
        }
        ObjectPtr Assignment::eval(Scope &scope)const
        {
            auto val = expr->eval(scope);
            scope.set(name, val);
            return val;
        }

        ObjectPtr Attribute::eval(Scope & scope) const
        {
            return scope.self()->get_attr(name);
        }

        ObjectPtr GlobalConstant::eval(Scope &scope)const
        {
            return scope.self()->get_constant(name);
        }


        std::string ConstantNav::to_string()const
        {
            return lhs->to_string() + "::" + name->str();
        }
        ObjectPtr ConstantNav::eval(Scope &scope)const
        {
            return lhs->eval(scope)->get_constant(name);
        }

        std::string FuncCall::to_string() const
        {
            std::stringstream ss;
            if (!args.empty()) ss << args[0]->to_string();
            for (size_t i = 1; i < args.size(); ++i)
                ss << ", " << args[i]->to_string();
            return ss.str();
        }
        FunctionArgs FuncCall::eval_args(Scope & scope) const
        {
            FunctionArgs ret;
            for (auto &arg : args) ret.push_back(arg->eval(scope));
            return ret;
        }

        const Method* MethodCall::method(Object *obj)const
        {
            return cache.get(obj, name);
        }

        std::string GlobalFuncCall::to_string() const
        {
            return name->str() + "(" + FuncCall::to_string() + ")";
        }
        ObjectPtr GlobalFuncCall::eval(Scope & scope) const
        {
            auto self = scope.self();
            auto args = eval_args(scope);
            return (*method(self.get()))(self.get(), args);
        }

        std::string MemberFuncCall::to_string() const
        {
            return lhs->to_string() + "." + name->str() + "(" + FuncCall::to_string() + ")";
        }
        ObjectPtr MemberFuncCall::eval(Scope & scope) const
        {
            auto self = lhs->eval(scope);
            auto args = eval_args(scope);
            return (*method(self.get()))(self.get(), args);
        }

        std::string SafeNavMemberFuncCall::to_string() const
        {
            return lhs->to_string() + "&." + name->str() + "(" + FuncCall::to_string() + ")";
        }
        ObjectPtr SafeNavMemberFuncCall::eval(Scope & scope) const
        {
            auto self = lhs->eval(scope);
            if (self == NIL_VALUE) return NIL_VALUE;
            else
            {
                auto args = eval_args(scope);
                return self->call_method(name, args);
            }
        }

        std::string ElementRefOp::to_string() const
        {
            return lhs->to_string() + "[" + FuncCall::to_string() + "]";
        }
        ObjectPtr ElementRefOp::eval(Scope & scope) const
        {
            auto self = lhs->eval(scope);
            auto args = eval_args(scope);
            return self->el_ref(args);
        }
        std::string ArrayLiteral::to_string() const
        {
            return "[" + FuncCall::to_string() + "]";
        }
        ObjectPtr ArrayLiteral::eval(Scope & scope) const
        {
            auto args = eval_args(scope);
            return make_array(args);
        }
        std::string HashLiteral::to_string() const
        {
            std::stringstream ss;
            ss << "{";
            for (auto i = args.begin(); i != args.end();)
            {
                if (i != args.begin()) ss << ", ";
                ss << (*i++)->to_string();
                ss << " => ";
                ss << (*i++)->to_string();
            }
            ss << "}";
            return ss.str();
        }
        ObjectPtr HashLiteral::eval(Scope & scope) const
        {
            auto args = eval_args(scope);
            return make_hash(args);
        }

        ObjectPtr InclusiveRangeOp::eval(Scope &scope)const
        {
            auto lhs_v = lhs->eval(scope);
            auto rhs_v = rhs->eval(scope);
            return create_object<Range>(lhs_v, rhs_v, false);
        }
        ObjectPtr ExclusiveRangeOp::eval(Scope &scope)const
        {
            auto lhs_v = lhs->eval(scope);
            auto rhs_v = rhs->eval(scope);
            return create_object<Range>(lhs_v, rhs_v, true);
        }

        std::string Block::to_string() const
        {
            std::stringstream ss;
            ss << "{|";
            if (!param_names.empty()) ss << param_names[0]->str();
            for (size_t i = 1; i < param_names.size(); ++i)
                ss << ", " << param_names[i]->str();
            ss << "| ";
            ss << code->to_string();
            ss << "}";
            return ss.str();
        }
        ObjectPtr Block::eval(Scope & scope) const
        {
            return std::make_shared<BlockProc>(*code, param_names, scope);
        }

        std::string Conditional::to_string() const
        {
            return "(" + cond->to_string() + " ? " + true_expr->to_string() + " : " + false_expr->to_string() + ")";
        }
        ObjectPtr Conditional::eval(Scope & scope) const
        {
            if (cond->eval(scope)->is_true())
            {
                return true_expr->eval(scope);
            }
            else
            {
                return false_expr->eval(scope);
            }
        }

        std::string InterpolatedString::to_string() const
        {
            std::string buf = "\"";
            for (auto &node : nodes)
            {
                if (node.expr) buf += "#{" + node.expr->to_string() + "}";
                else buf += node.literal_text;
            }
            buf += "\"";
            return buf;
        }
        ObjectPtr InterpolatedString::eval(Scope & scope) const
        {
            std::string buf;
            for (auto &node : nodes)
            {
                if (node.expr) buf += node.expr->eval(scope)->to_string();
                else buf += node.literal_text;
            }
            return make_value(std::move(buf));
        }

        std::string InterpolatedRegex::to_string()const
        {
            auto src_str = src->to_string();
            //replace quotes
            src_str.front() = '/';
            src_str.back() = '/';
            return src_str;
        }
        ObjectPtr InterpolatedRegex::eval(Scope &scope)const
        {
            auto str = src->eval(scope);
            return create_object<Regexp>(coerce<String>(str)->get_value(), opts);
        }
}
}
