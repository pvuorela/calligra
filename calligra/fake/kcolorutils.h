#ifndef FAKE_KCOLORUTILS_H
#define FAKE_KCOLORUTILS_H

#include <QColor>
#include <QPainter>

namespace KColorUtils {
    qreal luma(const QColor& c) { return 0.299 * c.redF() + 0.587 * c.greenF() + 0.114 * c.blueF(); }
    QColor lighten(const QColor& c, qreal amount = 0.5, qreal chromaInverseGain = 1.0) { return c.lighter(amount * 100.0); }
    QColor darken(const QColor& c, qreal amount = 0.5, qreal chromaGain = 1.0) { return c.darker(amount * 100.0); }
    QColor shade(const QColor& c, qreal lumaAmount, qreal chromaAmount = 0.0) { return c; }
    QColor tint(const QColor &base, const QColor &color, qreal amount = 0.3) { return color; }
    QColor mix(const QColor &c1, const QColor &c2, qreal bias = 0.5) { return c1; }
    QColor overlayColors(const QColor &base, const QColor &paint, QPainter::CompositionMode comp = QPainter::CompositionMode_SourceOver) { return paint; }

    qreal contrastRatioForLuma(qreal y1, qreal y2)
    {
        if (y1 > y2)
            return (y1 + 0.05) / (y2 + 0.05);
        else
            return (y2 + 0.05) / (y1 + 0.05);
    }

    qreal contrastRatio(const QColor& a, const QColor& b) { return contrastRatioForLuma(luma(a) , luma(b)); }
}

#endif
