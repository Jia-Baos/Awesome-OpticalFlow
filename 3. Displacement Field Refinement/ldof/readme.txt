Source code for running Large Displacement Optical Flow as
presented in IEEE PAMI 2010

Copyright (c) 2010 Thomas Brox

------------------------------
Terms of use
------------------------------

This program is provided for research purposes only and must not be
distributed to any third party without explicit permission by the 
copyright holder. Any direct or indirect commercial use is prohibited. 
If you are interested in a commercial use, please contact the copyright 
holder. 

If you used this program in your research work, you must cite the 
following publication:

Thomas Brox, Jitendra Malik. 
Large Displacement Optical Flow: Descriptor Matching in Variational
Motion Estimation
IEEE Transactions on Pattern Analysis and Machine Intelligence, 33(3): 500-513, 2011. 

This program is distributed WITHOUT ANY WARRANTY. 

------------------------------
Usage
------------------------------

This will run LDOF with the standard parameter setting sigma=0.8,
alpha=30, beta=300, gamma=5:

./ldof tennis492.ppm tennis493.ppm

This parameter setting works well on a variety of typical image 
sequences. If your setting is more special, you should try to adapt 
the parameters to your special needs:

./ldof image1.ppm image2.ppm sigma alpha beta gamma

Image1 and image2 must be images in the binary PPM format (P6). 
Please convert any other image formats into PPM using convert or
mogrify. 

------------------------------
Middlebury sequences
------------------------------

This package also provides binaries to reproduce the results reported 
for the Middlebury sequences from http://vision.middlebury.edu/flow/

./ldofmiddlebury image1.ppm image2.ppm

Again image1 and image2 must be images in PPM format (P6). 

------------------------------
Bugs
------------------------------

Please report bugs to Thomas Brox

