#include <switcher.h>

Switcher::Switcher()
    : _running(), _pending()
{
}

void Switcher::trigger(int id)
{
    _pending.set(id, true);
}

void Switcher::next()
{
    do {
        int highest = _pending.last_set();
        int current = _running.last_set();

        if (highest <= current) {
            break;
        }

        assert(!_running.item(highest));
        assert(_dispatch[highest] != nullptr);

        _pending.set(highest, false);

        _running.set(highest, true);
        _dispatch[highest]();
        _running.set(highest, false);
    } while (true);
}
