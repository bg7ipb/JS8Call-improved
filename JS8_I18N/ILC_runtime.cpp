// JS8CALL-CN ILC process-level runtime. [ups]. See ILC_runtime.h.
#include "ILC_runtime.h"

#include "ILC.h"
#include "ILC_codebook.h"

#include <memory>
#include <mutex>

namespace ILCRuntime {

namespace {

struct State {
    ILCCodebook cb;
    std::unique_ptr<ILC> codec;
    bool ready = false;
};

// Function-local static -> C++11 magic-static thread-safe init.
State &state()
{
    static State s;
    return s;
}

std::once_flag &initFlag()
{
    static std::once_flag f;
    return f;
}

bool g_lastInitOk = false;
QString g_lastInitErr;

} // namespace

bool init(const QString &codebookPath, QString *err)
{
    std::call_once(initFlag(), [&]() {
        State &s = state();
        QString loadErr;
        if (!s.cb.load(codebookPath, &loadErr)) {
            g_lastInitOk = false;
            g_lastInitErr = loadErr;
            return;
        }
        s.codec = std::make_unique<ILC>(s.cb);
        s.ready = true;
        g_lastInitOk = true;
    });

    if (err && !g_lastInitOk) *err = g_lastInitErr;
    return g_lastInitOk;
}

const ILC *instance()
{
    const State &s = state();
    return s.ready ? s.codec.get() : nullptr;
}

bool isReady()
{
    return state().ready;
}

bool containsCJK(const QString &text)
{
    for (const QChar c : text) {
        const ushort u = c.unicode();
        if (u >= 0x4E00 && u <= 0x9FFF) return true;
    }
    return false;
}

} // namespace ILCRuntime
