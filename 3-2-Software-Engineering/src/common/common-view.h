
//
// Air Conditioner - Common MVC View
// BOT Man, 2017
//

#ifndef AC_COMMON_VIEW_H
#define AC_COMMON_VIEW_H

#include <memory>
#include <functional>

namespace Air_Conditioner
{
    class ViewBase
    {
    public:
        virtual void Show () = 0;
        virtual ~ViewBase () {}  // Attention!!!
    };

    class ViewManager
    {
        std::unique_ptr<ViewBase> _viewNxt;
        std::unique_ptr<ViewBase> _viewCur;

    protected:
        template<typename View, typename... Args>
        void _Navigate (Args && ...args)
        {
            _viewNxt = std::make_unique<View> (std::forward<Args> (args)...);
        }

    public:
        virtual ~ViewManager () {}  // Attention!!!

        void Start ()
        {
            while (_viewNxt)  // Break until there is no page to show
            {
                _viewCur = std::move (_viewNxt);
                _viewCur->Show ();  // Block Here
            }
        }
    };
}

#endif // !AC_COMMON_VIEW_H