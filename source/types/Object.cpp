#include "types/Object.hpp"
#include "types/Boolean.hpp"
#include "types/Null.hpp"
#include "types/Number.hpp"
#include "types/String.hpp"
#include "Error.hpp"
#include <sstream>

namespace slim
{
    const FunctionTable EMPTY_METHOD_TABLE;

    const std::string Boolean::TYPE_NAME = "Boolean";
    const std::string Null::TYPE_NAME = "Null";
    const std::string Number::TYPE_NAME = "Number";
    const std::string String::TYPE_NAME = "String";

    const std::shared_ptr<Null> NULL_VALUE = std::make_shared<Null>();
    const std::shared_ptr<Boolean> TRUE_VALUE = std::make_shared<Boolean>(true);
    const std::shared_ptr<Boolean> FALSE_VALUE = std::make_shared<Boolean>(false);

    bool Object::eq(const Object *rhs)const
    {
        return this == rhs; //identity
    }
    int Object::cmp(const Object *rhs)const
    {
        throw UnorderableTypeError(this, "cmp", rhs);
    }
    ObjectPtr Object::call_method(const std::string &name, const FunctionArgs &args)
    {
        auto &method = method_table().get(this, name);
        return method(this, args);
    }
    ObjectPtr Object::to_string_obj()const
    {
        return make_value(to_string());
    }
    const MethodTable &Object::method_table()const
    {
        static const MethodTable table =
        {
            { &Object::to_string_obj, "to_s" }
        };
        return table;
    }

    std::string Number::to_string()const
    {
        std::stringstream ss;
        ss << v;
        return ss.str();
    }
    const MethodTable &Boolean::method_table()const
    {
        static const MethodTable table(Object::method_table(),
        {
            { &Boolean::to_f, "to_f" },
            { &Boolean::to_f, "to_d" },
            { &Boolean::to_i, "to_i" }
        });
        return table;
    }
    const MethodTable &Null::method_table()const
    {
        static const MethodTable table(Object::method_table(),
        {
            { &Null::to_f, "to_f" },
            { &Null::to_f, "to_d" },
            { &Null::to_i, "to_i" }
        });
        return table;
    }
    const MethodTable &Number::method_table()const
    {
        static const MethodTable table(Object::method_table(),
        {
            { &Number::to_f, "to_f" },
            { &Number::to_f, "to_d" },
            { &Number::to_i, "to_i" }
        });
        return table;
    }
    const MethodTable &String::method_table()const
    {
        static const MethodTable table(Object::method_table(),
        {
            { &String::to_f, "to_f" },
            { &String::to_f, "to_d" },
            { &String::to_i, "to_i" }
        });
        return table;
    }


    //to_f
    std::shared_ptr<Number> Number::to_f()
    {
        return std::static_pointer_cast<Number>(shared_from_this());
    }
    std::shared_ptr<Number> Null::to_f()
    {
        return make_value(0.0);
    }
    std::shared_ptr<Number> Boolean::to_f()
    {
        return make_value(b ? 1.0 : 0.0);
    }
    std::shared_ptr<Number> String::to_f()
    {
        double d = 0;
        try { d = std::stod(v.c_str()); }
        catch (const std::exception &) {}
        return make_value(d);
    }

    //to_i
    std::shared_ptr<Number> Number::to_i()
    {
        return make_value(std::trunc(v));
    }
    std::shared_ptr<Number> Null::to_i()
    {
        return make_value(0.0);
    }
    std::shared_ptr<Number> Boolean::to_i()
    {
        return make_value(b ? 1.0 : 0.0);
    }
    std::shared_ptr<Number> String::to_i()
    {
        int i = 0;
        try { i = std::stoi(v.c_str()); }
        catch (const std::exception &) {}
        return make_value((double)i);
    }


}

