#ifndef ILC_H
#define ILC_H

// JS8CALL-CN ILC codec. [ups] generic i18n codec infra; mirrors JS8_JSC's
// compress/decompress interface. Pure char-sequence <-> bitstream; the codec
// is unaware of ARQ_FLAG/NACK/CRC-8/bit[53]/72-bit wire -- those belong to the
// framer (DRAFT 7.1). Codebook is injected (locale-agnostic engine, DRAFT 7.1
// "given the codebook, deterministic"). Ported from ilc_mvp.py (seq=19).
//
//   tier  selector  idx bits  codelen
//   T0    0         3         4
//   T1    10        6         8
//   T2    110       9         12
//   T3    1110      12        16
//   T4    11110     15        20
//   ESC   11111     + 16-bit BMP code point (5+16)

#include <QList>
#include <QString>
#include <QVector>

#include "ILC_codebook.h"

typedef QVector<bool> Codeword;   // mirror JS8_JSC Codeword

struct ILCStats {
    int tier[5] = {0, 0, 0, 0, 0};
    int esc = 0;
    int tokens = 0;
};

class ILC {
public:
    explicit ILC(const ILCCodebook &codebook) : cb(codebook) {}

    // char sequence -> logical bitstream (incl. trailing EOM), MSB-first.
    // greedy-longest-match (DRAFT 7.1). On a super-BMP code point (unsupported,
    // DRAFT 7.8) sets ok=false and returns the partial codeword.
    Codeword compress(const QString &text, ILCStats *stats = nullptr,
                      bool *ok = nullptr) const;

    // JS8CALL-CN (Slice B2, D4): read-only capability predicate; same truth as
    // compress() -- a code point is encodable iff it is a BMP non-surrogate
    // scalar. Super-BMP (surrogate) is refused.
    static bool canEncode(char32_t cp);

    // logical bitstream -> char sequence. Stops at EOM; trailing zero padding
    // ignored. Unknown (tier,idx) -> U+FFFD.
    QString decompress(const Codeword &bits) const;

    // 51/43-bit logical framer (DRAFT 7.4 D5): chunk/reassembly only. FLAG/NACK/
    // CRC/bit[53] are protocol-layer and intentionally NOT handled here.
    static QList<Codeword> chunk(const Codeword &bits, int width);
    static Codeword        dechunk(const QList<Codeword> &frames);

private:
    const ILCCodebook &cb;
};

#endif // ILC_H
