#include "random_gen.hpp"
#include <cstddef>

using namespace gd;

RandomGen::RandomGen()
  : m_generator(m_rdGen()),
    m_shuffleGenerator(m_rdShuffle())
{}

size_t RandomGen::getRandomUint(size_t upper)
{
  std::uniform_int_distribution<size_t> distribution(0, upper - 1);
  while(!m_lock.try_lock()) {}

  size_t randomVal = distribution(m_generator);

  m_lock.unlock();
  return randomVal;
}
