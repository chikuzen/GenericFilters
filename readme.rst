===============================
Neighbors - VapourSynth plugin
===============================

This plugin modifies all pixel values with reference to the neighbors of them.

Currentry, Neighbors has six functions as follows.

All functions are supported 8/9/10/16bit planar not constant formats.

Minimum:
--------
Replaces the pixel by the local(3x3) minimum.::

    neighbors.Minimum(clip clip[, int[] planes])

planes - planes which processes. default will process all planes.

Maximum:
--------
Replaces the pixel by the local(3x3) maximum.::

    neighbors.Maximum(clip clip[, int[] planes])

planes - same as Minimum.

Median:
-------
Replaces the pixel by the local(3x3) median.::

    neighbors.Median(clip clip[, int[] planes])

planes - same as Minimum.

Convolution:
------------
General spatial convolution (horizontal/vertiac 3, horizontal/vertical 5, 3x3 or 5x5) filter.::

    neighbors.Convolution(clip clip[, int[] matrix, float bias, float divisor, int[] planes, data mode])

matrix - can be a matrix with 3, 5, 9 or 25 integer numbers. default is [0, 0, 0, 0, 1, 0, 0, 0, 0].

bias - additive bias to adjust the total output intensity. default is 0.0.

divisor - divides the output of the convolution (calculated before adding bias). 0.0 means sum of the elements of the matrix or 1.0(when the sum is zero). default is 0.0.

planes - same as Minimum.

mode - If this is set as 'v' when the number of elements of the matrix is 3 or 5, processing will be performed vertically.

ConvolutionHV:
--------------
It performs vertical 5 convolution first and then performs horizontal 5 convolution (faster than 5x5 convolution).::

    neighbors.ConvolutionHV(clip clip[, int[] horizontal, int[] vertical, float bias, float divisor_h, float divisor_v, int[] planes])

horizontal - horizontal matrix. the length must be five. default is [0, 0, 1, 0, 0].

vertical - vertical matrix. the length must be five. default is [0, 0, 1, 0, 0].

bias - same as Convolution.

divisor_h - horizontal diisor.

divisor_v - vertical diisor.

planes - same as Minimum.

Invert:
-------
Invert the pixel value.::

    neignbors.Invert(clip clip[, int[] planes])

planes - same as Minimum.

Limitter:
---------
Clamp the pixel value.::

    neighbors.Limitter(clip clip[, int min, int max, int[] planes])

min - minimum threshold of pixel value. default is 0.

max - maximum threshold of the pixel value. default is 65535.

planes - same as Minimum.

Examples:
---------
    >>> import vapoursynth as vs
    >>> core = vs.Core()
    >>> core.std.LoadPlugin('/path/to/neighbors.dll')
    >>> clip = something

    - blur(5x5) only Y(or R) plane:
    >>> matrix = [10, 10, 16, 10, 10]
    >>> blured = core.neighbors.ConvolutionHV(clip, matrix, matrix, planes=0)

    - Displacement UV(or GB) planes by quarter sample up:
    >>> matrix = [1,
                  3,
                  0]
    >>> clip = core.convo2d.Convolution(clip, matrix, planes=[1, 2], mode = 'v')

    - Edge detection with Sobel operator:
    >>> import math
    >>> def binalyze(val, thresh):
    ...     return 255 if val > thresh else 0
    ...
    >>> def get_lut(thresh):
    ...     lut = []
    ...     for y in range(256):
    ...         for x in range(256):
    ...             lut.append(binalyze(math.sqrt(x * x + y * y), thresh))
    ...     return lut
    ...
    >>> clip = core.resize.Point(clip, format=vs.GRAY8)
    >>> edge_h = core.neighbors.Convolution(clip, [1, 2, 1, 0, 0, 0, -1, -2, -1], divisor=8)
    >>> edge_v = core.neighbors.Convolution(clip, [1, 0, -1, 2, 0, -2, 1, 0, -1], divisor=8)
    >>> clip = core.std.Lut2([edge_h, edge_v], get_lut(16), 0)
    >>> clip = core.neighbors.Invert(clip) # invert edge mask

Note:
-----
    If input clip has some frames which sample types are float, those will not be processed.

How to compile:
---------------
    on unix like system(include mingw), type as follows::

    $ git clone git://github.com/chikuzen/neighbors.git
    $ cd ./neighbors
    $ ./configure
    $ make install

    if you want to use msvc++, then

    - rename all *.c to *.cpp
    - create vcxproj yourself

Source code:
------------
https://github.com/chikuzen/neigbors


Author: Oka Motofumi (chikuzen.mo at gmail dot com)
