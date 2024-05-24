#include "COpticFlowPart.h"

// Jia-Baos
// 2024-01-24 12:18
#include "./CFilter.h"

// Brox et al. CVPR 2009: geometric constraints from keypoint matching ---------

// diffusivity
void COpticFlow::diffusivity(CTensor<float> &aFlow, CTensor<float> &aFlowIncrement, CMatrix<float> &aResultX, CMatrix<float> &aResultY, CMatrix<float> *aEdgeMap, float aEdgeWeight)
{
  CMatrix<float> aGrad(aFlow.xSize(), aFlow.ySize());
  CMatrix<float> aChannel(aFlow.xSize(), aFlow.ySize());
  CDerivative<float> aDerivative(3);
  aResultX = 0;
  aResultY = 0;
  int aSize = aFlow.xSize() * aFlow.ySize();
  // Compute gradient magnitude of flow+increment
  // ---- U ----
  for (int i = 0; i < aSize; i++)
    aChannel.data()[i] = aFlow.data()[i] + aFlowIncrement.data()[i];
  // Gradients in x direction
  NFilter::filter(aChannel, aGrad, 1, aDerivative);
  int i = 0;
  for (int y = 0; y < aChannel.ySize(); y++, i++)
    for (int x = 0; x < aChannel.xSize() - 1; x++, i++)
    {
      float aGradX = aChannel.data()[i + 1] - aChannel.data()[i];
      float aGradY = 0.5 * (aGrad.data()[i] + aGrad.data()[i + 1]);
      aResultX.data()[i] = aGradX * aGradX + aGradY * aGradY;
    }
  // Gradients in y direction
  NFilter::filter(aChannel, aGrad, aDerivative, 1);
  for (i = 0; i < aSize - aChannel.xSize(); i++)
  {
    float aGradX = 0.5 * (aGrad.data()[i] + aGrad.data()[i + aChannel.xSize()]);
    float aGradY = aChannel.data()[i + aChannel.xSize()] - aChannel.data()[i];
    aResultY.data()[i] = aGradX * aGradX + aGradY * aGradY;
  }
  // ---- V ----
  float *pFlow = aFlow.data() + aSize;
  float *pFlowIncrement = aFlowIncrement.data() + aSize;
  for (int i = 0; i < aSize; i++)
    aChannel.data()[i] = *(pFlow + i) + *(pFlowIncrement + i);
  // Gradients in x direction
  NFilter::filter(aChannel, aGrad, 1, aDerivative);
  i = 0;
  for (int y = 0; y < aChannel.ySize(); y++, i++)
    for (int x = 0; x < aChannel.xSize() - 1; x++, i++)
    {
      float aGradX = aChannel.data()[i + 1] - aChannel.data()[i];
      float aGradY = 0.5 * (aGrad.data()[i] + aGrad.data()[i + 1]);
      aResultX.data()[i] += aGradX * aGradX + aGradY * aGradY;
    }
  // Gradients in y direction
  NFilter::filter(aChannel, aGrad, aDerivative, 1);
  for (i = 0; i < aSize - aChannel.xSize(); i++)
  {
    float aGradX = 0.5 * (aGrad.data()[i] + aGrad.data()[i + aChannel.xSize()]);
    float aGradY = aChannel.data()[i + aChannel.xSize()] - aChannel.data()[i];
    aResultY.data()[i] += aGradX * aGradX + aGradY * aGradY;
  }
  // Edge map
  if (aEdgeMap != 0)
  {
    CMatrix<float> &aMap = *aEdgeMap;
    for (int i = 0; i < aSize; i++)
    {
      aResultX.data()[i] += aEdgeWeight * aMap.data()[i] * aMap.data()[i];
      aResultY.data()[i] += aEdgeWeight * aMap.data()[i] * aMap.data()[i];
    }
  }
  // Compute diffusivity
  for (int i = 0; i < aSize; i++)
  {
    aResultX.data()[i] = 1.0f / sqrt(aResultX.data()[i] + 1e-6);
    aResultY.data()[i] = 1.0f / sqrt(aResultY.data()[i] + 1e-6);
  }
}

// nonlinearIteration
void COpticFlow::nonlinearIteration(CTensor<float> &aFirst, CTensor<float> &aSecond, CMatrix<float> &aConfidence,
                                    CTensor<float> &aGeometricU, CTensor<float> &aGeometricV, CTensor<float> &aGeometricConfidence, CTensor<float> &aFlow,
                                    CMatrix<float> *aEdgeMap, float aAlpha, float aBeta, float aGamma, float aEdgeWeight, int aFixedPointIterations, int aSORIterations, float aOmega)
{
  // Precomputations
  int aXSize = aFirst.xSize();
  int aYSize = aFirst.ySize();
  int aHypos = aGeometricU.zSize();
  int aSize = aXSize * aYSize;
  // Compute structure tensor and second order tensor
  CTensor<float> ST(aXSize, aYSize, 6, 0);
  float *pST0 = ST.data();
  float *pST1 = ST.data() + aSize;
  float *pST2 = ST.data() + 2 * aSize;
  float *pST3 = ST.data() + 3 * aSize;
  float *pST4 = ST.data() + 4 * aSize;
  float *pST5 = ST.data() + 5 * aSize;
  CMatrix<float> dx(aXSize, aYSize);
  CMatrix<float> dy(aXSize, aYSize);
  CMatrix<float> dz(aXSize, aYSize);
  CTensor<float> ST2(aXSize, aYSize, 6, 0);
  float *pST20 = ST2.data();
  float *pST21 = ST2.data() + aSize;
  float *pST22 = ST2.data() + 2 * aSize;
  float *pST23 = ST2.data() + 3 * aSize;
  float *pST24 = ST2.data() + 4 * aSize;
  float *pST25 = ST2.data() + 5 * aSize;
  CMatrix<float> hxx(aXSize, aYSize);
  CMatrix<float> hyy(aXSize, aYSize);
  CMatrix<float> hxy(aXSize, aYSize);
  CMatrix<float> hxz(aXSize, aYSize);
  CMatrix<float> hyz(aXSize, aYSize);
  CMatrix<float> aImage(aXSize, aYSize);
  CDerivative<float> aDerivative(5);
  CDerivative<float> aDerivative2(5);
  int i2 = 0;
  for (int k = 0; k < aFirst.zSize(); k++)
  {
    for (int i = 0; i < aSize; i++, i2++)
    {
      aImage.data()[i] = aFirst.data()[i2];
      dz.data()[i] = aSecond.data()[i2] - aFirst.data()[i2];
    }
    NFilter::filter(aImage, dx, aDerivative, 1);
    NFilter::filter(aImage, dy, 1, aDerivative);
    NFilter::filter(dx, hxx, aDerivative2, 1);
    NFilter::filter(dx, hxy, 1, aDerivative2);
    NFilter::filter(dy, hyy, 1, aDerivative2);
    NFilter::filter(dz, hxz, aDerivative2, 1);
    NFilter::filter(dz, hyz, 1, aDerivative2);
    for (int i = 0; i < aSize; i++)
    {
      float &xx = hxx.data()[i];
      float &yy = hyy.data()[i];
      float &xy = hxy.data()[i];
      float &xz = hxz.data()[i];
      float &yz = hyz.data()[i];
      float mag = 1.0f / sqrt(dx.data()[i] * dx.data()[i] + dy.data()[i] * dy.data()[i] + 1e-6f);
      *(pST0 + i) += mag * dx.data()[i] * dx.data()[i];
      *(pST1 + i) += mag * dy.data()[i] * dy.data()[i];
      *(pST2 + i) += mag * dz.data()[i] * dz.data()[i];
      *(pST3 + i) += mag * dx.data()[i] * dy.data()[i];
      *(pST4 + i) += mag * dx.data()[i] * dz.data()[i];
      *(pST5 + i) += mag * dy.data()[i] * dz.data()[i];
      float magx = 1.0f / sqrt(xx * xx + xy * xy + 1e-6f);
      float magy = 1.0f / sqrt(xy * xy + yy * yy + 1e-6f);
      *(pST20 + i) += (magx * xx * xx + magy * xy * xy);
      *(pST21 + i) += (magx * xy * xy + magy * yy * yy);
      *(pST22 + i) += (magx * xz * xz + magy * yz * yz);
      *(pST23 + i) += (xy * (magx * xx + magy * yy));
      *(pST24 + i) += (magx * xz * xx + magy * xy * yz);
      *(pST25 + i) += (magx * xz * xy + magy * yz * yy);
    }
  }
  float *pGeoDataU = aGeometricU.data();
  float *pGeoDataV = aGeometricV.data();
  float *pU = aFlow.data();
  float *pV = aFlow.data() + aSize;
  // Initialize flow increment with 0
  CTensor<float> aFlowIncrement(aXSize, aYSize, 2, 0);
  // Precomputations
  float invAlpha = 1.0 / aAlpha;
  for (int i = 0; i < aSize; i++)
  {
    float aConf = aConfidence.data()[i];
    *(pST0 + i) *= aConf;
    *(pST1 + i) *= aConf;
    *(pST2 + i) *= aConf;
    *(pST3 + i) *= aConf;
    *(pST4 + i) *= aConf;
    *(pST5 + i) *= aConf;
    *(pST20 + i) *= aConf;
    *(pST21 + i) *= aConf;
    *(pST22 + i) *= aConf;
    *(pST23 + i) *= aConf;
    *(pST24 + i) *= aConf;
    *(pST25 + i) *= aConf;
  }
  // Precompute Laplacian of coarse U and V
  CMatrix<float> U1(aXSize, aYSize);
  CMatrix<float> V1(aXSize, aYSize);
  CMatrix<float> U2(aXSize, aYSize);
  CMatrix<float> V2(aXSize, aYSize);
  CMatrix<float> U3(aXSize, aYSize);
  CMatrix<float> V3(aXSize, aYSize);
  CMatrix<float> U4(aXSize, aYSize);
  CMatrix<float> V4(aXSize, aYSize);
  int i = 0;
  for (int y = 0; y < aYSize; y++)
    for (int x = 0; x < aXSize; x++, i++)
    {
      float u = *(pU + i);
      float v = *(pV + i);
      if (x > 0)
      {
        U1.data()[i] = *(pU + i - 1) - u;
        V1.data()[i] = *(pV + i - 1) - v;
      }
      if (y > 0)
      {
        U2.data()[i] = *(pU + i - aXSize) - u;
        V2.data()[i] = *(pV + i - aXSize) - v;
      }
      if (x + 1 < aXSize)
      {
        U3.data()[i] = *(pU + i + 1) - u;
        V3.data()[i] = *(pV + i + 1) - v;
      }
      if (y + 1 < aYSize)
      {
        U4.data()[i] = *(pU + i + aXSize) - u;
        V4.data()[i] = *(pV + i + aXSize) - v;
      }
    }
  float aNegOmega = 1.0 - aOmega;
  CMatrix<float> GX(aXSize, aYSize);
  CMatrix<float> GY(aXSize, aYSize);
  CMatrix<float> R1(aXSize, aYSize);
  CMatrix<float> R2(aXSize, aYSize);
  CMatrix<float> R3Sum(aXSize, aYSize);
  CMatrix<float> R3U(aXSize, aYSize);
  CMatrix<float> R3V(aXSize, aYSize);
  for (int t = 0; t < aFixedPointIterations; t++)
  {
    // Update diffusivities
    diffusivity(aFlow, aFlowIncrement, GX, GY, aEdgeMap, aEdgeWeight);
    // Update robust factor of data term
    float *pUIncrement = aFlowIncrement.data();
    float *pVIncrement = aFlowIncrement.data() + aSize;
    for (int i = 0; i < aSize; i++)
    {
      float u = *(pUIncrement + i);
      float v = *(pVIncrement + i);
      R1.data()[i] = *(pST0 + i) * u * u + *(pST1 + i) * v * v + *(pST2 + i) + 2.0 * (*(pST3 + i) * u * v + *(pST4 + i) * u + *(pST5 + i) * v);
      if (R1.data()[i] < 0)
        R1.data()[i] = 0.0;
      R1.data()[i] = invAlpha / sqrt(R1.data()[i] + 1e-6);
      R2.data()[i] = *(pST20 + i) * u * u + *(pST21 + i) * v * v + *(pST22 + i) + 2.0 * (*(pST23 + i) * u * v + *(pST24 + i) * u + *(pST25 + i) * v);
      if (R2.data()[i] < 0)
        R2.data()[i] = 0.0;
      R2.data()[i] = invAlpha * aGamma / sqrt(R2.data()[i] + 1e-6);
    }
    int j = 0;
    R3Sum = 0;
    R3U = 0;
    R3V = 0;
    for (int k = 0; k < aHypos; k++)
      for (int i = 0; i < aSize; i++, j++)
        if (aGeometricConfidence.data()[j] > 0)
        {
          float tempu = *(pUIncrement + i) + *(pU + i) - *(pGeoDataU + j);
          float tempv = *(pVIncrement + i) + *(pV + i) - *(pGeoDataV + j);
          float temp = tempu * tempu + tempv * tempv;
          float r3 = invAlpha * aBeta * aGeometricConfidence.data()[j] / sqrt(temp + 1e-6);
          R3Sum.data()[i] += r3;
          R3U.data()[i] += r3 * (*(pU + i) - *(pGeoDataU + j));
          R3V.data()[i] += r3 * (*(pV + i) - *(pGeoDataV + j));
        }
    // SOR Iterations
    for (int sor = 0; sor < aSORIterations; sor++)
    {
      int i = 0;
      int i2 = aSize;
      for (int y = 0; y < aYSize; y++)
        for (int x = 0; x < aXSize; x++, i++, i2++)
        {
          float aNum = 0.0f;
          float aSum = 0.0f;
          float aSum2 = 0.0f;
          if (x > 0)
          {
            float g = GX.data()[i - 1];
            aNum += g;
            aSum += g * (aFlowIncrement.data()[i - 1] + U1.data()[i]);
            aSum2 += g * (aFlowIncrement.data()[i2 - 1] + V1.data()[i]);
          }
          if (y > 0)
          {
            float g = GY.data()[i - aXSize];
            aNum += g;
            aSum += g * (aFlowIncrement.data()[i - aXSize] + U2.data()[i]);
            aSum2 += g * (aFlowIncrement.data()[i2 - aXSize] + V2.data()[i]);
          }
          if (x + 1 < aXSize)
          {
            float g = GX.data()[i];
            aNum += g;
            aSum += g * (aFlowIncrement.data()[i + 1] + U3.data()[i]);
            aSum2 += g * (aFlowIncrement.data()[i2 + 1] + V3.data()[i]);
          }
          if (y + 1 < aYSize)
          {
            float g = GY.data()[i];
            aNum += g;
            aSum += g * (aFlowIncrement.data()[i + aXSize] + U4.data()[i]);
            aSum2 += g * (aFlowIncrement.data()[i2 + aXSize] + V4.data()[i]);
          }
          float aTemp = aOmega / (aNum + *(pST0 + i) * R1.data()[i] + *(pST20 + i) * R2.data()[i] + R3Sum.data()[i]);
          aFlowIncrement.data()[i] = aNegOmega * aFlowIncrement.data()[i] + (aSum - R1.data()[i] * (*(pST3 + i) * aFlowIncrement.data()[i2] + *(pST4 + i)) - R2.data()[i] * (*(pST23 + i) * aFlowIncrement.data()[i2] + *(pST24 + i)) - R3U.data()[i]) * aTemp;
          aTemp = aOmega / (aNum + *(pST1 + i) * R1.data()[i] + *(pST21 + i) * R2.data()[i] + R3Sum.data()[i]);
          aFlowIncrement.data()[i2] = aNegOmega * aFlowIncrement.data()[i2] + (aSum2 - R1.data()[i] * (*(pST3 + i) * aFlowIncrement.data()[i] + *(pST5 + i)) - R2.data()[i] * (*(pST23 + i) * aFlowIncrement.data()[i] + *(pST25 + i)) - R3V.data()[i]) * aTemp;
        }
    }
  }
  // Add increment to flow
  aFlow += aFlowIncrement;
}

// warpingGeometric
void COpticFlow::warpingGeometric(CTensor<float> aFirst, CTensor<float> aSecond, CTensor<float> &aFlow,
                                  CTensor<float> &aGeometricU, CTensor<float> &aGeometricV, CTensor<float> &aGeometricConfidence, CMatrix<float> *aEdgeMap,
                                  float aSigma, float aAlpha, float aBeta, float aGamma, float aEdgeWeight, float aEta, int aFixedpointIterations, int aSORIterations, float aOmega)
{
  // Detect maximum number of levels
  float aMinSize = aFirst.ySize();
  if (aFirst.xSize() < aMinSize)
    aMinSize = aFirst.xSize();
  int aMaxScale = (int)(log(15 / aMinSize) / log(aEta));
  if (aMaxScale < 0)
    aMaxScale = 0;
  // Presmooth input images
  if (aSigma > 0.0f)
  {
    CSmooth<float> aGauss(aSigma, 3);
    NFilter::filter(aFirst, aGauss, 1, 1);
    NFilter::filter(aFirst, 1, aGauss, 1);
    NFilter::filter(aSecond, aGauss, 1, 1);
    NFilter::filter(aSecond, 1, aGauss, 1);
  }
  // Initialize optic flow
  aFlow.setSize(aFirst.xSize(), aFirst.ySize(), 2);
  aFlow = 0;
  // Iterate coarse to fine
  for (int s = aMaxScale; s >= -5; s--)
  {
    int aScale = s;
    if (aScale < 0)
      aScale = 0;
    int aCoarseXSize = (int)ceil(aFirst.xSize() * pow(aEta, aScale + 1));
    int aCoarseYSize = (int)ceil(aFirst.ySize() * pow(aEta, aScale + 1));
    int aXSize = (int)ceil(aFirst.xSize() * pow(aEta, aScale));
    int aYSize = (int)ceil(aFirst.ySize() * pow(aEta, aScale));
    if (aXSize == aCoarseXSize && aYSize == aCoarseYSize)
      continue;
    int aSize = aXSize * aYSize;
    CTensor<float> aFirstCoarse(aFirst);
    CTensor<float> aSecondCoarse(aSecond);
    CTensor<float> aGeometricUCoarse(aGeometricU);
    CTensor<float> aGeometricVCoarse(aGeometricV);
    CTensor<float> aGeometricConfidenceCoarse(aGeometricConfidence);
    CMatrix<float> aEdgeMapCoarse;
    if (aEdgeMap != 0)
      aEdgeMapCoarse = *aEdgeMap;
    // Downsample images and geometric data
    if (s > 0)
    {
      aFirstCoarse.downsample(aXSize, aYSize);
      aSecondCoarse.downsample(aXSize, aYSize);
      if (aEdgeMap != 0)
        aEdgeMapCoarse.downsample(aXSize, aYSize);
      aGeometricUCoarse.downsample(aXSize, aYSize, aGeometricConfidence);
      aGeometricVCoarse.downsample(aXSize, aYSize, aGeometricConfidence);
      aGeometricConfidenceCoarse.downsample(aXSize, aYSize, aGeometricConfidence);
      // Correct geometric data (smaller images -> smaller flow vectors)
      float downX = (float)aXSize / aFirst.xSize();
      float downY = (float)aYSize / aFirst.ySize();
      aGeometricUCoarse *= downX;
      aGeometricVCoarse *= downY;
      // Downsample and correct initial flow (same reason)
      if (aScale == aMaxScale)
      {
        aFlow.downsample(aXSize, aYSize);
        for (int i = 0; i < aSize; i++)
        {
          aFlow.data()[i] *= downX;
          aFlow.data()[i + aSize] *= downY;
        }
      }
    }
    // Upsampling and correction of coarse result (larger images -> larger vectors)
    if (aScale < aMaxScale && s >= 0)
    {
      aFlow.upsampleBilinear(aXSize, aYSize);
      float upX = (float)aXSize / aCoarseXSize;
      float upY = (float)aYSize / aCoarseYSize;
      for (int i = 0; i < aSize; i++)
      {
        aFlow.data()[i] *= upX;
        aFlow.data()[i + aSize] *= upY;
      }
    }
    CMatrix<float> aConfidence(aXSize, aYSize, 1.0);
    // Warp second image
    CTensor<float> aWarped(aXSize, aYSize, aFirst.zSize());
    if (aScale == aMaxScale)
      aWarped = aSecondCoarse;
    else
    {
      int i = 0;
      for (int y = 0; y < aYSize; y++)
        for (int x = 0; x < aXSize; x++, i++)
        {
          float ax = x + aFlow.data()[i];
          float ay = y + aFlow.data()[i + aSize];
          int x1 = (int)(ax);
          int x2 = x1 + 1;
          int y1 = (int)(ay);
          int y2 = y1 + 1;
          float alphaX = ax - x1;
          float alphaY = ay - y1;
          if (x1 < 0 || x2 >= aXSize || y1 < 0 || y2 >= aYSize)
          {
            aConfidence.data()[i] = 0.0;
            for (int k = 0; k < aWarped.zSize(); k++)
              aWarped.data()[i + k * aSize] = aFirstCoarse.data()[i + k * aSize];
          }
          else
            for (int k = 0; k < aWarped.zSize(); k++)
            {
              float a = (1.0 - alphaX) * aSecondCoarse(x1, y1, k) + alphaX * aSecondCoarse(x2, y1, k);
              float b = (1.0 - alphaX) * aSecondCoarse(x1, y2, k) + alphaX * aSecondCoarse(x2, y2, k);
              aWarped.data()[i + k * aSize] = (1.0 - alphaY) * a + alphaY * b;
            }
        }
    }
    // Compute optic flow
    if (s == -1)
      aSORIterations *= 2;
    if (s < 0)
      aGeometricConfidenceCoarse = 0.0;
    if (aEdgeMap == 0)
      nonlinearIteration(aFirstCoarse, aWarped, aConfidence, aGeometricUCoarse, aGeometricVCoarse, aGeometricConfidenceCoarse, aFlow, 0, aAlpha, aBeta, aGamma * pow(aEta, aScale), aEdgeWeight, aFixedpointIterations, aSORIterations, aOmega);
    else
      nonlinearIteration(aFirstCoarse, aWarped, aConfidence, aGeometricUCoarse, aGeometricVCoarse, aGeometricConfidenceCoarse, aFlow, &aEdgeMapCoarse, aAlpha, aBeta, aGamma * pow(aEta, aScale), aEdgeWeight, aFixedpointIterations, aSORIterations, aOmega);
  }
}

// Visualization tools ---------------------------------------------------------

// cartesianToRGB
void COpticFlow::cartesianToRGB(float x, float y, float &R, float &G, float &B)
{
  float radius = sqrt(x * x + y * y);
  if (radius > 1)
    radius = 1;
  float phi;
  if (x == 0.0)
    if (y >= 0.0)
      phi = 0.5 * NMath::Pi;
    else
      phi = 1.5 * NMath::Pi;
  else if (x > 0.0)
    if (y >= 0.0)
      phi = atan(y / x);
    else
      phi = 2.0 * NMath::Pi + atan(y / x);
  else
    phi = NMath::Pi + atan(y / x);
  float alpha, beta; // weights for linear interpolation
  phi *= 0.5;
  // interpolation between red (0) and blue (0.25 * NMath::Pi)
  if ((phi >= 0.0) && (phi < 0.125 * NMath::Pi))
  {
    beta = phi / (0.125 * NMath::Pi);
    alpha = 1.0 - beta;
    R = (int)(radius * (alpha * 255.0 + beta * 255.0));
    G = (int)(radius * (alpha * 0.0 + beta * 0.0));
    B = (int)(radius * (alpha * 0.0 + beta * 255.0));
  }
  if ((phi >= 0.125 * NMath::Pi) && (phi < 0.25 * NMath::Pi))
  {
    beta = (phi - 0.125 * NMath::Pi) / (0.125 * NMath::Pi);
    alpha = 1.0 - beta;
    R = (int)(radius * (alpha * 255.0 + beta * 64.0));
    G = (int)(radius * (alpha * 0.0 + beta * 64.0));
    B = (int)(radius * (alpha * 255.0 + beta * 255.0));
  }
  // interpolation between blue (0.25 * NMath::Pi) and green (0.5 * NMath::Pi)
  if ((phi >= 0.25 * NMath::Pi) && (phi < 0.375 * NMath::Pi))
  {
    beta = (phi - 0.25 * NMath::Pi) / (0.125 * NMath::Pi);
    alpha = 1.0 - beta;
    R = (int)(radius * (alpha * 64.0 + beta * 0.0));
    G = (int)(radius * (alpha * 64.0 + beta * 255.0));
    B = (int)(radius * (alpha * 255.0 + beta * 255.0));
  }
  if ((phi >= 0.375 * NMath::Pi) && (phi < 0.5 * NMath::Pi))
  {
    beta = (phi - 0.375 * NMath::Pi) / (0.125 * NMath::Pi);
    alpha = 1.0 - beta;
    R = (int)(radius * (alpha * 0.0 + beta * 0.0));
    G = (int)(radius * (alpha * 255.0 + beta * 255.0));
    B = (int)(radius * (alpha * 255.0 + beta * 0.0));
  }
  // interpolation between green (0.5 * NMath::Pi) and yellow (0.75 * NMath::Pi)
  if ((phi >= 0.5 * NMath::Pi) && (phi < 0.75 * NMath::Pi))
  {
    beta = (phi - 0.5 * NMath::Pi) / (0.25 * NMath::Pi);
    alpha = 1.0 - beta;
    R = (int)(radius * (alpha * 0.0 + beta * 255.0));
    G = (int)(radius * (alpha * 255.0 + beta * 255.0));
    B = (int)(radius * (alpha * 0.0 + beta * 0.0));
  }
  // interpolation between yellow (0.75 * NMath::Pi) and red (Pi)
  if ((phi >= 0.75 * NMath::Pi) && (phi <= NMath::Pi))
  {
    beta = (phi - 0.75 * NMath::Pi) / (0.25 * NMath::Pi);
    alpha = 1.0 - beta;
    R = (int)(radius * (alpha * 255.0 + beta * 255.0));
    G = (int)(radius * (alpha * 255.0 + beta * 0.0));
    B = (int)(radius * (alpha * 0.0 + beta * 0.0));
  }
  if (R < 0)
    R = 0;
  if (G < 0)
    G = 0;
  if (B < 0)
    B = 0;
  if (R > 255)
    R = 255;
  if (G > 255)
    G = 255;
  if (B > 255)
    B = 255;
}

// flowToImage
void COpticFlow::flowToImage(CTensor<float> &aFlow, CTensor<float> &aImage, float aDivisor)
{
  aImage.setSize(aFlow.xSize(), aFlow.ySize(), 3);
  float aFactor = sqrt(0.5) * 0.5 / aDivisor;
  int aSize = aFlow.xSize() * aFlow.ySize();
  for (int i = 0; i < aSize; i++)
  {
    float R, G, B;
    cartesianToRGB(aFactor * aFlow.data()[i], aFactor * aFlow.data()[i + aSize], R, G, B);
    aImage.data()[i] = R;
    aImage.data()[i + aSize] = G;
    aImage.data()[i + 2 * aSize] = B;
  }
}

// Others ----------------------------------------------------------------------

// warp
void COpticFlow::warp(const CTensor<float> &aImage, CTensor<float> &aWarped, CMatrix<bool> &aOutside, CTensor<float> &aFlow)
{
  int aXSize = aImage.xSize();
  int aYSize = aImage.ySize();
  aOutside = false;
  for (int y = 0; y < aYSize; y++)
    for (int x = 0; x < aXSize; x++)
    {
      float ax = x + aFlow(x, y, 0);
      float ay = y + aFlow(x, y, 1);
      int x1 = (int)(ax);
      int x2 = x1 + 1;
      int y1 = (int)(ay);
      int y2 = y1 + 1;
      float alphaX = ax - x1;
      float alphaY = ay - y1;
      if (x1 < 0 || x2 >= aXSize || y1 < 0 || y2 >= aYSize)
        aOutside(x, y) = true;
      else
        for (int k = 0; k < aImage.zSize(); k++)
        {
          float a = (1.0 - alphaX) * aImage(x1, y1, k) + alphaX * aImage(x2, y1, k);
          float b = (1.0 - alphaX) * aImage(x1, y2, k) + alphaX * aImage(x2, y2, k);
          aWarped(x, y, k) = (1.0 - alphaY) * a + alphaY * b;
        }
    }
}

// writeMiddlebury
void COpticFlow::writeMiddlebury(CTensor<float> &aFlow, const char *aFilename)
{
  const char *dot = strrchr(aFilename, '.');
  if (dot == NULL || strcmp(dot, ".flo") != 0)
    std::cout << aFilename << " should have extension '.flo'" << std::endl;
  FILE *stream = fopen(aFilename, "wb");
  if (stream == 0)
    std::cout << "Could not open " << aFilename << std::endl;
  // write the header
  fprintf(stream, "PIEH");
  int aXSize = aFlow.xSize();
  int aYSize = aFlow.ySize();
  if ((int)fwrite(&aXSize, sizeof(int), 1, stream) != 1 || (int)fwrite(&aYSize, sizeof(int), 1, stream) != 1)
    std::cout << "Problem writing header" << std::endl;
  // write the data
  for (int y = 0; y < aFlow.ySize(); y++)
    for (int x = 0; x < aFlow.xSize(); x++)
    {
      float u = aFlow(x, y, 0);
      float v = aFlow(x, y, 1);
      fwrite(&u, sizeof(float), 1, stream);
      fwrite(&v, sizeof(float), 1, stream);
    }
  fclose(stream);
}

// readMiddlebury
void COpticFlow::readMiddlebury(const char *aFilename, CTensor<float> &aFlow)
{
  FILE *stream = fopen(aFilename, "rb");
  if (stream == 0)
    std::cout << "Could not open " << aFilename << std::endl;
  float help;
  fread(&help, sizeof(float), 1, stream);
  int aXSize, aYSize;
  fread(&aXSize, sizeof(int), 1, stream);
  fread(&aYSize, sizeof(int), 1, stream);
  aFlow.setSize(aXSize, aYSize, 2);
  for (int y = 0; y < aFlow.ySize(); y++)
    for (int x = 0; x < aFlow.xSize(); x++)
    {
      fread(&aFlow(x, y, 0), sizeof(float), 1, stream);
      fread(&aFlow(x, y, 1), sizeof(float), 1, stream);
    }
  fclose(stream);
}

// angularError
float COpticFlow::angularError(CTensor<float> &aFlow, CTensor<float> &aCorrect)
{
  float aae = 0.0;
  int aCounter = 0;
  for (int y = 0; y < aFlow.ySize(); y++)
    for (int x = 0; x < aFlow.xSize(); x++)
    {
      if (fabs(aCorrect(x, y, 0)) > 1e9 || fabs(aCorrect(x, y, 1)) > 1e9)
        continue;
      float uc = aCorrect(x, y, 0);
      float vc = aCorrect(x, y, 1);
      float ue = aFlow(x, y, 0);
      float ve = aFlow(x, y, 1);
      float aux = (uc * ue + vc * ve + 1.0) / sqrt((uc * uc + vc * vc + 1.0) * (ue * ue + ve * ve + 1.0));
      if (aux > 1.0)
        aux = 1.0;
      else if (aux < -1.0)
        aux = -1.0;
      aae += (180.0 / 3.1415927) * acos(aux);
      aCounter++;
    }
  aae /= aCounter;
  return aae;
}
