#ifndef __FX_H
#define __FX_H

class Layer;

class Fx {
  public:
    virtual int render(Layer *l, int _w, int _h, int *input, int tw, int th, int twpitch)=0;
};


#endif