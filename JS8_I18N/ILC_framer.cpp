// JS8CALL-CN 72-bit Compound framer. [ups]. Bit contract = DRAFT §4.2/§4.4.
#include "ILC_framer.h"

#include "../JS8_Main/Varicode.h"

#include <QVector>

namespace ILCFramer {

namespace {

using Bits = QVector<bool>;

constexpr quint8 kFrameTypeCompound = 0b001;   // Varicode::FrameCompound
constexpr int    kPosSingle = 0b00;
constexpr int    kPosFirst  = 0b01;
constexpr int    kPosMid    = 0b10;
constexpr int    kPosLast   = 0b11;

inline void writeBits(Bits &dst, int offset, quint32 value, int width)
{
    for (int i = 0; i < width; ++i)
        dst[offset + i] = ((value >> (width - 1 - i)) & 1u) != 0u;
}

inline quint32 readBits(const Bits &src, int offset, int width)
{
    quint32 v = 0;
    for (int i = 0; i < width; ++i)
        v = (v << 1) | (src.at(offset + i) ? 1u : 0u);
    return v;
}

// Pack the 64 bit input span of the CRC into 8 bytes, bit[0]=byte0 MSB
// (DRAFT §4.2 CRC spec). Input span = bit[0..8] ++ bit[17..71] of the wire.
QByteArray collectCrcInput(const Bits &frame)
{
    QByteArray bytes(8, 0);
    int p = 0;
    auto push = [&](bool b) {
        bytes[p >> 3] = static_cast<char>(
            static_cast<quint8>(bytes[p >> 3]) | (b ? (1u << (7 - (p & 7))) : 0u));
        ++p;
    };
    for (int i = 0; i <= 8; ++i) push(frame.at(i));
    for (int i = 17; i <= 71; ++i) push(frame.at(i));
    // 9 + 55 = 64 -> exactly fills 8 bytes
    return bytes;
}

} // namespace

quint8 crc8Autosar(const quint8 *data, int len)
{
    quint8 crc = 0xFF;
    for (int i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b)
            crc = (crc & 0x80) ? static_cast<quint8>((crc << 1) ^ 0x2F)
                               : static_cast<quint8>(crc << 1);
    }
    return crc ^ 0xFF;
}

EncodeResult encode(const ILC &codec, const QString &text, int langID)
{
    EncodeResult r;
    if (langID < 0 || langID > 7) {
        r.err = QStringLiteral("langID out of 3-bit range: %1").arg(langID);
        return r;
    }

    bool codecOk = true;
    Bits payload = codec.compress(text, nullptr, &codecOk);
    if (!codecOk) {
        r.err = QStringLiteral("ILC::compress refused input (super-BMP code point)");
        return r;
    }

    const QList<Bits> chunks = ILC::chunk(payload, kFramePayloadBits);
    if (chunks.size() > kMaxFrames) {
        r.err = QStringLiteral("message exceeds 8-frame cap: %1 chunks (>%2 bit)")
                    .arg(chunks.size())
                    .arg(kMaxFrames * kFramePayloadBits);
        return r;
    }

    const int n = chunks.size();
    for (int i = 0; i < n; ++i) {
        Bits frame(72, false);

        // bit[0..2] FrameType=001
        writeBits(frame, 0, kFrameTypeCompound, 3);

        // bit[3..5] total/seq field: first|single -> total-1 ; subsequent -> seq
        // (frame position first/mid/last lives in the outer i3bit First/Last)
        const int b35 = (i == 0) ? (n - 1) : i;
        writeBits(frame, 3, static_cast<quint32>(b35), 3);

        // bit[6..8] langID
        writeBits(frame, 6, static_cast<quint32>(langID), 3);

        // bit[9..16] CRC placeholder (filled after payload)
        // bit[17] ARQ_FLAG=0
        frame[17] = false;

        // bit[18..52] ILC part1 (35 bits) ; bit[54..71] ILC part2 (18 bits)
        const Bits &c = chunks[i];
        for (int k = 0; k < 35; ++k) frame[18 + k] = c.at(k);
        frame[53] = true;                       // anti-APRS lock
        for (int k = 0; k < 18; ++k) frame[54 + k] = c.at(35 + k);

        const QByteArray crcInput = collectCrcInput(frame);
        const quint8 crc = crc8Autosar(
            reinterpret_cast<const quint8 *>(crcInput.constData()),
            crcInput.size());
        writeBits(frame, 9, crc, 8);

        // 72 bits -> 64-bit value + 8-bit rem (DRAFT bit[0]=MSB of value)
        const quint64 value = Varicode::bitsToInt(frame.mid(0, 64));
        const quint8  rem   = static_cast<quint8>(Varicode::bitsToInt(frame.mid(64, 8)));
        r.frames.append(Varicode::pack72bits(value, rem));
    }

    r.ok = true;
    return r;
}

DecodeResult decode(const ILC &codec, const QList<QString> &frames)
{
    DecodeResult r;
    if (frames.isEmpty()) {
        r.err = QStringLiteral("empty frame list");
        return r;
    }
    if (frames.size() > kMaxFrames) {
        r.err = QStringLiteral("frame list exceeds 8-frame cap: %1").arg(frames.size());
        return r;
    }

    struct Slot { int seq; Bits payload; };
    QList<Slot> rxSlots;
    int declaredTotal = -1;
    int langID = -1;

    for (int fi = 0; fi < frames.size(); ++fi) {
        const QString &f = frames.at(fi);
        if (f.length() != kFrameCharLen) {
            r.err = QStringLiteral("frame length != 12 chars: %1").arg(f.length());
            return r;
        }

        quint8 rem = 0;
        const quint64 value = Varicode::unpack72bits(f, &rem);
        Bits bits = Varicode::intToBits(value, 64) + Varicode::intToBits(rem, 8);
        if (bits.size() != 72) {
            r.err = QStringLiteral("bit reassembly produced %1 bits").arg(bits.size());
            return r;
        }

        if (readBits(bits, 0, 3) != kFrameTypeCompound) {
            r.err = QStringLiteral("FrameType != FrameCompound (got %1)")
                        .arg(readBits(bits, 0, 3));
            return r;
        }
        if (!bits.at(53)) { r.err = QStringLiteral("bit[53] anti-APRS lock not set"); return r; }
        if (bits.at(17)) {
            r.err = QStringLiteral("ARQ_FLAG=1 not supported in this version");
            return r;
        }

        const QByteArray crcInput = collectCrcInput(bits);
        const quint8 crcCalc = crc8Autosar(
            reinterpret_cast<const quint8 *>(crcInput.constData()),
            crcInput.size());
        const quint8 crcWire = static_cast<quint8>(readBits(bits, 9, 8));
        if (crcCalc != crcWire) {
            r.err = QStringLiteral("CRC mismatch wire=0x%1 calc=0x%2")
                        .arg(crcWire, 2, 16, QLatin1Char('0'))
                        .arg(crcCalc, 2, 16, QLatin1Char('0'));
            return r;
        }

        const int frameLang = static_cast<int>(readBits(bits, 6, 3));
        if (langID < 0) langID = frameLang;
        else if (langID != frameLang) {
            r.err = QStringLiteral("langID mismatch across frames: %1 vs %2")
                        .arg(langID).arg(frameLang);
            return r;
        }

        // position now derives from list order (the outer i3bit First/Last owns
        // it on the wire); bit[3..5] is the single total/seq field.
        int pos;
        if (frames.size() == 1)            pos = kPosSingle;
        else if (fi == 0)                  pos = kPosFirst;
        else if (fi == frames.size() - 1)  pos = kPosLast;
        else                               pos = kPosMid;
        const int b57 = static_cast<int>(readBits(bits, 3, 3));
        int seq, total;
        switch (pos) {
        case kPosSingle: seq = 0; total = 1; break;
        case kPosFirst:  seq = 0; total = b57 + 1; break;
        case kPosMid:    seq = b57; total = -1; break;
        case kPosLast:   seq = b57; total = b57 + 1; break;
        default: r.err = QStringLiteral("invalid position bits"); return r;
        }
        if (total > 0) {
            if (declaredTotal < 0) declaredTotal = total;
            else if (declaredTotal != total) {
                r.err = QStringLiteral("total-frames disagreement: %1 vs %2")
                            .arg(declaredTotal).arg(total);
                return r;
            }
        }

        Bits payload(kFramePayloadBits, false);
        for (int k = 0; k < 35; ++k) payload[k]      = bits.at(18 + k);
        for (int k = 0; k < 18; ++k) payload[35 + k] = bits.at(54 + k);
        rxSlots.append({seq, payload});
    }

    std::sort(rxSlots.begin(), rxSlots.end(),
              [](const Slot &a, const Slot &b) { return a.seq < b.seq; });
    for (int i = 0; i < rxSlots.size(); ++i) {
        if (rxSlots[i].seq != i) {
            r.err = QStringLiteral("non-contiguous seq numbers (slot %1 has seq %2)")
                        .arg(i).arg(rxSlots[i].seq);
            return r;
        }
    }
    if (declaredTotal > 0 && declaredTotal != rxSlots.size()) {
        r.err = QStringLiteral("declared total %1 != received %2")
                    .arg(declaredTotal).arg(rxSlots.size());
        return r;
    }

    // T1 token-aware framing: every frame self-terminates at its all-1s
    // padding / EOM, so decompress each frame separately and concatenate.
    // The old whole-list dechunk spliced padding into the bitstream and
    // corrupted tokens straddling a frame boundary (seq102 multi-frame bug).
    for (const Slot &s : rxSlots)
        r.text += codec.decompress(ILC::dechunk(QList<Bits>{s.payload}));
    r.langID     = langID;
    r.frameCount = rxSlots.size();
    r.ok         = true;
    return r;
}

FrameView validateFrame(const QString &frame)
{
    FrameView v;
    if (frame.length() != kFrameCharLen) {
        v.err = QStringLiteral("frame length != 12 chars: %1").arg(frame.length());
        return v;
    }

    quint8 rem = 0;
    const quint64 value = Varicode::unpack72bits(frame, &rem);
    Bits bits = Varicode::intToBits(value, 64) + Varicode::intToBits(rem, 8);
    if (bits.size() != 72) {
        v.err = QStringLiteral("bit reassembly produced %1 bits").arg(bits.size());
        return v;
    }

    if (readBits(bits, 0, 3) != kFrameTypeCompound) {
        v.err = QStringLiteral("FrameType != FrameCompound (got %1)")
                    .arg(readBits(bits, 0, 3));
        return v;
    }
    if (!bits.at(53)) { v.err = QStringLiteral("bit[53] anti-APRS lock not set"); return v; }
    if (bits.at(17)) {
        v.err = QStringLiteral("ARQ_FLAG=1 not supported in this version");
        return v;
    }

    const QByteArray crcInput = collectCrcInput(bits);
    const quint8 crcCalc = crc8Autosar(
        reinterpret_cast<const quint8 *>(crcInput.constData()), crcInput.size());
    const quint8 crcWire = static_cast<quint8>(readBits(bits, 9, 8));
    if (crcCalc != crcWire) {
        v.err = QStringLiteral("CRC mismatch wire=0x%1 calc=0x%2")
                    .arg(crcWire, 2, 16, QLatin1Char('0'))
                    .arg(crcCalc, 2, 16, QLatin1Char('0'));
        return v;
    }

    v.langID        = static_cast<int>(readBits(bits, 6, 3));
    v.totalSeqField = static_cast<int>(readBits(bits, 3, 3));

    Codeword payload(kFramePayloadBits, false);
    for (int k = 0; k < 35; ++k) payload[k]      = bits.at(18 + k);
    for (int k = 0; k < 18; ++k) payload[35 + k] = bits.at(54 + k);
    v.payload = payload;
    v.ok = true;
    return v;
}

void StreamAccumulator::reset()
{
    payloads_.clear();
    shownChars_    = 0;
    langID_        = -1;
    declaredTotal_ = -1;
}

QString StreamAccumulator::feed(const ILC &codec, const QString &frame,
                                bool isFirst, bool isLast, bool *ok)
{
    if (ok) *ok = false;
    const FrameView v = validateFrame(frame);
    if (!v.ok) return {};
    if (v.langID < kLangIdCn || v.langID > kLangIdKo) return {};

    if (isFirst) reset();
    if (langID_ < 0) langID_ = v.langID;
    else if (langID_ != v.langID) return {};   // langID flip mid-message

    // seq: first|single -> 0 ; mid|last -> raw bit[3..5].
    const int seq = isFirst ? 0 : v.totalSeqField;
    if (isFirst || isLast) declaredTotal_ = v.totalSeqField + 1;
    payloads_.insert(seq, v.payload);

    // Decode the contiguous prefix from seq 0; stop at the first gap.
    // T1 token-aware framing: decompress each frame separately and
    // concatenate -- each self-terminates at padding/EOM, so the old
    // whole-prefix dechunk spliced padding mid-stream (seq102 multi-frame bug).
    QString full;
    for (int s = 0; payloads_.contains(s); ++s)
        full += codec.decompress(ILC::dechunk(QList<Codeword>{payloads_.value(s)}));

    QString delta;
    if (full.size() > shownChars_) {
        delta = full.mid(shownChars_);
        shownChars_ = full.size();
    }
    if (ok) *ok = true;
    return delta;
}

} // namespace ILCFramer
