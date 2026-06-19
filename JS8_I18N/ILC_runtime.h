#ifndef ILC_RUNTIME_H
#define ILC_RUNTIME_H

// JS8CALL-CN ILC process-level runtime. [ups] adapter between the
// locale-agnostic codec/framer (ILC + ILCFramer + ILCCodebook) and the
// app's static TX path (Varicode::buildMessageFrames) / constructor-time
// RX path (DecodedText::DecodedText). Owns a single codebook + codec
// instance for the process; both call sites obtain it via instance().

#include <QString>

class ILC;

namespace ILCRuntime {

// Load codebook from `codebookPath` and construct the singleton ILC.
// Idempotent: first successful call wins; later calls are no-ops returning
// the prior result. Failure → i18n stays off (instance() == nullptr) and
// the app continues; caller writes diagnostic via *err.
bool init(const QString &codebookPath, QString *err = nullptr);

// Returns the singleton codec, or nullptr if init() was never called or
// the last init() failed. TX/RX hooks gate on nullptr to fall back.
const ILC *instance();

bool isReady();

// RX streaming reassembler for the per-offset receive path. Feeds one wire
// `frame` (with its outer i3bit First/Last) into the accumulator keyed by
// `offset`, returning the newly decodable text (delta) for that frame --
// decoding only the contiguous prefix from seq 0. `driftRange` is the +/-Hz
// frequency tolerance (the caller's rxThreshold(submode); 0 disables): a
// continuation frame within +/-driftRange of an existing bucket is merged onto
// it (move-to-newest), so physical-channel drift does not fragment a
// multi-frame message. *ok=false when `frame` is not a valid ILC frame or
// i18n is off, in which case the caller leaves its text unchanged. The
// per-offset accumulator is released after the Last frame or a 90s idle TTL.
QString accumulate(int offset, int driftRange, const QString &frame,
                   bool isFirst, bool isLast, bool *ok = nullptr);

// True when `text` contains ≥1 CJK Unified Ideograph (U+4E00..U+9FFF
// main block). Cheap O(n) gate used by the TX hook to decide whether
// to attempt ILC encoding.
bool containsCJK(const QString &text);

} // namespace ILCRuntime

#endif // ILC_RUNTIME_H
