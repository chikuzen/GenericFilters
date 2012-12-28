==========================
Tweak - VapourSynth plugin
==========================

This plugin modifies all pixel values with various algorithms.

All functions support 8/9/10/16bit planar formats.

Currently, Tweak has eleven functions as follows.

Minimum:
--------
Replaces the pixel by the local(3x3) minimum.::

    tweak.Minimum(clip clip[, int[] planes])

planes - Choose which planes to process. default will process all planes.
          Allowed values are 0, 1, and 2.
          0 = Y (luma) or R (red)
          1 = U (Cb)   or G (green)
          2 = V (Cr)   or B (blue)
          examples: planes=[0]    = processes the Y or R plane only.
                    planes=[1,2]  = processes the U V or G B planes only.

Maximum:
--------
Replaces the pixel by the local(3x3) maximum.::

    tweak.Maximum(clip clip[, int[] planes])

planes - same as Minimum.

Median:
-------
Replaces the pixel by the local(3x3) median.::

    tweak.Median(clip clip[, int[] planes])

planes - same as Minimum.

Convolution:
------------
General spatial convolution (horizontal/vertical 3, horizontal/vertical 5, 3x3 or 5x5) filter.::

    tweak.Convolution(clip clip[, int[] matrix, float bias, float divisor, int[] planes, data mode])

matrix - can be a matrix with 3, 5, 9 or 25 integer numbers. default is [0, 0, 0, 0, 1, 0, 0, 0, 0].

bias - additive bias to adjust the total output intensity. default is 0.0.

divisor - divides the output of the convolution (calculated before adding bias). 0.0 means sum of the elements of the matrix or 1.0(when the sum is zero). default is 0.0.

planes - same as Minimum.

mode - If this is set as 'v' when the number of elements of the matrix is 3 or 5, processing will be performed vertically.

ConvolutionHV:
--------------
It performs vertical 5 convolution first and then performs horizontal 5 convolution (faster than 5x5 convolution).::

    tweak.ConvolutionHV(clip clip[, int[] horizontal, int[] vertical, float bias, float divisor_h, float divisor_v, int[] planes])

horizontal - horizontal matrix. the length must be five. default is [0, 0, 1, 0, 0].

vertical - vertical matrix. the length must be five. default is [0, 0, 1, 0, 0].

bias - same as Convolution.

divisor_h - horizontal diisor.

divisor_v - vertical diisor.

planes - same as Minimum.

Inflate:
--------
Local(3x3) average by taking into account only values higher than the pixel.::

    tweak.Inflate(clip clip[, int thresh, int[] planes])

thresh - The neighbouring pixels higher than this value are ignored.

Deflate:
--------
Local(3x3) average by taking into account only values lower than the pixel.::

    tweak.Deflate(clip clip[, int thresh, int[] planes])

thresh - The neighbouring pixels lower than this value are ignored.

Invert:
-------
Invert the pixel value.::

    tweak.Invert(clip clip[, int[] planes])

planes - same as Minimum.

Limitter:
---------
Clamp the pixel value.::

    tweak.Limitter(clip clip[, int min, int max, int[] planes])

min - minimum threshold of pixel value. default is 0.

max - maximum threshold of the pixel value. default is the max value of input format.

planes - same as Minimum.

Levels:
-------
Adjusts brightness, contrast, and gamma.::

    tweak.Levels(clip clip[, int min_in, int max_in, float gamma, int min_out, int max_out])

min_in - determine minimum input pixel value. default is 0.

max_in - determine maximum input pixel value. default is 255 * (2 ^ (8 - bits_per_pixel)).

gamma - gamma. default is 1.0.

min_out - determine minimum output pixel value. default is 0.

max_out - determine maximum output pixel value. default is 255 * (2 ^ (8 - bits_per_pixel)).

planes - same as Minimum.

The conversion function is::

    output = ((input - min_in) / (max_in - min_in)) ^ (1.0 / gamma) * (max_out - min_out) + min_out

Binarize:
---------
Binarize the pixel value.::

    tweak.Binarize(clip clip[, int thresh, inv v0, int v1, int[] planes])

thresh - threshold. default is half of the maximum of input format(128, 256, 512 or 32768).

v0 - If the value of pixel is lower than thresh, output will be this. Default is 0.

v1 - If the value of pixel is same or higher than thresh, output will be this. Default is the maximum value of input(255, 511, 1023 or 65535).

planes - same as Minimum.

Examples:
---------
    >>> import vapoursynth as vs
    >>> core = vs.Core()
    >>> core.std.LoadPlugin('/path/to/tweak.dll')
    >>> clip = something

    - blur(5x5) only Y(or R) plane:
    >>> matrix = [10, 10, 16, 10, 10]
    >>> blured = core.tweak.ConvolutionHV(clip, matrix, matrix, planes=0)

    - Displacement UV(or GB) planes by quarter sample up:
    >>> matrix = [1,
                  3,
                  0]
    >>> clip = core.tweak.Convolution(clip, matrix, planes=[1, 2], mode = 'v')

    - Edge detection with Sobel operator:
    >>> import math
    >>> def get_lut(thresh):
    ...     lut = []
    ...     for y in range(256):
    ...         for x in range(256):
    ...             lut.append(binalyze(math.sqrt(x * x + y * y), thresh))
    ...     return lut
    ...
    >>> clip = core.resize.Point(clip, format=vs.GRAY8)
    >>> edge_h = core.tweak.Convolution(clip, [1, 2, 1, 0, 0, 0, -1, -2, -1], divisor=8)
    >>> edge_v = core.tweak.Convolution(clip, [1, 0, -1, 2, 0, -2, 1, 0, -1], divisor=8)
    >>> clip = core.std.Lut2([edge_h, edge_v], get_lut(16), 0)
    >>> clip = core.tweak.Binarize(clip, 10) # binarize edge mask
    >>> clip = core.tweak.Invert(clip) # invert edge mask

    - Convert TV levels to PC levels:
    >>> y = core.tweak.levels(clip, 16, 236, 1.0, 0, 255, 0)
    >>> uv = core.tweak.levels(clip, 16, 240, 1.0, 0, 255, [1, 2])
    >>> clip = core.std.ShufflePlanes([y, uv], [0, 1, 2], vs.YUV)

Note:
-----
    If input clip has some frames which sample types are float, those will not be processed.

How to compile:
---------------
    on unix like system(include mingw), type as follows::

    $ git clone git://github.com/chikuzen/tweak.git
    $ cd ./tweak/src
    $ ./configure
    $ make install

    if you want to use msvc++, then

    - rename all *.c to *.cpp
    - create vcxproj yourself

Source code:
------------
https://github.com/chikuzen/tweak


Author: Oka Motofumi (chikuzen.mo at gmail dot com)