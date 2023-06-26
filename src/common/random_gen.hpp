#ifndef __MAJORMINER_RANDOM_GEN_HPP_
#define __MAJORMINER_RANDOM_GEN_HPP_

#include <cassert>
#include <cstddef>
#include <random>
#include <mutex>
#include <gd_types.hpp>

namespace gd
{

  class RandomGen
  {
    public:
      RandomGen();
      size_t getRandomUint(size_t upper); //exclusive upper, ie. [0, upper)

      template<typename T>
      const T& getRandom(const Vector<T>& vec)
      {
        assert(vec.size() != 0 && "Vector should not be empty when sampling from it!");
        return vec.at(getRandomUint(vec.size()));
      }


    private:
      std::random_device m_rdGen;
      std::default_random_engine m_generator;
  };

  template<typename T = float, typename = std::enable_if_t<std::is_floating_point<T>::value>>
  struct ProbabilisticDecision
  {
    public:
      ProbabilisticDecision() : m_gen(m_rd()), m_dist(static_cast<T>(0.0), static_cast<T>(1.0)) {}
      T operator()() { return m_dist(m_gen); }
      bool operator()(T value) { return m_dist(m_gen) < value; }

    private:
      std::random_device m_rd;
      std::mt19937 m_gen;
      std::uniform_real_distribution<T> m_dist;
  };
}


#endif