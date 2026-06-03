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

namespace varicode_test { bool shouldCnRouteForTest(QString const &line); }

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

    // ---- TX assembly: ILC content + compound callsign prepend (Slice B) ----
    out << QString(76, QLatin1Char('-')) << "\n";
    {
        const QString txMycall = QStringLiteral("BG7IPB");
        const QString txMygrid = QStringLiteral("OM89");
        const QStringList txMsgs = {
            QString::fromUtf8("你好"),
            QString::fromUtf8("我的天线是八木，收到你信号很强，今天天气很好"),
        };
        bool txAll = true;
        for (const QString &m : txMsgs) {
            ILCFramer::EncodeResult enc = ILCFramer::encode(ilc, m);
            if (!enc.ok) {
                out << "  tx encode FAIL: " << enc.err << "\n";
                txAll = false;
                continue;
            }
            QList<QPair<QString, int>> tx;
            const QString cmpMsg = QString("`%1 %2").arg(txMycall).arg(txMygrid);
            const QString cmpFrame = Varicode::packCompoundMessage(cmpMsg, nullptr);
            const bool aPrepend = !cmpFrame.isEmpty() && cmpFrame.size() == 12;
            if (!cmpFrame.isEmpty()) tx.append({cmpFrame, Varicode::JS8Call});
            for (auto const &f : enc.frames) tx.append({f, Varicode::JS8Call});
            ILCFramer::DecodeResult dec = ILCFramer::decode(ilc, enc.frames);
            const bool aContent = dec.ok && dec.text == m;
            const bool aCount = tx.size() == enc.frames.size() + 1;
            const bool pass = aPrepend && aContent && aCount;
            out << m.leftJustified(28, QLatin1Char(' '))
                << QString::asprintf(" tx=%d (ilc=%d+pre=1) ", int(tx.size()), int(enc.frames.size()))
                << (pass ? "PASS" : "FAIL") << "\n";
            if (!pass) {
                out << "    prepend=" << aPrepend << " content=" << aContent
                    << " count=" << aCount << " err='" << dec.err << "'\n";
                txAll = false;
            }
        }
        if (!txAll) {
            out << ">>> TX assembly FAIL\n";
            return 7;
        }
        out << ">>> TX assembly PASS\n";
    }

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

    // V11: CN-gate routing (shouldCnRoute) -- Slice B2 / D-69-1..3.
    // Lowercase-ASCII -> ILC (V11.5) is reachable only via this direct library
    // call; the UI path forces toUpper before the gate (seq71), not prod-reachable.
    {
        struct V { QString line; bool expect; const char *tag; };
        QString cjk = QString::fromUtf8("你好 世界");
        V vectors[] = {
            {"CQ CQ CQ OL72",          false, "V11.1 CQ"},
            {"BG7IPB: HEARTBEAT OL72", false, "V11.2 HB"},
            {"BG7IPB: K1ABC SNR -10",  false, "V11.3 directed"},
            {cjk,                      true,  "V11.4 CJK"},
            {"hello",                  true,  "V11.5 lowercase"},
        };
        int fails = 0;
        for (auto const &v : vectors) {
            bool actual = varicode_test::shouldCnRouteForTest(v.line);
            bool pass = (actual == v.expect);
            out << "[V11] " << v.tag << " got=" << int(actual)
                << " exp=" << int(v.expect) << (pass ? " PASS" : " FAIL") << "\n";
            if (!pass) ++fails;
        }
        if (fails) { out << "[V11] " << fails << " FAILED\n"; return 1; }
    }

    {
        out << QString(76, QLatin1Char('-')) << "\n";
        int fails = 0;
        QString const nihao = QString::fromUtf8("你好");
        QString const canon = QString::fromUtf8("我的天线是八木，收到你信号很强，今天天气很好");

        int e1 = Varicode::estimateCnFrames(nihao);
        bool p1 = (e1 == 1);
        out << "[V12.1] estimateCnFrames(short)=" << e1 << " exp=1"
            << (p1 ? " PASS" : " FAIL") << "\n";
        if (!p1) ++fails;

        int e2 = Varicode::estimateCnFrames(canon);
        bool p2 = (e2 == 3);
        out << "[V12.2] estimateCnFrames(canonical)=" << e2 << " exp=3"
            << (p2 ? " PASS" : " FAIL") << "\n";
        if (!p2) ++fails;

        QStringList c3 = Varicode::chunkCnText(canon);
        bool p3 = (c3.size() == 1 && c3.first() == canon);
        out << "[V12.3] chunkCnText(<=cap) nChunks=" << c3.size()
            << " roundtrip=" << int(p3) << (p3 ? " PASS" : " FAIL") << "\n";
        if (!p3) ++fails;

        QString big;
        for (int k = 0; k < 5; ++k) big += canon;
        int e4 = Varicode::estimateCnFrames(big);
        QStringList c4 = Varicode::chunkCnText(big);
        bool eachFits = true;
        for (auto const &c : c4)
            if (Varicode::estimateCnFrames(c) > ILCFramer::kMaxFrames) eachFits = false;
        bool lossless = (c4.join(QString()) == big);
        bool p4 = (e4 > ILCFramer::kMaxFrames) && (c4.size() >= 2) && eachFits && lossless;
        out << "[V12.4] est=" << e4 << " nChunks=" << c4.size()
            << " eachFits=" << int(eachFits) << " lossless=" << int(lossless)
            << (p4 ? " PASS" : " FAIL") << "\n";
        if (!p4) ++fails;

        out << ">>> V12 4-assertion " << (fails ? "FAIL" : "PASS") << "\n";
        if (fails) { out << "[V12] " << fails << " FAILED\n"; return 8; }
    }

    return 0;
}
