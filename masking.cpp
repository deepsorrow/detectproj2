#include "masking.h"

Mask::Mask()
{

}

auto Mask::set_detected(const int _timeout) -> void{
    detected = true;
    timeout = _timeout;
}
