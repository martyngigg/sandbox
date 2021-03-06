#ifndef CONTAINERITEM_H_
#define CONTAINERITEM_H_
#include <boost/iterator/iterator_facade.hpp>
#include <cstdint>

class Container;
class ContainerIterator;

class ContainerItem {
public:
  void advance(int64_t delta) {
    m_index = delta < 0 ? std::max(static_cast<uint64_t>(0),
                                   static_cast<uint64_t>(m_index) + delta)
                        : std::min(m_container->size(),
                                   m_index + static_cast<size_t>(delta));
  }

  void incrementIndex() {
    if (m_index < m_container->size()) {
      ++m_index;
    }
  }

  void decrementIndex() {
    if (m_index > 0) {
      --m_index;
    }
  }

  size_t index() const { return m_index; }

private:
  friend class ContainerIterator;

  /// Private constructor, can only be created by HistogramIterator
  ContainerItem(const Container &container, const size_t index)
      : m_container(&container), m_index(index) {}

  ContainerItem(const ContainerItem &other) = default;
  ContainerItem &operator=(const ContainerItem &rhs) = default;
  ContainerItem(ContainerItem &&other) = default;
  ContainerItem &operator=(ContainerItem &&rhs) = default;

  // non-owning pointer. A reference makes the class unable to define
  // an assignment operator that we need
  const Container *m_container;
  std::size_t m_index;
};

#endif // CONTAINERITEM_H_
