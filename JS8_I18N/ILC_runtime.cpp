// JS8CALL-CN ILC process-level runtime. [ups]. See ILC_runtime.h.
#include "ILC_runtime.h"

#include "ILC.h"
#include "ILC_codebook.h"
#include "ILC_framer.h"

#include <memory>
#include <mutex>
#include <QHash>
#include <QDateTime>

namespace ILCRuntime {

namespace {

struct State {
    ILCCodebook cb;
    std::unique_ptr<ILC> codec;
    bool ready = false;
    QHash<int, ILCFramer::StreamAccumulator> accs;  // per-offset RX reassembly
    QHash<int, qint64> lastSeen;  // per-offset last-feed ms, for idle TTL sweep
};

constexpr qint64 kIlcBucketTtlMs = 90 * 1000;  // mirror upstream message-buffer TTL

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

QString accumulate(int offset, int driftRange, const QString &frame,
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
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    // TTL sweep: a frame may drift past +/-driftRange in one step, or a sender
    // vanish, stranding a bucket that never sees its Last frame and is never
    // released. Drop buckets idle past kIlcBucketTtlMs (mirrors upstream's 90s
    // message-buffer TTL in processBufferedActivity.cpp). Reached only for
    // valid ILC frames, so the common (non-CN) decode pays nothing.
    for (auto it = s.lastSeen.begin(); it != s.lastSeen.end(); ) {
        if (nowMs - it.value() > kIlcBucketTtlMs) {
            s.accs.remove(it.key());
            it = s.lastSeen.erase(it);
        } else {
            ++it;
        }
    }
    // Frequency-drift tolerance: physical-channel drift shifts a multi-frame
    // message's per-frame decode offset within +/-driftRange Hz. The upstream
    // text path absorbs this via hasExistingMessageBuffer's +/-range
    // move-to-newest (mainwindow.cpp); the per-offset ILC accumulator must do
    // the same or a drifting message fragments across buckets, each incomplete.
    // Merge a continuation frame onto the nearest existing bucket within range
    // (move-to-newest). isFirst starts a fresh message at its own offset --
    // skip the merge so a new transmission cannot hijack a nearby in-progress
    // bucket. driftRange == 0 (the constant-key TX paths) is a no-op merge.
    if (!isFirst && !s.accs.contains(offset)) {
        for (int probe = offset - driftRange; probe <= offset + driftRange; ++probe) {
            if (probe == offset) continue;
            if (s.accs.contains(probe)) {
                s.accs[offset] = s.accs.take(probe);
                s.lastSeen.remove(probe);
                break;
            }
        }
    }
    // JS8CALL-CN: isFirst comes from the outer i3bit JS8CallFirst, which the
    // TX side (Varicode buildMessageFrames + cnFirstIlcIdx) plants on the first
    // ILC content frame. No synthesis needed; mid-join works because the
    // accumulator's decode run starts from the lowest received seq.
    const bool firstForSlot = isFirst;
    const QString delta =
        s.accs[offset].feed(*c, frame, firstForSlot, isLast, ok);
    if (isLast) {
        s.accs.remove(offset);
        s.lastSeen.remove(offset);
    } else {
        s.lastSeen[offset] = nowMs;
    }
    return delta;
}

} // namespace ILCRuntime
