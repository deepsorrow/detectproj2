#ifndef MASK_H
#define MASK_H
#include <vector>
class Mask{
public:
    Mask ();
    auto set_detected(const int _timeout = 60) -> void;

    bool detected = false;
    int timeout = 0;
private:


};

#endif // MASK_H
