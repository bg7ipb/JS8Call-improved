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

// True when `text` contains ≥1 CJK Unified Ideograph (U+4E00..U+9FFF
// main block). Cheap O(n) gate used by the TX hook to decide whether
// to attempt ILC encoding.
bool containsCJK(const QString &text);

} // namespace ILCRuntime

#endif // ILC_RUNTIME_H
