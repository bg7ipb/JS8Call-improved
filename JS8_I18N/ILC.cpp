// JS8CALL-CN ILC codec. [ups]. Ported from ilc_mvp.py encode/decode/frame (seq=19).
#include "ILC.h"

namespace {
const int IDXBITS[5] = {3, 6, 9, 12, 15};

inline void appendBits(Codeword &out, quint32 value, int width)
{
    for (int b = width - 1; b >= 0; --b)             // MSB-first
        out.append(((value >> b) & 1u) != 0u);
}

inline void appendToken(Codeword &out, int tier, int idx)
{
    for (int k = 0; k < tier; ++k) out.append(true); // `tier` ones
    out.append(false);                               // '0' terminator
    appendBits(out, static_cast<quint32>(idx), IDXBITS[tier]);
}

inline void appendEsc(Codeword &out, quint16 cp)
{
    for (int k = 0; k < 5; ++k) out.append(true);    // 11111
    appendBits(out, cp, 16);
}

inline quint32 readBits(const Codeword &bs, int p, int width)
{
    quint32 v = 0;
    for (int b = 0; b < width; ++b)
        v = (v << 1) | (bs.at(p + b) ? 1u : 0u);
    return v;
}
} // namespace

Codeword ILC::compress(const QString &text, ILCStats *stats, bool *ok) const
{
    if (ok) *ok = true;
    Codeword out;
    int pos = 0;
    const int n = text.length();
    while (pos < n) {
        bool hit = false;
        const int maxL = qMin(cb.maxLen, n - pos);
        for (int L = maxL; L >= 1; --L) {            // greedy longest first
            const QString sub = text.mid(pos, L);
            const auto it = cb.enc.constFind(sub);
            if (it != cb.enc.constEnd()) {
                appendToken(out, it->tier, it->idx);
                if (stats) { stats->tier[it->tier]++; stats->tokens++; }
                pos += L;
                hit = true;
                break;
            }
        }
        if (hit)
            continue;

        const QChar c = text.at(pos);
        if (c.isHighSurrogate() || c.isLowSurrogate()) {
            if (ok) *ok = false;                     // super-BMP (DRAFT 7.8)
            return out;
        }
        appendEsc(out, c.unicode());
        if (stats) { stats->esc++; stats->tokens++; }
        ++pos;
    }
    appendToken(out, cb.eomTier, cb.eomIdx);         // EOM
    return out;
}

bool ILC::canEncode(char32_t cp)
{
    // Mirrors compress(): any BMP non-surrogate scalar encodes; a super-BMP
    // (surrogate) code point is refused. Read-only, codebook-independent.
    return cp <= 0xFFFFu && !(cp >= 0xD800u && cp <= 0xDFFFu);
}

QString ILC::decompress(const Codeword &bs) const
{
    QString res;
    int p = 0;
    const int n = bs.size();
    while (p < n) {
        int k = 0;
        while (p < n && bs.at(p) && k < 5) { ++k; ++p; }

        if (k == 5) {                                // ESC
            if (p + 16 > n) break;
            res.append(QChar(static_cast<quint16>(readBits(bs, p, 16))));
            p += 16;
            continue;
        }

        int tier;
        if (k == 0) {
            if (p >= n) break;
            ++p;                                     // consume '0'
            tier = 0;
        } else {
            if (p >= n || bs.at(p)) break;           // expect '0'
            ++p;
            tier = k;
        }

        const int ib = IDXBITS[tier];
        if (p + ib > n) break;
        const int idx = static_cast<int>(readBits(bs, p, ib));
        p += ib;

        if (tier == cb.eomTier && idx == cb.eomIdx) break;

        const auto it = cb.dec.constFind(ILCCodebook::key(tier, idx));
        res.append(it != cb.dec.constEnd() ? it.value() : QChar(0xFFFD));
    }
    return res;
}

QList<Codeword> ILC::chunk(const Codeword &bs, int width)
{
    QList<Codeword> frames;
    if (width <= 0) return frames;
    if (bs.isEmpty()) {                              // empty -> one padded frame
        frames.append(Codeword(width, false));
        return frames;
    }
    for (int i = 0; i < bs.size(); i += width) {
        Codeword f = bs.mid(i, width);
        while (f.size() < width) f.append(false);    // pad last with 0
        frames.append(f);
    }
    return frames;
}

Codeword ILC::dechunk(const QList<Codeword> &frames)
{
    Codeword out;
    for (const Codeword &f : frames) out += f;
    return out;
}
