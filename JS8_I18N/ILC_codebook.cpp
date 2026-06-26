// JS8CALL-CN ILC codebook loader. [ups]. Ported from ilc_mvp.py (seq=19).
// Reads the whole file as bytes and decodes UTF-8 explicitly so behaviour is
// independent of the QTextStream default codec (Qt5 vs Qt6).
#include "ILC_codebook.h"

#include <QByteArray>
#include <QFile>
#include <QStringList>

namespace {
// Minimal RFC4180-style CSV row parser: handles `"..."` quoted fields so a
// token can itself be `,` or `"`. Mirrors the csv.reader the MVP relied on.
// No embedded-newline support (codebook rows are single-line by convention).
QStringList splitCsvRow(const QString &line)
{
    QStringList out;
    QString cur;
    bool inQuotes = false;
    for (int i = 0; i < line.size(); ++i) {
        const QChar c = line.at(i);
        if (inQuotes) {
            if (c == QLatin1Char('"')) {
                if (i + 1 < line.size() && line.at(i + 1) == QLatin1Char('"')) {
                    cur.append(QLatin1Char('"'));   // "" -> literal "
                    ++i;
                } else {
                    inQuotes = false;               // closing quote
                }
            } else {
                cur.append(c);
            }
        } else if (c == QLatin1Char(',')) {
            out.append(cur);
            cur.clear();
        } else if (c == QLatin1Char('"') && cur.isEmpty()) {
            inQuotes = true;                        // opening quote (only at field start)
        } else {
            cur.append(c);
        }
    }
    out.append(cur);
    return out;
}
} // namespace

bool ILCCodebook::load(const QString &path, QString *err)
{
    enc.clear();
    dec.clear();
    eomTier = eomIdx = -1;
    maxLen = 1;
    version.clear();

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (err) *err = QStringLiteral("cannot open codebook: %1").arg(path);
        return false;
    }
    const QString content = QString::fromUtf8(f.readAll());
    f.close();

    const QStringList lines = content.split(QLatin1Char('\n'));
    bool headerSeen = false;
    for (const QString &raw : lines) {
        QString line = raw;
        if (line.endsWith(QLatin1Char('\r')))
            line.chop(1);                       // CRLF safety
        if (line.startsWith(QLatin1Char('#')))
            continue;                           // comment
        if (line.isEmpty())
            continue;
        if (!headerSeen) {                      // first non-# line = header
            headerSeen = true;
            continue;
        }
        // RFC4180-style split so a token can be `,` or `"` (matches the
        // csv.reader the MVP used; previous naive split mis-parsed `","`).
        const QStringList r = splitCsvRow(line);
        if (r.size() < 4)
            continue;
        if (r.at(0) == QLatin1String("version")) {
            version = r.at(1);              // metadata sentinel row; not a token
            continue;
        }
        bool okT = false, okI = false;
        const int t = r.at(0).toInt(&okT);
        const int i = r.at(1).toInt(&okI);
        if (!okT || !okI)
            continue;
        const QString tok = r.at(2);
        const QString ty  = r.at(3);

        if (ty == QLatin1String("Control") && tok == QLatin1String("<EOM>")) {
            eomTier = t;                        // EOM excluded from enc/dec
            eomIdx  = i;
            continue;
        }
        enc.insert(tok, Entry{t, i});
        dec.insert(key(t, i), tok);
        if (tok.length() > maxLen)
            maxLen = tok.length();
    }

    if (eomTier < 0) {
        if (err) *err = QStringLiteral("EOM not found in codebook");
        return false;
    }
    return true;
}
