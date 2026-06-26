#ifndef ILC_CODEBOOK_H
#define ILC_CODEBOOK_H

// JS8CALL-CN ILC codebook loader. [ups] generic i18n codec infra.
// Parses a JS8CALL-CN codebook CSV (tier,index,token,type,codelen) into
// encode/decode maps. The codebook is injected into ILC, keeping the codec
// engine locale-agnostic. Ported from i18n-mvp/ilc_mvp.py load_codebook (seq=19).

#include <QHash>
#include <QString>

class ILCCodebook {
public:
    struct Entry { int tier; int idx; };

    // Load from CSV at `path`. false on error (err set). Mirrors the MVP:
    // skip '#' comment lines, skip the first remaining line (header), then
    // rows; <EOM> Control row recorded separately (excluded from enc/dec).
    bool load(const QString &path, QString *err = nullptr);

    bool isValid() const { return eomTier >= 0 && !enc.isEmpty(); }
    int  tokenCount() const { return enc.size(); }

    QHash<QString, Entry> enc;   // token -> (tier,idx)
    QHash<int, QString>   dec;   // key(tier,idx) -> token

    int eomTier = -1;
    int eomIdx  = -1;
    int maxLen  = 1;
    QString version;   // codebook version from sentinel row; empty = unknown (old codebook)

    static int key(int tier, int idx) { return tier * 100000 + idx; }
};

#endif // ILC_CODEBOOK_H
