#pragma once
/// VTK header file
#include <vtkCamera.h>
#include <vtkCellPicker.h>
#include <vtkColorTransferFunction.h>
#include <vtkContourFilter.h>
#include <vtkDICOMImageReader.h>
#include <vtkErrorCode.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageAnisotropicDiffusion3D.h>
#include <vtkImageClip.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageGradientMagnitude.h>
#include <vtkImageMapToColors.h>
#include <vtkImageMarchingCubes.h>
#include <vtkImageMedian3D.h>
#include <vtkImagePlaneWidget.h>
#include <vtkInteractorStyleImage.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkOpenGLProjectedTetrahedraMapper.h>
#include <vtkOutlineFilter.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkPointPicker.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkRecursiveDividingCubes.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceImageViewer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkSmartPointer.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkTextureObject.h>
#include <vtkDecimatePro.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkImageThreshold.h>
#include <vtkImageStencilData.h>
#include <vtkImageData.h>
#include <vtkImageHistogramStatistics.h>

/// ITK header file
//#include <itkImageSeriesReader.h>
//#include <itkGDCMImageIO.h>
//#include <itkGDCMSeriesFileNames.h>
//#include <itkImageFileWriter.h>
//#include <itkVTKImageToImageFilter.h>
//#include <itkImageToVTKImageFilter.h>
//#include <itkConnectedThresholdImageFilter.h>
//
//
//#define CT	"0008|0060"
//#define PA	"0008|0021"
//
//static const int kDimension = 3;
//typedef itk::Image<int, kDimension> ImageType;
//typedef ImageType::IndexType PixelIndexType;
//typedef itk::VTKImageToImageFilter<ImageType> ConverterType;
//typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;

