// JS8CALL-CN ILC codec self-test. Mirrors ilc_mvp.py::run() (seq=19).
// Standalone (links QtCore only). Verifies round-trip + reproduces D8 bit/char.
// Usage: ilc_selftest [path-to-codebook_v0.1.csv]
#include "ILC.h"
#include "ILC_codebook.h"

#include <QCoreApplication>
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

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);

    const QString path = (argc > 1)
        ? QString::fromLocal8Bit(argv[1])
        : QFileInfo(QString::fromLocal8Bit(argv[0])).absolutePath()
              + QStringLiteral("/codebook_v0.1.csv");

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

    const int FRAME = 51, MAXF = 8;
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
    return 0;
}
