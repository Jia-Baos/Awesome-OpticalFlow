#include <string>
#include <vector>
#include <ctime>
#include "CTensor.h"
#include "CFilter.h"
#include "COpticFlowPart.h"

std::string gFilename1;
std::string gFilename2;

float edgeStrength(CTensor<float> &aImage)
{
  CTensor<float> dx(aImage.xSize(), aImage.ySize(), aImage.zSize());
  CTensor<float> dy(aImage.xSize(), aImage.ySize(), aImage.zSize());
  CDerivative<float> aDerivative(3);
  NFilter::filter(aImage, dx, aDerivative, 1, 1);
  NFilter::filter(aImage, dy, 1, aDerivative, 1);
  CMatrix<float> mag(aImage.xSize(), aImage.ySize(), 0);
  for (int k = 0; k < aImage.zSize(); k++)
    for (int y = 0; y < dx.ySize(); y++)
      for (int x = 0; x < dx.xSize(); x++)
      {
        mag(x, y) += dx(x, y, k) * dx(x, y, k);
        mag(x, y) += dy(x, y, k) * dy(x, y, k);
      }
  float max = 0.0f;
  float avg = 0.0f;
  for (int i = 0; i < mag.size(); i++)
  {
    mag.data()[i] = sqrt(mag.data()[i]);
    avg += mag.data()[i];
    if (mag.data()[i] > max)
      max = mag.data()[i];
  }
  avg /= mag.size();
  float aStrength = (0.3 * avg + 0.7 * max) / 100;
  return aStrength * aStrength;
}

// computeHOG ------------------------------------------------------------------
void computeHOG(CTensor<float> &aBox, CTensor<float> &aHOG)
{
  float inv15pi = 15.0 / (2.0 * NMath::Pi);
  int aXSize = aBox.xSize();
  int aYSize = aBox.ySize();
  int aSize = aXSize * aYSize;
  aHOG.setSize(aXSize, aYSize, 15);
  aHOG = 0;
  CDerivative<float> aDerivative(3);
  CTensor<float> dx(aXSize, aYSize, aBox.zSize());
  CTensor<float> dy(aXSize, aYSize, aBox.zSize());
  NFilter::filter(aBox, dx, aDerivative, 1, 1);
  NFilter::filter(aBox, dy, 1, aDerivative, 1);
  int i2 = 0;
  for (int z = 0; z < aBox.zSize(); z++)
    for (int i = 0; i < aSize; i++, i2++)
    {
      float gradx = dx.data()[i2];
      float grady = dy.data()[i2];
      if (gradx == 0 && grady == 0)
        continue;
      float mag = sqrt(gradx * gradx + grady * grady);
      float ori = atan2(grady, gradx) + NMath::Pi; // now between 0 and 2pi
      int dori = (int)(ori * inv15pi);
      if (dori == 15)
        dori = 0;
      aHOG.data()[i + dori * aSize] += mag;
    }
  CSmooth<float> aGauss(0.8, 1.0);
  NFilter::filter(aHOG, 1, 1, aGauss);
  for (int i = 0; i < aSize; i++)
  {
    float temp1 = aHOG.data()[i];
    float temp2 = aHOG.data()[i + 14 * aSize];
    aHOG.data()[i] = aGauss(-1) * temp2 + (1.0 - aGauss(-1)) * temp1;
    aHOG.data()[i + 14 * aSize] = aGauss(1) * temp1 + (1.0 - aGauss(-1)) * temp2;
  }
  NFilter::boxFilterX(aHOG, 7);
  NFilter::boxFilterY(aHOG, 7);
}

void computeConfidence(CTensor<float> &aImage1, CTensor<float> &aImage2, CMatrix<float> &aConf)
{
  aConf.setSize(aImage1.xSize(), aImage1.ySize());
  int aXSize = aImage1.xSize();
  int aYSize = aImage1.ySize();
  int aSize = aXSize * aYSize;
  // Compute gradient
  CTensor<float> dx(aXSize, aYSize, aImage1.zSize());
  CTensor<float> dy(aXSize, aYSize, aImage1.zSize());
  CDerivative<float> aDerivative(3);
  NFilter::filter(aImage1, dx, aDerivative, 1, 1);
  NFilter::filter(aImage1, dy, 1, aDerivative, 1);
  // Compute second moment matrix
  CMatrix<float> dxx(aXSize, aYSize, 0);
  CMatrix<float> dyy(aXSize, aYSize, 0);
  CMatrix<float> dxy(aXSize, aYSize, 0);
  CMatrix<float> mag(aXSize, aYSize);
  int i2 = 0;
  for (int k = 0; k < aImage1.zSize(); k++)
    for (int i = 0; i < aSize; i++, i2++)
    {
      dxx.data()[i] += dx.data()[i2] * dx.data()[i2];
      dyy.data()[i] += dy.data()[i2] * dy.data()[i2];
      dxy.data()[i] += dx.data()[i2] * dy.data()[i2];
    }
  for (int i = 0; i < aSize; i++)
    mag.data()[i] = sqrt(dxx.data()[i] + dyy.data()[i]);
  // Smooth second moment matrix
  NFilter::recursiveSmoothX(dxx, 4.0);
  NFilter::recursiveSmoothY(dxx, 4.0);
  NFilter::recursiveSmoothX(dyy, 4.0);
  NFilter::recursiveSmoothY(dyy, 4.0);
  NFilter::recursiveSmoothX(dxy, 4.0);
  NFilter::recursiveSmoothY(dxy, 4.0);
  // Compute smallest eigenvalue
  for (int i = 0; i < aSize; i++)
  {
    float a = dxx.data()[i];
    float b = dxy.data()[i];
    float c = dyy.data()[i];
    float temp = 0.5 * (a + c);
    aConf.data()[i] = mag.data()[i] * (temp - sqrt(temp * temp + b * b - a * c));
  }
}

void runFlow(CTensor<float> &aImage1, CTensor<float> &aImage2, CTensor<float> &aResult, float sigma, float alpha, float beta, float gamma)
{
  int aXSize = aImage1.xSize();
  int aYSize = aImage1.ySize();
  int aSize = aImage1.xSize() * aImage1.ySize();
  int aStep = 4;
  int r = 80;
  CTensor<float> aGeoU(aImage1.xSize(), aImage1.ySize(), 1, 0);
  CTensor<float> aGeoV(aImage1.xSize(), aImage1.ySize(), 1, 0);
  CTensor<float> aConfidence(aImage1.xSize(), aImage1.ySize(), 1, 0);
  // Compute HOG, HOG energy, and smallest eigenvalue of structure tensor in first frame
  NFilter::recursiveSmoothX(aImage1, sigma);
  NFilter::recursiveSmoothY(aImage1, sigma);
  NFilter::recursiveSmoothX(aImage2, sigma);
  NFilter::recursiveSmoothY(aImage2, sigma);
  CTensor<float> aHOG1;
  computeHOG(aImage1, aHOG1);
  CMatrix<float> aCornerness;
  computeConfidence(aImage1, aImage2, aCornerness);
  CMatrix<float> aEnergy1(aXSize, aYSize, 0);
  float aAverage = 0.0f;
  int i2 = 0;
  for (int z = 0; z < aHOG1.zSize(); z++)
    for (int i = 0; i < aSize; i++, i2++)
    {
      aEnergy1.data()[i] += aHOG1.data()[i2];
    }
  for (int i = 0; i < aSize; i++)
    aAverage += aCornerness.data()[i];
  aAverage /= aSize;
  // Compute HOG and HOG energy in second frame
  CTensor<float> aHOG2;
  computeHOG(aImage2, aHOG2);
  CMatrix<float> aEnergy2(aXSize, aYSize, 0);
  i2 = 0;
  for (int z = 0; z < aHOG2.zSize(); z++)
    for (int i = 0; i < aSize; i++, i2++)
    {
      aEnergy2.data()[i] += aHOG2.data()[i2];
    }
  // Block matching
  CVector<float> aDesc1(9 * aHOG1.zSize());
  CVector<float> aDesc2(9 * aHOG1.zSize());
  CVector<float> aDesc3(9 * aHOG1.zSize());
  for (int ay = 4; ay < aYSize - 4; ay += aStep)
    for (int ax = 4; ax < aXSize - 4; ax += aStep)
    {
      if (aCornerness(ax, ay) < 0.125 * aAverage)
        continue;
      // Descriptor in first image
      int i = 0;
      for (int y = -4; y <= 4; y += 4)
        for (int x = -4; x <= 4; x += 4)
          for (int k = 0; k < aHOG1.zSize(); k++, i++)
            aDesc1(i) = aHOG1(ax + x, ay + y, k);
      // Compute search radius in second image
      int x1 = ax - r;
      if (x1 < 4)
        x1 = 4;
      int y1 = ay - r;
      if (y1 < 4)
        y1 = 4;
      int x2 = ax + r;
      if (x2 >= aXSize - 4)
        x2 = aXSize - 5;
      int y2 = ay + r;
      if (y2 >= aYSize - 4)
        y2 = aYSize - 5;
      // Find best and second best match in second image
      float bestDist = 1e9f;
      float best2Dist = 1e9f;
      int bestX = 0;
      int bestY = 0;
      int best2X = 0;
      int best2Y = 0;
      for (int by = y1; by <= y2; by++)
        for (int bx = x1; bx <= x2; bx++)
        {
          if (aEnergy2(bx, by) > 2.0f * aEnergy1(ax, ay) || aEnergy1(ax, ay) > 2.0f * aEnergy2(bx, by))
            continue;
          float temp = aImage1(ax, ay, 0) - aImage2(bx, by, 0);
          float dist = temp * temp;
          temp = aImage1(ax, ay, 1) - aImage2(bx, by, 1);
          dist += temp * temp;
          temp = aImage1(ax, ay, 2) - aImage2(bx, by, 2);
          dist += temp * temp;
          if (dist > 300)
            continue;
          dist = 0.0f;
          int i = 0;
          for (int y = -4; y <= 4; y += 4)
            for (int x = -4; x <= 4; x += 4)
              for (int k = 0; k < aHOG1.zSize(); k++, i++)
                aDesc2(i) = aHOG2(bx + x, by + y, k);
          for (int i = 0; i < aDesc1.size(); i++)
          {
            temp = aDesc1(i) - aDesc2(i);
            dist += temp * temp;
          }
          if (dist < bestDist)
          {
            best2Dist = bestDist;
            best2X = bestX;
            best2Y = bestY;
            bestDist = dist;
            bestX = bx;
            bestY = by;
          }
          else if (dist < best2Dist)
          {
            best2Dist = dist;
            best2X = bx;
            best2Y = by;
          }
        }
      // Consistency check -> best match must be best match in backward direction
      x1 = bestX - r;
      if (x1 < 4)
        x1 = 4;
      y1 = bestY - r;
      if (y1 < 4)
        y1 = 4;
      x2 = bestX + r;
      if (x2 >= aXSize - 4)
        x2 = aXSize - 5;
      y2 = bestY + r;
      if (y2 >= aYSize - 4)
        y2 = aYSize - 5;
      int j = 0;
      for (int y = -4; y <= 4; y += 4)
        for (int x = -4; x <= 4; x += 4)
          for (int k = 0; k < aHOG1.zSize(); k++, j++)
            aDesc2(j) = aHOG2(bestX + x, bestY + y, k);
      float bestDistBack = 1e9;
      int bestXBack = 0;
      int bestYBack = 0;
      for (int by = y1; by <= y2; by++)
        for (int bx = x1; bx <= x2; bx++)
        {
          if (aEnergy1(bx, by) > 2.0f * aEnergy2(bestX, bestY) || aEnergy2(bestX, bestY) > 2.0f * aEnergy1(bx, by))
            continue;
          float temp = aImage2(bestX, bestY, 0) - aImage1(bx, by, 0);
          float dist = temp * temp;
          temp = aImage2(bestX, bestY, 1) - aImage1(bx, by, 1);
          dist += temp * temp;
          temp = aImage2(bestX, bestY, 2) - aImage1(bx, by, 2);
          dist += temp * temp;
          if (dist > 300)
            continue;
          dist = 0.0f;
          int i = 0;
          for (int y = -4; y <= 4; y += 4)
            for (int x = -4; x <= 4; x += 4)
              for (int k = 0; k < aHOG1.zSize(); k++, i++)
                aDesc3(i) = aHOG1(bx + x, by + y, k);
          for (int i = 0; i < aDesc1.size(); i++)
          {
            temp = aDesc3(i) - aDesc2(i);
            dist += temp * temp;
          }
          if (dist < bestDistBack)
          {
            bestDistBack = dist;
            bestXBack = bx;
            bestYBack = by;
          }
        }
      // Consistency check passed
      if (fabs(bestXBack - ax) <= 1 && fabs(bestYBack - ay) <= 1)
      {
        aGeoU(ax, ay, 0) = bestX - ax;
        aGeoV(ax, ay, 0) = bestY - ay;
        aConfidence(ax, ay, 0) = (best2Dist - bestDist) / (bestDist + 1e-6);
      }
    }
  // Compute variational flow
  COpticFlow::warpingGeometric(aImage1, aImage2, aResult, aGeoU, aGeoV, aConfidence, 0, 0.0f, alpha, beta, gamma, 0, 0.95, 5, 5, 1.85);
}

int main(int argc, char **args)
{
  // Run on two images
  // if (argc >= 3)
  // {
  //   gFilename1 = args[1];
  //   gFilename2 = args[2];
  //   gFilename1.erase(gFilename1.find_last_of('.'), gFilename1.length());
  //   gFilename2.erase(gFilename2.find_last_of('.'), gFilename2.length());
  //   std::string attach = "LDOF";
  //   clock_t start = clock();
  //   CTensor<float> aImage1;
  //   aImage1.readFromPPM((gFilename1 + ".ppm").c_str());
  //   aImage1.makeColorTensor();
  //   float aEdgeStrength = edgeStrength(aImage1);
  //   CTensor<float> aImage2;
  //   aImage2.readFromPPM((gFilename2 + ".ppm").c_str());
  //   aImage2.makeColorTensor();
  //   float sigma = 0.8f;
  //   if (argc >= 4)
  //     sigma = atof(args[3]);
  //   float alpha = 30;
  //   if (argc >= 5)
  //     alpha = atof(args[4]);
  //   float beta = 300;
  //   if (argc >= 6)
  //     beta = atof(args[5]);
  //   float gamma = 5;
  //   if (argc >= 7)
  //     gamma = atof(args[6]);
  //   CTensor<float> aResult;
  //   runFlow(aImage1, aImage2, aResult, sigma, alpha, beta, gamma);
  //   clock_t finish = clock();
  //   // Write result files
  //   CTensor<float> aFlowImage(aImage1.xSize(), aImage1.ySize(), 3);
  //   COpticFlow::flowToImage(aResult, aFlowImage, 8);
  //   aFlowImage.writeToPPM((gFilename1 + attach + ".ppm").c_str());
  //   COpticFlow::writeMiddlebury(aResult, (gFilename1 + attach + ".flo").c_str());
  //   std::cout << "Computation time: " << (double(finish) - double(start)) / CLOCKS_PER_SEC << std::endl;
  // }
  // else
  // {
  //   std::cout << "Usage: ldof image1.ppm image2.ppm" << std::endl;
  // }

  // Jia-Baos
  // 2024-01-24 12:18
  gFilename1 = "D:\\CmakeProject\\OpticalFlow\\ldof\\MPI-Sintel\\frame_0029.ppm";
  gFilename2 = "D:\\CmakeProject\\OpticalFlow\\ldof\\MPI-Sintel\\frame_0030.ppm";
  gFilename1.erase(gFilename1.find_last_of('.'), gFilename1.length());
  gFilename2.erase(gFilename2.find_last_of('.'), gFilename2.length());
  std::string attach = "LDOF";
  clock_t start = clock();
  CTensor<float> aImage1;
  aImage1.readFromPPM((gFilename1 + ".ppm").c_str());
  aImage1.makeColorTensor();
  float aEdgeStrength = edgeStrength(aImage1);
  CTensor<float> aImage2;
  aImage2.readFromPPM((gFilename2 + ".ppm").c_str());
  aImage2.makeColorTensor();
  float sigma = 0.8f;

  float alpha = 30;
  float beta = 300;
  float gamma = 5;

  CTensor<float> aResult;
  runFlow(aImage1, aImage2, aResult, sigma, alpha, beta, gamma);
  clock_t finish = clock();

  // Write result files
  CTensor<float> aFlowImage(aImage1.xSize(), aImage1.ySize(), 3);
  COpticFlow::flowToImage(aResult, aFlowImage, 8);
  aFlowImage.writeToPPM((gFilename1 + attach + ".ppm").c_str());
  COpticFlow::writeMiddlebury(aResult, (gFilename1 + attach + ".flo").c_str());
  std::cout << "Computation time: " << (double(finish) - double(start)) / CLOCKS_PER_SEC << std::endl;

  return 0;
}
