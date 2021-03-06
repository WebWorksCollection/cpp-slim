#pragma once
#include "Object.hpp"
namespace slim
{
    class Nil;
    extern const std::shared_ptr<Nil> NIL_VALUE;
    /**The "nil" singleton type.
     * Additional instances should not be created, only the global slim::NIL_VALUE instance should
     * be used.
     */
    class Nil : public Object
    {
    public:
        template<class T>
        static std::shared_ptr<T> create()
        {
            return NIL_VALUE;
        }

        explicit Nil() {}

        static const std::string &name()
        {
            static const std::string TYPE_NAME = "Nil";
            return TYPE_NAME;
        }
        virtual const std::string& type_name()const override { return name(); }
        virtual std::string to_string()const override { return ""; }
        virtual std::string inspect()const override  { return "nil"; }
        virtual bool is_true()const override { return false; }
        virtual size_t hash()const { return 0; }
        std::shared_ptr<Number> to_f();
        std::shared_ptr<Number> to_i();
    protected:
        virtual const MethodTable &method_table()const;
    };
}
