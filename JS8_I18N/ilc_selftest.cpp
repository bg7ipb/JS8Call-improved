// JS8CALL-CN ILC codec self-test. Mirrors ilc_mvp.py::run() (seq=19).
// Standalone (links QtCore only). Verifies round-trip + reproduces D8 bit/char.
// EXEC-22·1: adds CRC-8/AUTOSAR independent check + framer round-trip + V10
// 3-assertion (real CN frame -> Varicode::unpackCompoundMessage compatibility).
// Usage: ilc_selftest [path-to-codebook_v0.1.csv]
#include "ILC.h"
#include "ILC_codebook.h"
#include "ILC_framer.h"
#include "ilc_selftest.h"

#include "../JS8_Main/Varicode.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QTextStream>

static int hanzi(const QString &s)
{
    int c = 0;
    for (const QChar ch : s)
        if (ch.unicode() >= 0x4E00 && ch.unicode() <= 0x9FFF) ++c;
    return c;
}

int runIlcSelftest()
{
    QTextStream out(stdout);

    QString path = QCoreApplication::applicationDirPath()
                   + QStringLiteral("/codebook_v0.1.csv");
    if (!QFile::exists(path))
        path = QCoreApplication::applicationDirPath()
               + QStringLiteral("/../JS8_I18N/codebook_v0.1.csv");

    ILCCodebook cb;
    QString err;
    if (!cb.load(path, &err)) {
        out << "codebook load FAILED: " << err << "\n";
        return 2;
    }
    out << QStringLiteral("codebook: %1 tokens, maxlen=%2, EOM@(%3,%4)\n")
            .arg(cb.tokenCount()).arg(cb.maxLen).arg(cb.eomTier).arg(cb.eomIdx);

    ILC ilc(cb);

    const QStringList msgs = {
        QString::fromUtf8("CQ CQ DE BG7IPB"),
        QString::fromUtf8("你好，信号很好，谢谢！73"),
        QString::fromUtf8("我的天线是八木，收到你信号很强"),
        QString::fromUtf8("今天天气很好，风力三级"),
        QString::fromUtf8("稀有字测试：㐀"),
    };

    const int FRAME = 53, MAXF = 8;
    long ab = 0, ac = 0;
    bool allpass = true;

    out << QString(76, QLatin1Char('-')) << "\n";
    for (const QString &m : msgs) {
        ILCStats st;
        bool ok = true;
        const Codeword bs = ilc.compress(m, &st, &ok);
        const QList<Codeword> frames = ILC::chunk(bs, FRAME);
        const QString rt = ilc.decompress(ILC::dechunk(frames));
        const bool pass = ok && (rt == m);
        allpass = allpass && pass;

        out << m.leftJustified(20, QLatin1Char(' '))
            << QString::asprintf(" ch=%2d hz=%2d bits=%3d bpc=%4.1f fr=%d "
                                 "T0-4=%d/%d/%d/%d/%d esc=%d ",
                                 int(m.length()), hanzi(m), int(bs.size()),
                                 m.length() ? double(bs.size()) / m.length() : 0.0,
                                 int(frames.size()), st.tier[0], st.tier[1], st.tier[2],
                                 st.tier[3], st.tier[4], st.esc)
            << (pass ? "PASS" : "FAIL") << "\n";
        if (!pass)
            out << "    FAIL got=" << rt << "\n";
        if (frames.size() > MAXF)
            out << QStringLiteral("    NOTE: %1 frames > 8 (over capacity)\n")
                       .arg(frames.size());
        ab += bs.size();
        ac += m.length();
    }
    out << QString(76, QLatin1Char('-')) << "\n";
    const double o = ac ? double(ab) / ac : 0.0;
    out << QString::asprintf("aggregate: chars=%ld bits=%ld  bit/char=%.2f\n", ac, ab, o);
    out << QString::asprintf("8-frame budget=%d bit (FLAG=0) -> ~%.0f char/8 frames\n",
                             FRAME * MAXF, o ? FRAME * MAXF / o : 0.0);
    out.flush();
    if (!allpass) {
        out << ">>> round-trip FAIL\n";
        return 1;
    }
    out << ">>> ALL round-trip PASS\n";

    // ---- CRC-8/AUTOSAR independent check (input "123456789" -> 0xDF) ----
    out << QString(76, QLatin1Char('-')) << "\n";
    {
        const QByteArray check = QByteArrayLiteral("123456789");
        const quint8 got = ILCFramer::crc8Autosar(
            reinterpret_cast<const quint8 *>(check.constData()), check.size());
        const bool ok = (got == 0xDF);
        out << QString::asprintf("CRC-8/AUTOSAR(\"123456789\") = 0x%02X (want 0xDF) %s\n",
                                 got, ok ? "PASS" : "FAIL");
        if (!ok) return 3;
    }

    // ---- Framer round-trip vectors (incl. multi-frame >53 bit) ----
    out << QString(76, QLatin1Char('-')) << "\n";
    const QStringList framerMsgs = {
        QString::fromUtf8("你好"),                   // tiny, 1 frame
        QString::fromUtf8("信号很好谢谢73"),         // mid, 1 frame border
        QString::fromUtf8("我的天线是八木，收到你信号很强，今天天气很好"), // multi-frame
    };
    bool framerAll = true;
    for (const QString &m : framerMsgs) {
        ILCFramer::EncodeResult enc = ILCFramer::encode(ilc, m);
        if (!enc.ok) {
            out << "  framer encode FAIL: " << enc.err << "\n";
            framerAll = false;
            continue;
        }
        ILCFramer::DecodeResult dec = ILCFramer::decode(ilc, enc.frames);
        const bool pass = dec.ok && dec.text == m
                          && dec.langID == ILCFramer::kLangIdCn
                          && dec.frameCount == enc.frames.size();
        out << m.leftJustified(28, QLatin1Char(' '))
            << QString::asprintf(" frames=%d lang=%d ", int(enc.frames.size()), dec.langID)
            << (pass ? "PASS" : "FAIL") << "\n";
        if (!pass) {
            out << "    got text='" << dec.text << "' err='" << dec.err << "'\n";
            framerAll = false;
        }
    }
    if (!framerAll) {
        out << ">>> framer round-trip FAIL\n";
        return 4;
    }
    out << ">>> framer round-trip PASS\n";

    // ---- Streaming accumulator: feed a multi-frame message frame-by-frame --
    out << QString(76, QLatin1Char('-')) << "\n";
    {
        const QString streamMsg =
            QString::fromUtf8("我的天线是八木，收到你信号很强，今天天气很好");
        ILCFramer::EncodeResult senc = ILCFramer::encode(ilc, streamMsg);
        bool streamOk = senc.ok && senc.frames.size() >= 2;
        if (!streamOk) {
            out << "stream setup FAIL (need multi-frame): " << senc.err << "\n";
            return 5;
        }
        ILCFramer::StreamAccumulator acc;
        QString assembled;
        const int n = senc.frames.size();
        for (int i = 0; i < n; ++i) {
            bool fok = false;
            const QString delta = acc.feed(ilc, senc.frames.at(i),
                                           i == 0, i == n - 1, &fok);
            assembled += delta;
            out << QString::asprintf("  frame %d/%d ok=%d delta='", i + 1, n, fok ? 1 : 0)
                << delta << "' cum='" << assembled << "'\n";
            if (!fok) streamOk = false;
        }
        const bool pass = streamOk && assembled == streamMsg;
        out << "stream incremental "
            << QString::asprintf("frames=%d ", n)
            << (pass ? "PASS" : "FAIL") << "\n";
        if (!pass) {
            out << "    cum='" << assembled << "' want='" << streamMsg << "'\n";
            return 5;
        }
    }
    out << ">>> stream accumulator PASS\n";

    // ---- V10: real CN frame -> Varicode::unpackCompoundMessage compat ----
    out << QString(76, QLatin1Char('-')) << "\n";
    {
        ILCFramer::EncodeResult enc =
            ILCFramer::encode(ilc, QString::fromUtf8("你好73"));
        if (!enc.ok || enc.frames.isEmpty()) {
            out << "V10 setup FAIL: " << enc.err << "\n";
            return 5;
        }
        const QString wire = enc.frames.first();
        quint8 vtype = 0, vbits3 = 0;
        QStringList unpacked = Varicode::unpackCompoundMessage(wire, &vtype, &vbits3);

        const bool a1 = (vtype == Varicode::FrameCompound);
        // unpackCompoundFrame seeds list = [callsign, ""]; the grid/cmd append
        // branches in unpackCompoundMessage gate on `extra` range. bit[53]=1
        // forces extra > nmaxgrid -> neither branch fires -> list stays length 2
        // with index 1 still empty. Any non-empty index>=1 = grid/cmd leaked.
        bool a2 = (unpacked.size() == 2);
        for (int i = 1; a2 && i < unpacked.size(); ++i)
            if (!unpacked.at(i).isEmpty()) a2 = false;
        const bool a3 = true; // no crash reaching here

        out << QString::asprintf("V10 a1(type==FrameCompound) = %s\n", a1 ? "PASS" : "FAIL");
        out << QString::asprintf("V10 a2(no grid/cmd append)  = %s (got %d entries)\n",
                                 a2 ? "PASS" : "FAIL", int(unpacked.size()));
        out << QString::asprintf("V10 a3(no crash)            = %s\n", a3 ? "PASS" : "FAIL");
        if (!(a1 && a2 && a3)) {
            for (const QString &u : unpacked) out << "    entry='" << u << "'\n";
            return 6;
        }
        out << ">>> V10 3-assertion PASS\n";
    }

    return 0;
}
