> [!CAUTION] 
> DEPRECATED! Use https://github.com/dnjulek/vapoursynth-zip
## MTCombMask

This filter produces a mask showing areas that are combed. MTCombMask uses 3-point sampling to determine combing in a pixel.
The thresholds work as following: after calculating the combing value, if one is below thY1, the pixel is set to 0, if above thY2, it is set to 255, and if in between, it is set to the combing value divided by 256.

Ported from [AviSynth plugin](http://avisynth.nl/index.php/MTCombMask)

### Usage
```python
mtcm.MTCombMask(vnode clip[, int thY1=30, int thY2=30])
```
### Parameters:

- clip\
    A clip to process. It must be in YUV/GRAY 8-bit.
    
- thY1\
    Pixels below thY1 are set to 0.\
    Must be between 0 and 255 and must be <= thY2.\
    Default: 30.
    
- thY2\
    Pixels above thY2 are set to 255.\
    Must be between 0 and 255 and must be >= thY1.\
    Default: 30.
