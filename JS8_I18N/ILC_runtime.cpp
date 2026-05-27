// JS8CALL-CN ILC process-level runtime. [ups]. See ILC_runtime.h.
#include "ILC_runtime.h"

#include "ILC.h"
#include "ILC_codebook.h"
#include "ILC_framer.h"

#include <memory>
#include <mutex>
#include <QHash>

namespace ILCRuntime {

namespace {

struct State {
    ILCCodebook cb;
    std::unique_ptr<ILC> codec;
    bool ready = false;
    QHash<int, ILCFramer::StreamAccumulator> accs;  // per-offset RX reassembly
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

QString accumulate(int offset, const QString &frame,
                   bool isFirst, bool isLast, bool *ok)
{
    const ILC *c = instance();
    // Short-circuit before touching the per-offset map: non-ILC frames (the
    // common case) must not default-insert an accumulator that is never
    // released. validateFrame is cheap; feed() re-validates internally.
    if (!c || !ILCFramer::validateFrame(frame).ok) {
        if (ok) *ok = false;
        return {};
    }
    State &s = state();
    // JS8CALL-CN: callsign-prepend takes the i3bit JS8CallFirst on the first ILC
    // frame, so synthesize isFirst when this offset slot is fresh; otherwise the
    // opening Compound frame never lands in seq 0 and decode returns empty.
    const bool firstForSlot = isFirst || !s.accs.contains(offset);
    const QString delta =
        s.accs[offset].feed(*c, frame, firstForSlot, isLast, ok);
    if (isLast) s.accs.remove(offset);
    return delta;
}

} // namespace ILCRuntime
