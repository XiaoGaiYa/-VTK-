#include "header.h"
#include "region_growth.h"


#if 0
vtkImageData* RegionGrowingByConnectedThredholdFilter(vtkImageData* pImage, double worldSeedPoint[3])
{
    vtkSmartPointer<vtkImageData> imageData =
        vtkSmartPointer<vtkImageData>::New();
    imageData->DeepCopy(pImage);

    // Compute image dimensions and seed index
    int* dims = imageData->GetDimensions();
    int seedIdx[3];
    imageData->ComputeStructuredCoordinates(worldSeedPoint, seedIdx);
    if (seedIdx[0] < 0 || seedIdx[0] >= dims[0] ||
        seedIdx[1] < 0 || seedIdx[1] >= dims[1] ||
        seedIdx[2] < 0 || seedIdx[2] >= dims[2])
    {
        return nullptr;
    }

    // Threshold image to isolate connected region
    vtkSmartPointer<vtkImageThreshold> threshold =
        vtkSmartPointer<vtkImageThreshold>::New();
    threshold->SetInputData(imageData);
    threshold->SetInValue(1);
    threshold->SetOutValue(0);
    threshold->SetOutputScalarTypeToUnsignedChar();
    threshold->ThresholdBetween(1, 255);
    threshold->Update();

    // Create stencil from thresholded image and seed point
    vtkSmartPointer<vtkImageStencilData> stencilData =
        vtkSmartPointer<vtkImageStencilData>::New();
    stencilData->SetSpacing(threshold->GetOutput()->GetSpacing());
    stencilData->SetOrigin(threshold->GetOutput()->GetOrigin());

    // Add seed to stencil data
    vtkIdType seedId = imageData->ComputePointId(seedIdx);
    vtkSmartPointer<vtkImageData> seedImage =
        vtkSmartPointer<vtkImageData>::New();
    seedImage->CopyStructure(imageData);
    unsigned char* seedPtr = (unsigned char*)seedImage->GetScalarPointer(seedIdx);
    *seedPtr = 1;
    stencilData->InitStencil();
    stencilData->InsertNextExtent(seedIdx[0], seedIdx[0], seedIdx[1], seedIdx[1],
        seedIdx[2], seedIdx[2]);
    stencilData->InsertNextValue(1);

    // Apply the stencil to the input image
    vtkSmartPointer<vtkImageStencil> stencil =
        vtkSmartPointer<vtkImageStencil>::New();
    stencil->SetInputData(imageData);
    stencil->SetStencilData(stencilData);
    stencil->ReverseStencilOff();
    stencil->SetBackgroundValue(0);
    stencil->Update();

    // Compute the connected components of the masked image
    vtkSmartPointer<vtkConnectivityFilter> connectivityFilter =
        vtkSmartPointer<vtkConnectivityFilter>::New();
    connectivityFilter->SetInputData(stencil->GetOutput());
    connectivityFilter->SetExtractionModeToAllRegions();
    connectivityFilter->ColorRegionsOn();
    connectivityFilter->Update();

    return connectivityFilter->GetOutput();
}
#endif

#if 0
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkImageFileWriter.h>
#include <itkVTKImageToImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>


static const int kDimension = 3;
typedef itk::Image<int, kDimension> ImageType;
typedef ImageType::IndexType PixelIndexType;
typedef itk::VTKImageToImageFilter<ImageType> ConverterType;
typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;

ImageType::Pointer g_itkdata = nullptr;
ImageType::IndexType g_pixel_idx;


using namespace itk;
void ConstructITKImage(vtkImageData* data)
{
    double imageSpacing[3] = { 0.0 };
    double imageOrigin[3] = { 0.0 };
    int imageDimension[3] = { 0 };
    data->GetSpacing(imageSpacing);
    data->GetOrigin(imageOrigin);
    data->GetDimensions(imageDimension);

    const ImageType::SizeType  size = { {imageDimension[0], imageDimension[1], imageDimension[2]} }; //Size along {X,Y,Z}
    const ImageType::IndexType start = { {0,0,0} }; // First index on {X,Y,Z}

    ImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);

    //Types for converting between ITK and VTK
    typedef itk::VTKImageToImageFilter<ImageType> VTKImageToImageType;

    //Converting to ITK Image Format
    VTKImageToImageType::Pointer vtkImageToImageFilter = VTKImageToImageType::New();
    vtkImageToImageFilter->SetInput(data);
    vtkImageToImageFilter->UpdateLargestPossibleRegion();

    //将vtk的图像转化为itk的图像，以便利用itk的分割算法进行分割.
    g_itkdata->SetRegions(region);
    g_itkdata->Allocate();
    g_itkdata = const_cast<itk::Image<int, kDimension>*>(vtkImageToImageFilter->GetImporter()->GetOutput());

    ImageType::SpacingType spacing;
    spacing[0] = imageSpacing[0]; // spacing along X
    spacing[1] = imageSpacing[1]; // spacing along Y
    spacing[2] = imageSpacing[2]; // spacing along Z
    g_itkdata->SetSpacing(spacing);

    ImageType::PointType newOrigin;
    newOrigin[0] = imageOrigin[0];
    newOrigin[1] = imageOrigin[1];
    newOrigin[2] = imageOrigin[2];
    g_itkdata->SetOrigin(newOrigin);

    ImageType::DirectionType direction;
    direction.SetIdentity();
    g_itkdata->SetDirection(direction);
    g_itkdata->UpdateOutputInformation();
}

bool CheckSeedPointInsideImage(double worldSeedPoint[3])
{
    typedef itk::Point< double, ImageType::ImageDimension > PointType;
    PointType point;
    point[0] = worldSeedPoint[0];    // x coordinate
    point[1] = worldSeedPoint[1];    // y coordinate
    point[2] = worldSeedPoint[2];    // z coordinate

    //根据选取的种子点的世界坐标，获取这个种子点在itk图像中的像素索引值.
    bool isInside = g_itkdata->TransformPhysicalPointToIndex(point, g_pixel_idx);
    return isInside;
}

vtkImageData* RegionGrowingByConnectedThredholdFilter(vtkImageData* pImage, double worldSeedPoint[3])
{
    ConstructITKImage(pImage);

    if(!CheckSeedPointInsideImage(worldSeedPoint)) return 0;

    typedef itk::Image< int, kDimension > InternalImageType;
    typedef itk::ConnectedThresholdImageFilter< InternalImageType, InternalImageType > ConnectedFilterType;
    ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();

    connectedThreshold->SetInput(g_itkdata);

    //由于区域生长是在阈值分割后进行的，此时的vtkImageData是二值的，灰度只有0和1。因此基于阈值的区域生长的灰度上下限都是1.
    //区域生长分割后的图像，要经过vtkLookupTable进行颜色映射.vtkLookupTable也只有0和1两个预设值.
    connectedThreshold->SetLower(1);
    connectedThreshold->SetUpper(1);
    connectedThreshold->SetReplaceValue(1);
    connectedThreshold->SetSeed(g_pixel_idx);

    typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;

    //Converting Back from ITK to VTK Image for Visualization.
    ConnectorType::Pointer connector = ConnectorType::New();
    connector->SetInput(connectedThreshold->GetOutput());
    connector->Update();

    return connector->GetOutput();
}

#endif