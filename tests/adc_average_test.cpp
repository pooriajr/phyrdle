#include "../adc_average.h"
#include <cassert>
int main() {
  AdcAverage filter;
  for (unsigned i=0;i<8;++i) assert(filter.update(100,i*5)==100);
  assert(filter.update(900,40)==200);
  for(unsigned i=1;i<8;++i) filter.update(900,40+i*5);
  assert(filter.update(900,80)==900);
  assert(filter.update(0,200)==0); // no stale averaging after a pause
  filter.reset();
  for(unsigned i=0;i<16;++i) filter.update(i%2?990:978,205+i*5);
  assert(filter.update(978,285)==984);
  AdcAverage other;
  assert(other.update(1023,0)==1023);
}
