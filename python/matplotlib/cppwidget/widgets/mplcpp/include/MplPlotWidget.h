#ifndef MPLPLOTWIDGET_H
#define MPLPLOTWIDGET_H

#include "MplFigureCanvas.h"

namespace Python {

/**
 * @brief High-level widget that contains a C++ matplotlib Qt canvas plus
 * optional toolbar.
 *
 * Most users will want to interact with this class instead of the lower level
 * classes. There are convenience functions defined to add high level items
 * instead of having to constantly access the Axes class.
 */
class MplPlotWidget : public QWidget {
  Q_OBJECT
public:
  MplPlotWidget(int subplotLayout, QWidget *parent = nullptr);

  /// Access the canvas
  /// @return A reference to the canvas object
  inline MplFigureCanvas &canvas() const { return *m_canvas; }

  ///@{
  /// @name Convenience methods passing through to the canvas
  inline void addSubPlot(int subplotLayout) {
    m_canvas->addSubPlot(subplotLayout);
  }
  template <typename XArrayType, typename YArrayType>
  inline void plot(XArrayType &&x, YArrayType &&y, const char *format) {
    m_canvas->plot(std::forward<XArrayType>(x), std::forward<YArrayType>(y),
                   format);
  }
  ///@}

private:
  MplFigureCanvas *m_canvas;
};
}
#endif // MPLPLOTWIDGET_H
