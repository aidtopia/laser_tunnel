#ifndef PATTERNBUFFER_H
#define PATTERNBUFFER_H

// The PatternBuffer is a bitmask for 256 "pixels" mapped
// around the cone of the laser tunnel.

class PatternBuffer {
  public:
    PatternBuffer() : m_buffer(), m_scan_index(0), m_scan_start(0) {}

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

    bool scan() { return (*this)[m_scan_index++]; }
    void resync() { m_scan_index = m_scan_start; }
    void rotate(int amount = 1) {
      noInterrupts();
      m_scan_start += static_cast<uint8_t>(amount);
      interrupts();
    }
    void setRotation(int rot) {
      noInterrupts();
      m_scan_start = static_cast<uint8_t>(rot);
      interrupts();
    }
    
  private:
    uint8_t b(uint8_t i) const { return m_buffer[i >> 3]; }
    uint8_t &b(uint8_t i)      { return m_buffer[i >> 3]; }
    static uint8_t mask(uint8_t i) { return 0b10000000 >> (i & 0b0111); }
  
    uint8_t m_buffer[32];
    uint8_t m_scan_index;
    uint8_t m_scan_start;
};

#endif
