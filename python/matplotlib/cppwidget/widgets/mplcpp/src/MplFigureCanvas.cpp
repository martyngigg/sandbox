#include "MplFigureCanvas.h"
#include "NDArray1D.h"
#include "PythonErrors.h"
#include "SipUtils.h"

#include <QVBoxLayout>

#include <iostream>

namespace {
//------------------------------------------------------------------------------
// Static constants/functions
//------------------------------------------------------------------------------
#if QT_VERSION >= QT_VERSION_CHECK(4, 0, 0) &&                                 \
    QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
// Define PyQt version and matplotlib backend
const char *PYQT_MODULE = "PyQt4";
const char *MPL_QT_BACKEND = "matplotlib.backends.backend_qt4agg";

#elif QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) &&                               \
    QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
// Define PyQt version and matplotlib backend
const char *PYQT_MODULE = "PyQt5";
const char *MPL_QT_BACKEND = "matplotlib.backends.backend_qt5agg";

#else
#error "Unknown Qt version. Cannot determine matplotlib backend."
#endif

// Return static instance of figure type
const Python::PythonObject &mplFigureType() {
  static Python::PythonObject figureType;
  if (figureType.isNone())
    figureType = Python::getAttrOnModule("matplotlib.figure", "Figure");
  return figureType;
}

// Return static instance of figure canvas type
const Python::PythonObject &mplFigureCanvasType() {
  static Python::PythonObject figureCanvasType;
  if (figureCanvasType.isNone()) {
    // Importing PyQt version first helps matplotlib select the correct backend.
    // We should do this in some kind of initialisation routine
    Python::importModule(PYQT_MODULE);
    auto figureCanvasBaseType =
        Python::getAttrOnModule(MPL_QT_BACKEND, "FigureCanvasQTAgg");
    PyObject *pClassName = PyBytes_FromString("MyMplCanvas");
    PyObject *pClassBases = PyTuple_New(1); // An empty tuple for bases is equivalent to `(object,)`
    PyTuple_SetItem(pClassBases, 0, figureCanvasBaseType.get());
    PyObject *pClassDic = PyDict_New();

    // pClass = type(pClassName, pClassBases, pClassDic)
    figureCanvasType = Python::PythonObject(Python::NewRef(PyObject_CallFunctionObjArgs((PyObject*)&PyType_Type, pClassName, pClassBases, pClassDic, NULL)));

    Py_CLEAR(pClassName);
    Py_CLEAR(pClassBases);
    Py_CLEAR(pClassDic);
  }
  return figureCanvasType;
}
}

namespace Python {
//------------------------------------------------------------------------------
// MplFigureCanvas::PyObjectHolder - Private implementation
//------------------------------------------------------------------------------
struct MplFigureCanvas::PyObjectHolder {
  PyObjectHolder(int subplotLayout) {
    // Create a figure and attach it to a canvas object. This creates a
    // blank widget
    PythonObject figure(
        NewRef(PyObject_CallObject(mplFigureType().get(), NULL)));
    PythonObject(
        NewRef(PyObject_CallMethod(figure.get(), PYSTR_LITERAL("add_subplot"),
                                   PYSTR_LITERAL("i"), subplotLayout)));
    auto instance = PyObject_CallFunction(mplFigureCanvasType().get(),
                                          PYSTR_LITERAL("(O)"), figure.get());
    if (!instance) {
      throw PythonError(errorToString());
    }
    canvas = PythonObject(NewRef(instance));
  }

  /**
   * Return the Axes object that is currently active. Analogous to figure.gca()
   * @return matplotlib.axes.Axes object
   */
  PythonObject gca() {
    auto figure = PythonObject(
        NewRef(PyObject_GetAttrString(canvas.get(), PYSTR_LITERAL("figure"))));
    return PythonObject(NewRef(PyObject_CallMethod(
        figure.get(), PYSTR_LITERAL("gca"), PYSTR_LITERAL(""), nullptr)));
  }

  PythonObject canvas;
};

//------------------------------------------------------------------------------
// MplFigureCanvas
//------------------------------------------------------------------------------
/**
 * @brief Constructs an empty plot widget with the given subplot layout.
 *
 * @param subplotLayout The sublayout geometry defined in matplotlib's
 * convenience format: [Default=111]. See
 * https://matplotlib.org/api/pyplot_api.html#matplotlib.pyplot.subplot
 *
 * @param parent A pointer to the parent widget, can be nullptr
 */
MplFigureCanvas::MplFigureCanvas(int subplotLayout, QWidget *parent)
    : QWidget(parent), m_pydata(new PyObjectHolder(subplotLayout)) {
  setLayout(new QVBoxLayout);
  auto cpp = static_cast<QWidget *>(sipUnwrap(m_pydata->canvas.get()));
  assert(cpp);
  layout()->addWidget(cpp);
}

/**
 * @brief Destroys the object
 */
MplFigureCanvas::~MplFigureCanvas() { delete m_pydata; }

/**
 * Retrieve information about the subplot geometry
 * @return
 */
SubPlotSpec MplFigureCanvas::subplotSpec() const {
  auto axes = m_pydata->gca();
  auto geometry = PythonObject(NewRef(PyObject_CallMethod(
      axes.get(), PYSTR_LITERAL("get_geometry"), PYSTR_LITERAL(""), nullptr)));

  return SubPlotSpec(PyInt_AS_LONG(PyTuple_GET_ITEM(geometry.get(), 0)),
                     PyInt_AS_LONG(PyTuple_GET_ITEM(geometry.get(), 1)));
}

/**
 * Call FigureCanvas.draw method on base class
 */
void MplFigureCanvas::draw() {
  PythonObject(
      NewRef(PyObject_CallMethod(m_pydata->canvas.get(), PYSTR_LITERAL("draw"),
                                 PYSTR_LITERAL(""), nullptr)));
}

/**
 * Equivalent of Figure.add_subplot. If the subplot already exists then
 * it simply sets that plot number to be active
 * @param subplotLayout Subplot geometry in matplotlib convenience format,
 * e.g 2,1,2 would stack 2 plots on top of each other and set the second
 * to active
 */
void MplFigureCanvas::addSubPlot(int subplotLayout) {
  auto figure = PythonObject(NewRef(
      PyObject_GetAttrString(m_pydata->canvas.get(), PYSTR_LITERAL("figure"))));
  PythonObject(
      NewRef(PyObject_CallMethod(figure.get(), PYSTR_LITERAL("add_subplot"),
                                 PYSTR_LITERAL("(i)"), subplotLayout)));
}

/**
 * Plot lines or markers to the axes
 * @param x A container of X points. Requires support for forward iteration.
 * @param y A container of Y points. Requires support for forward iteration.
 * @param format A format string for the line/markers
 */
template <typename XArrayType, typename YArrayType>
void MplFigureCanvas::plot(const XArrayType &x, const YArrayType &y,
                           const char *format) {
  NDArray1D xnp(x), ynp(y);
  auto axes = m_pydata->gca();
  PythonObject(NewRef(PyObject_CallMethod(axes.get(), PYSTR_LITERAL("plot"),
                                          PYSTR_LITERAL("(OOs)"), xnp.get(),
                                          ynp.get(), format)));
}

//------------------------------------------------------------------------------
// Explicit template instantations
//------------------------------------------------------------------------------
using VectorDouble = std::vector<double>;
template void MplFigureCanvas::plot<VectorDouble, VectorDouble>(
    const VectorDouble &, const VectorDouble &, const char *);
}
