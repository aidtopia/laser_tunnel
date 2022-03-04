#ifndef PATTERNBUFFER_H
#define PATTERNBUFFER_H

// The PatternBuffer is a bitmask for 256 "pixels" mapped
// around the cone of the laser tunnel.

class PatternBuffer {
  public:
    PatternBuffer() : m_buffer() {}

    void clear() { for (auto &b : m_buffer) b = 0; }
    constexpr size_t size() { return 8*sizeof(m_buffer); }
    bool operator[](int i) const {
      const auto index = static_cast<uint8_t>(i);
      return (b(index) & mask(index)) != 0;
    }
    void setPixel(uint8_t i)    { b(i) |=  mask(i); }
    void clearPixel(uint8_t i)  { b(i) &= ~mask(i); }
    void togglePixel(uint8_t i) { b(i) ^=  mask(i); }

    void setTestPattern() {
      for (auto &b : m_buffer) b = 0b11110000;
    }
    
  private:
    uint8_t b(uint8_t i) const { return m_buffer[i >> 3]; }
    uint8_t &b(uint8_t i)      { return m_buffer[i >> 3]; }
    static uint8_t mask(uint8_t i) { return 0b10000000 >> (i & 0b0111); }
  
    uint8_t m_buffer[32];
};

#endif
