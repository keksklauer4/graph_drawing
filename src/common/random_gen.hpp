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
      void shuffle(T* data, size_t size)
      {
        if (size == 0) return;
        while(!m_shuffleLock.try_lock()) {}

        std::shuffle(data, data + size, m_shuffleGenerator);

        m_shuffleLock.unlock();
      }

      template<typename T>
      const T& getRandom(const Vector<T>& vec)
      {
        assert(vec.size() != 0 && "Vector should not be empty when sampling from it!");
        return vec.at(getRandomUint(vec.size()));
      }


    private:
      std::mutex m_lock;
      std::mutex m_shuffleLock;
      std::random_device m_rdGen;
      std::random_device m_rdShuffle;
      std::default_random_engine m_generator;
      std::default_random_engine m_shuffleGenerator;
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