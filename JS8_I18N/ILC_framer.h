#ifndef ILC_FRAMER_H
#define ILC_FRAMER_H

// JS8CALL-CN 72-bit Compound framer. [ups] protocol-layer adapter sitting on
// top of the ILC codec (DRAFT 7.1 codec/framer split). Owns ARQ_FLAG, anti-APRS
// bit[53] lock, CRC-8/AUTOSAR and cross-frame chunk/reassembly per DRAFT
// §4.2/§4.4; the codec stays unaware of all of those.
//
// Wire layout (DRAFT §4.2 streaming 2-segment, ARQ_FLAG=0 only):
//   bit[0..2]   FrameType = 001 (FrameCompound)
//   bit[3..5]   total/seq: first|single -> total_frames-1 ; subsequent -> seq
//               (frame position first/mid/last is carried by the outer i3bit
//                First/Last, not in this payload)
//   bit[6..8]   langID (1=CN; parametric, never hard-coded)
//   bit[9..16]  CRC-8/AUTOSAR over bit[0..8] ++ bit[17..71] (MSB-first)
//   bit[17]     ARQ_FLAG = 0 (this version emits/consumes data frames only)
//   bit[18..52] ILC payload part1 (35 bits)
//   bit[53]     anti-APRS lock = 1
//   bit[54..71] ILC payload part2 (18 bits)
// Per-frame ILC capacity = 53 bit (35 + 18). 8 frames cap = 424 bit.

#include "ILC.h"

#include <QList>
#include <QString>

namespace ILCFramer {

constexpr int kFramePayloadBits = 53;   // ILC bits per frame, ARQ_FLAG=0
constexpr int kMaxFrames        = 8;    // DRAFT §4.4.1 hard ceiling
constexpr int kFrameCharLen     = 12;   // Varicode::pack72bits output width
constexpr int kLangIdEn         = 0;
constexpr int kLangIdCn         = 1;
constexpr int kLangIdJa         = 2;
constexpr int kLangIdKo         = 3;

struct EncodeResult {
    QList<QString> frames;   // each = 12-char alphabet72 string (pack72bits)
    bool    ok = false;
    QString err;             // human-readable diagnostic when ok=false
};

struct DecodeResult {
    QString text;
    int     langID    = -1;
    int     frameCount = 0;  // declared total (from first or last frame)
    bool    ok = false;
    QString err;
};

// Encode a CN/JA/KO message into 1..8 Compound frames.
// langID defaults to CN; pass kLangIdJa/Ko for future locales.
// Returns ok=false when text exceeds 8 * 51 = 408 bit (caller surfaces, no auto-truncate).
EncodeResult encode(const ILC &codec, const QString &text, int langID = kLangIdCn);

// Decode an ordered list of frames (transmission order) back into text.
// Validates FrameType=001, CRC, bit[53]=1, ARQ_FLAG=0, langID consistency,
// position/seq plausibility. Out-of-order tolerated via bit[5..7] resort.
DecodeResult decode(const ILC &codec, const QList<QString> &frames);

// CRC-8/AUTOSAR (poly 0x2F / init 0xFF / refin=refout=false / xorout 0xFF).
// Exposed for self-test only; impl lives in ILC_framer.cpp (no extra TU).
quint8 crc8Autosar(const quint8 *data, int len);

} // namespace ILCFramer

#endif // ILC_FRAMER_H
