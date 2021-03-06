#pragma once
#include <memory>
#include <string>
#include <vector>

namespace slim
{
    namespace expr
    {
        class ScopeAttributes;
        class Scope;
    }
    namespace tpl
    {
        class TemplatePart;
    }
    class ViewModel;
    typedef std::shared_ptr<ViewModel> ViewModelPtr;

    /**@brief A parsed template, ready to be rendered using variables in a ViewModel.*/
    class Template
    {
    public:
        Template(std::unique_ptr<tpl::TemplatePart> &&root);
        Template(Template &&);
        Template& operator = (Template &&);
        Template(const Template &) = delete;
        Template& operator = (const Template &) = delete;

        ~Template();

        /**Render this template.
         * @param model Contains the variables for script blocks within the template.
         * @param doctype If true, prefix the HTML5 doctype.
         */
        std::string render(ViewModelPtr model, bool doctype = true)const;
        /**Render this template with an existing variable scope.
         * Used for partials (the "locals" hash param).
         */
        std::string render_partial(expr::Scope &scope);
        /**Render this template with a layout template.
         * This template will be rendered first, and its output, and any "content_for" blocks will
         * then be used for the layouts "yield" output.
         */
        std::string render_layout(Template &layout, ViewModelPtr model, bool doctype = true)const;

        /**Converts the template part into a string representation, mainly for debugging.
         * Because the origenal template structure has all ready been lost, as it was converted
         * to plain HTML fragments with script blocks, the output will use the Ruby ERB syntax,
         * without any whitespace formatting.
         */
        std::string to_string()const;
    private:
        /**The root TemplatePart part. Most likely a TemplatePartsList, but this is not garunteed.*/
        std::unique_ptr<tpl::TemplatePart> root;
    };
}
