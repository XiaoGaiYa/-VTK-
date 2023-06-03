#pragma once

#include <QtWidgets/QMainWindow>
#include <qlabel.h>
#include <qthread>
#include "ui_visualdicom.h"


class FilterSetting;

class vtkDICOMImageReader;
class vtkImageData;
class vtkCellPicker;
class vtkImageMarchingCubes;
class vtkContourFilter;
class vtkRecursiveDividingCubes;
class vtkGenericOpenGLRenderWindow;
class vtkRenderer;
class vtkProperty;
class vtkOpenGLGPUVolumeRayCastMapper;
class vtkResliceImageViewer;
class vtkVolume;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyDataNormals;
class vtkSmoothPolyDataFilter;
class vtkVolumeProperty;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;
class vtkResliceCursorCallback;
class vtkImagePlaneWidget;
class vtkImageViewer2;
class vtkTextProperty;
class vtkTextMapper;
class vtkActor2D;
class ImageInteractorStyle;
class vtkRenderWindowInteractor;
class BoxWidget3D;
class BoxWidget3DCallback;
class InteractorStyle;
class vtkAreaPicker;
class vtkDecimatePro;
class vtkInteractorStyleRubberBandPick;
class Threshold;
class VisualDicom : public QMainWindow
{
    Q_OBJECT

public:
    enum MethodType { kPolygon = 0 , kVolume = 3 };
    enum PolygonMethod { kCubeMove = 0 , kCubeSubdivision , kContourFilter };
    enum VolumeMethod { kRaycast = kVolume , kMaximumDensityProjection };
    enum FilterMethod { kNull , kGaussian, kMidValue , kBilateral };
    enum CutMethod { kThreshold , kBoxWidget , kDrawline };     //, kRegionalGrowth
    enum ViewIdx { kAxial , kSagittal , kCoronal , k3D , kViewNum };
    VisualDicom(QWidget *parent = nullptr); 
    ~VisualDicom();

    virtual void resizeEvent(QResizeEvent* event) override;

private slots:
    void Import();
    void Export();
    void Save();
    void Select(MethodType);
    void Info();
    void Display();
    void Filtering();
    void Cutting();
    void FourView();
    void SingleView(ViewIdx);

private:
    void Init();
    void Update();
    void StateMessage();
    void SliceView();
    void DisplayForCubeMove();
    void DisplayForCubeSubdivision();
    void DisplayForContourFilter();
    void DisplayForRaycast();
    void DisplayForMaximumDensityProjection();
    void GaussionFilter();
    void MidValueFilter();
    void BilateralFilter();
    void RegionalGrowthCut();
    void ThresholdCut();
    void BoxWidgetCut();
    void DrawlineCut();
    void SetViewVisible(bool flag);

private:
    Ui::VisualDicomClass ui;
    FilterSetting* setting_;
    Threshold* threshold_;

    vtkSmartPointer<vtkDICOMImageReader> reader_;
    vtkSmartPointer<vtkImageData> data_;
    vtkSmartPointer<vtkImageData> filter_data_;
    vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> mapper_;
    vtkSmartPointer<vtkVolumeProperty> property_;
    vtkSmartPointer<vtkPiecewiseFunction> opacity_fun_;
    vtkSmartPointer<vtkColorTransferFunction> color_fun_;
    vtkSmartPointer<vtkVolume> volume_;
    vtkSmartPointer<vtkActor> actor_;

    vtkSmartPointer<vtkImageMarchingCubes> march_cube_;
    vtkSmartPointer<vtkContourFilter> contour_filter_;
    vtkSmartPointer<vtkRecursiveDividingCubes> recursive_dividing_cube_;
    vtkSmartPointer<vtkPolyDataMapper> poly_mapper_;

    vtkSmartPointer<vtkRenderer> render_[kViewNum];
    vtkSmartPointer<vtkRenderer> render_polygon_3d_;

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renwin_[kViewNum];
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renwin_polygon_3d_;

    QString path_ = QString();
    QString message_ = QString("滤波算法：%1 <--> 滤波器参数:[%2] <--> 切割风格：%3");
    QLabel* info_left = nullptr;
    QLabel* info_right = nullptr;
    int dim_[3] = {};
    double color_[3] = { 0,0,0 };                                              //颜色
    vtkSmartPointer<vtkTextProperty> text_prop_[3];                                       //属性
    vtkSmartPointer<vtkTextMapper> text_map_[3];                                       //属性
    vtkSmartPointer<vtkActor2D> text_act_[3];                                       //属性
    vtkSmartPointer<vtkImageViewer2> riw_[3];
    vtkSmartPointer<vtkImagePlaneWidget> plane_widget_[3];
    vtkSmartPointer<vtkRenderWindowInteractor> interactor_[5];
    vtkSmartPointer<vtkAreaPicker> picker_;                                      //拾取器
    vtkSmartPointer<ImageInteractorStyle> style_[3];                                      //拾取器
    vtkSmartPointer<InteractorStyle> cut_style_;                              //回调类
    vtkSmartPointer<vtkInteractorStyleRubberBandPick> boxcut_style_;                              //回调类
    vtkSmartPointer<vtkDecimatePro> deci_;
    vtkSmartPointer<vtkSmoothPolyDataFilter> smoother_;
    vtkSmartPointer<vtkPolyDataNormals> normals_;
    vtkSmartPointer<BoxWidget3D> box_widget_;                              //回调类
    vtkSmartPointer<BoxWidget3DCallback> callback_;                              //回调类
    //vtkSmartPointer<vtkInteractorStyle> default_style_;
    //vtkSmartPointer<vtkInteractorStyleDrawPolygon> 
    MethodType type_ = kVolume;
    bool cut_save_inside_ = true;

    //QThread import_thread_;
    //QThread slice_thread_;
    //QThread render_thread_;
};
