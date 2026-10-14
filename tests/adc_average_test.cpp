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
  AdcAverage insertion;
  for(unsigned i=0;i<8;++i) assert(insertion.update(5,i*5,20)==5);
  assert(insertion.update(980,40,20)==980); // immediate insertion, no ramp
  assert(insertion.update(984,45,20)==982); // averaging resumes normally
  assert(insertion.update(0,50,20)==0); // immediate removal
  assert(insertion.update(410,55,20)==410); // another tile
  assert(insertion.update(20,60,20)==20); // inclusive empty threshold
  assert(insertion.update(21,65,20)==21);
  AdcAverage other;
  assert(other.update(1023,0)==1023);
}
