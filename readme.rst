===================================
GenericFilters - VapourSynth plugin
===================================

This plugin is a set of common image-processing filters.

This plugin modifies all pixel values with various algorithms.

All functions support 8/9/10/16bit planar formats.

Currently, GenericFilters has thirteen functions as follows.

Minimum:
--------
Replaces the pixel by the local(3x3) minimum. (aka. Erosion)::

    generic.Minimum(clip clip[, int[] planes, int threshold, bint[] coordinates])

planes - Choose which planes to process. default will process all planes. Allowed values are 0, 1, and 2.::

    0 = Y (luma) or R (red)
    1 = U (Cb)   or G (green)
    2 = V (Cr)   or B (blue)

    examples
    planes=[0]    = processes the Y or R plane only.
    planes=[1,2]  = processes the U V or G B planes only.

threshold - Allows to limit the maximum change. default is 65535.

coordinates - List which specifies the pixel to refer to. This list must have eight integers which consisted of 0 or 1. Default is [1, 1, 1, 1, 1, 1, 1, 1].::

    [top-left, top, top-right, left, right, bottom-left, bottom, bottom-right]

    [0, 0, 0, 1, 1, 0, 0, 0] - referring to only left and right pixels.
    [0, 1, 0, 1, 1, 0, 1, 0] - The pixels which exist in the direction of slanting are ignored.

Maximum:
--------
Replaces the pixel by the local(3x3) maximum. (aka. Dilation)::

    generic.Maximum(clip clip[, int[] planes, int threshold, bint[] coordinates])

planes - same as Minimum.

threshold - same as Minimum.

coordinates - same as Minimum.

Median:
-------
Replaces the pixel by the local(3x3) median.::

    generic.Median(clip clip[, int[] planes])

planes - same as Minimum.

Convolution:
------------
Spatial convolution (horizontal/vertical 3, horizontal/vertical 5, 3x3 or 5x5) filter.::

    generic.Convolution(clip clip[, int[] matrix, float bias, float divisor, int[] planes, data mode, bint saturate])

matrix - can be a matrix with 3, 5, 9 or 25 integer numbers. default is [0, 0, 0, 0, 1, 0, 0, 0, 0].

bias - additive bias to adjust the total output intensity. default is 0.0.

divisor - divides the output of the convolution (calculated before adding bias). 0.0 means sum of the elements of the matrix or 1.0(when the sum is zero). default is 0.0.

planes - same as Minimum.

mode - If this is set as 'v' when the number of elements of the matrix is 3 or 5, processing will be performed vertically.

saturate - If this is set as False, output values will turn into absolute values. Default is True.

ConvolutionHV:
--------------
It performs vertical 5 convolution first and then performs horizontal 5 convolution (faster than 5x5 convolution).::

    generic.ConvolutionHV(clip clip[, int[] horizontal, int[] vertical, float bias, float divisor_h, float divisor_v, int[] planes, bint saturate])

horizontal - horizontal matrix. the length must be five. default is [0, 0, 1, 0, 0].

vertical - vertical matrix. the length must be five. default is [0, 0, 1, 0, 0].

bias - same as Convolution.

divisor_h - horizontal divisor.

divisor_v - vertical divisor.

planes - same as Minimum.

saturate - same as Convolution.

Blur:
-----
Blur image with 3x3 kernel.

    generic.Blur(clip clip[, float ration_h, float ratio_v, int[] planes])

ratio_h: The rate of the pixel of the left and right blended to a central pixel(0.0 to 1.0). Default is 0.5.

ratio_v: The rate of the pixel of the top and bottom blended to a central pixel(0.0 to 1.0). Default is the value of ratio_h.::

    examples

    generic.Blur(clip, ratio_h=0.5, ratio_v=0.5)
    horizontal kernel = [0.5, 1.0, 0.5], vertical kernel = [0.5, 1.0, 0.5]
    This is same as generic.Convolution(clip, [1, 2, 1, 2, 4, 2, 1, 2, 1])

    generic.Blur(clip, ratio_h=1.0, ratio_v=0)
    horizontal kernel = [1.0, 1.0, 1.0], vertical kernel = [0.0, 1.0, 0.0]
    This is same as generic.Convolution(clip, [0, 0, 0, 1, 1, 1, 0, 0, 0])
    

planes - same as Minimum.

Sobel:
------
Detect edge with Sobel operator.::

    generic.Sobel(clip clip[, int min, int max, int[] planes, int rshift])

min: if output pixel value is lower than this, it will be zero. default is 0.

max: if output pixel value is same or higher than this, it will be maximum value of the format. default is 65535.

rshift: shift the output values to right by this count before clamp. Default is 0.

Prewitt:
--------
Detect edge with template matching using Prewitt operator(aka. Prewitt compass edge detection).::

    generic.Prewitt(clip clip[, int min, int max, int[] planes, int rshift])

min: same as Sobel.

max: same as Sobel.

rshift: same as Sobel.

Inflate:
--------
Local(3x3) average by taking into account only values higher than the pixel.::

    generic.Inflate(clip clip[, int threshold, int[] planes, int threshold])

planes - same as Minimum.

threshold - same as Minimum.

Deflate:
--------
Local(3x3) average by taking into account only values lower than the pixel.::

    generic.Deflate(clip clip[, int thresh, int[] planes, int threshold])

planes - same as Minimum.

threshold - same as Minimum.

Invert:
-------
Invert the pixel value.::

    generic.Invert(clip clip[, int[] planes])

planes - same as Minimum.

Limiter:
---------
Clamp the pixel value.::

    generic.Limiter(clip clip[, int min, int max, int[] planes])

min - minimum threshold of pixel value. default is 0.

max - maximum threshold of the pixel value. default is the max value of input format.

planes - same as Minimum.

Levels:
-------
Adjusts brightness, contrast, and gamma.::

    generic.Levels(clip clip[, int min_in, int max_in, float gamma, int min_out, int max_out])

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

    generic.Binarize(clip clip[, int threshold, inv v0, int v1, int[] planes])

threshold - threshold. default is half of the maximum of input format(128, 256, 512 or 32768).

v0 - If the value of pixel is lower than threshold, output will be this. Default is 0.

v1 - If the value of pixel is same or higher than threshold, output will be this. Default is the maximum value of input(255, 511, 1023 or 65535).

planes - same as Minimum.

Examples:
---------
    >>> import vapoursynth as vs
    >>> core = vs.Core()
    >>> core.std.LoadPlugin('/path/to/genericfilters.dll')
    >>> std = core.std
    >>> generic = core.generic
    >>> clip = something

    - Sharpen(3x3) only Y(or R) plane:
    >>> matrix = [-1, -2, -1, -2, 16, -2, -1, -2, -1]
    >>> div = sum(matrix)
    >>> blurred = generic.Convolution(clip, matrix, divisor=div, planes=0)

    - Displacement UV(or GB) planes by quarter sample up:
    >>> matrix = [1,
                  3,
                  0]
    >>> clip = generic.Convolution(clip, matrix, planes=[1, 2], mode = 'v')

    - Bob:
    >>> height = clip.height
    >>> clip = std.SeparateFields(clip, tff=True)
    >>> top = generic.Convolution(clip[::2], [0, 3, 1], mode='v')
    >>> bottom = generic.Convolution(clip[1::2], [1, 3, 0], mode='v')
    >>> clip = core.resize.Bicubic(std.Interleave([top, bottom]), height=height)

    - Unsharp Masking:
    >>> blurred = generic.Convolution(clip, [2,  4,  5,  4, 2,
                                             4,  9, 12,  9, 4,
                                             5, 12, 15, 12, 5,
                                             4,  9, 12,  9, 4,
                                             2,  4,  5,  4, 2]) # gaussian blur
    >>> clip = std.Expr([clip, blurred], "x x + y -")

    - Convert TV levels to PC levels(8bit YUV):
    >>> y = generic.Levels(clip, 16, 236, 1.0, 0, 255, 0)
    >>> uv = generic.Levels(clip, 16, 240, 1.0, 0, 255, [1, 2])
    >>> clip = std.ShufflePlanes([y, uv], [0, 1], vs.YUV)

Note:
-----
    If input clip has some frames which sample types are float, those will not be processed.

    The output values of Convolution(HV) are clamped to [0..255](8bit format) or [0..65535](9/10/16bit format).
    In the case format is 9/10bit, they may exceed the maximum of the format.
    This is not a bug but a specification of this plugin.

How to compile:
---------------
    on unix like system(include mingw), type as follows::

    $ git clone git://github.com/chikuzen/GenericFilters.git
    $ cd ./GenericFilters/src
    $ ./configure
    $ make install

    if you want to use msvc++, then

    - rename all *.c to *.cpp
    - create vcxproj yourself

Source code:
------------
https://github.com/chikuzen/GenericFilters


Author: Oka Motofumi (chikuzen.mo at gmail dot com)