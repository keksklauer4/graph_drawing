#ifndef __GD_SHORT_TERM_SET_HPP__
#define __GD_SHORT_TERM_SET_HPP__

#include <cstddef>
#include <gd_types.hpp>

namespace gd
{
  template<typename T, size_t size = 32>
  class ShortTermSet
  {
    public:
      ShortTermSet(){ static_assert(size > 0, "Set needs to have size > 0!"); }

      void insert(T element)
      {
        if (m_elements.contains(element)) return;
        if (m_elements.size() == size) m_elements.erase(m_ringbuffer[m_elementIdx]);
        m_elements.insert(element);
        m_ringbuffer[m_elementIdx] = element;
        m_elementIdx = (m_elementIdx + 1) % size;
      }

      bool contains(T element) const
      { return m_elements.contains(element); }

      void clear() { m_elements.clear(); }

    private:
      T m_ringbuffer[size];
      Set<T> m_elements;
      size_t m_elementIdx; // points to next element idx
  };

  typedef ShortTermSet<vertex_t, 16> ShortTermVertexSet;
}


#endif
