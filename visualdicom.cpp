#include <qfiledialog.h>
#include <qmessagebox.h>
#include <atomic>
#include <QResizeEvent>
#include <memory>

#include "header.h"
#include "visualdicom.h"
#include "filter_setting.h"
#include "cursor_interface.h"
#include "returntype.h"
#include "drawline_cut.h"
#include "boxwidget.h"
#include "threshold.h"


FiltersSets* res_filters = nullptr;
ThresholdSets* res_threshold = nullptr;
std::unique_ptr<Threshold> g_window = nullptr;


std::once_flag g_setwin, g_setdata;

VisualDicom::VisualDicom(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    this->setting_ = new FilterSetting;
    this->threshold_ = new Threshold;
    this->Init();

    ui.widget_polygon_3d_->setVisible(false);
    ui.widget_3d->setVisible(true);
    connect(ui.action_import, &QAction::triggered, [this] { this->Import(); });
    connect(ui.action_export, &QAction::triggered, [this] { this->Export(); });
    connect(ui.action_save, &QAction::triggered, [this] { this->Save(); });
    connect(ui.action_info, &QAction::triggered, [this] { this->Info(); });

    connect(ui.tool_open, &QAction::triggered, [this] { this->Import(); });
    connect(ui.tool_three, &QAction::triggered, [this] { this->SliceView(); });

    connect(ui.setting_filter, &QAction::triggered, [this] { this->setting_->show(); });
    connect(ui.cut_threshold, &QAction::triggered, [this] { this->threshold_->show(); });
    connect(ui.action_volume, &QAction::triggered, [this] { this->Select(kVolume); });
    connect(ui.action_polygon, &QAction::triggered, [this] { this->Select(kPolygon); });

    connect(ui.tool_save, &QAction::triggered, [this] { this->Save(); });
    connect(ui.tool_info, &QAction::triggered, [this] { this->Info(); });
    connect(ui.tool_visual, &QAction::triggered, [this] { this->Display(); });
    connect(ui.tool_cut, &QAction::triggered, [this] { this->Cutting(); });
    connect(ui.btn_clear, &QPushButton::clicked, [this] { this->ui.text_info->clear(); });

    connect(ui.action_inside, &QAction::triggered, [this] { this->cut_save_inside_ = true; this->StateMessage(); });
    connect(ui.action_outside, &QAction::triggered, [this] { this->cut_save_inside_ = false; this->StateMessage(); });

    connect(ui.comb_filter, &QComboBox::currentIndexChanged, [this] { this->StateMessage(); });
    connect(ui.slider_axial, &QSlider::valueChanged, [this](int slice) { this->style_[0]->SetSliceIndex(slice); });
    connect(ui.slider_sagittal, &QSlider::valueChanged, [this](int slice) { this->style_[1]->SetSliceIndex(slice); });
    connect(ui.slider_coronal, &QSlider::valueChanged, [this](int slice) { this->style_[2]->SetSliceIndex(slice); });
    
    connect(ui.view_axial, &QAction::triggered, [this] { this->SingleView(kAxial); });
    connect(ui.view_sagittal, &QAction::triggered, [this] { this->SingleView(kSagittal); });
    connect(ui.view_coronal, &QAction::triggered, [this] { this->SingleView(kCoronal); });
    connect(ui.view_3d, &QAction::triggered, [this] { this->SingleView(k3D); });
    connect(ui.view_2x2, &QAction::triggered, [this] { this->FourView(); });
    //connect(ui.slider_axial, &QSlider::valueChanged, [this] { this->style_[0]->SetSliceIndex(ui.slider_axial->value()); });
    //connect(ui.slider_sagittal, &QSlider::valueChanged, [](int slice) { cout << "slice index:" << slice << endl; });
    //connect(ui.slider_coronal, &QSlider::valueChanged, [](int slice) { cout << "slice index:" << slice << endl; });

    this->info_left = new QLabel(message_);
    this->info_right = new QLabel("Author:程序员小盖 Title:VisualDicom");
    ui.statusbar->addWidget(info_left);
    ui.statusbar->addPermanentWidget(info_right);
    this->StateMessage();
}

VisualDicom::~VisualDicom()
{}

void VisualDicom::resizeEvent(QResizeEvent * event)
{
    QSize size = event->size();
    ui.edit_path->resize(size.width() - 60, 25);
    ui.widget_left->resize(205, size.height() - 95);
    ui.text_info->resize(205, size.height() - 170);
    ui.btn_clear->move(156, size.height() - 130);

    int nw = (size.width() - 205) / 2;
    int nh = (size.height() - 106) / 2;
    ui.widget_axial->resize(nw - 20, nh - 20);

    ui.slider_axial->resize(20, nh - 20);
    ui.slider_axial->move(nw + 185, 25);
    ui.label_axial->move(205, nh + 5);

    ui.widget_sagittal->resize(nw - 20, nh - 20);
    ui.widget_sagittal->move(nw + 205, 25);
    ui.slider_sagittal->resize(20, nh - 20);
    ui.slider_sagittal->move(nw * 2 + 185, 25);
    ui.label_sagittal->move(nw + 205, nh + 5);

    ui.widget_coronal->resize(nw - 20, nh - 20);
    ui.widget_coronal->move(205, nh + 30);
    ui.slider_coronal->resize(20, nh - 25);
    ui.slider_coronal->move(nw + 185, nh + 30);
    ui.label_coronal->move(205, nh * 2 + 10);

    ui.widget_3d->resize(nw, nh);
    ui.widget_3d->move(nw + 205, nh + 30);
    ui.widget_polygon_3d_->resize(nw, nh);
    ui.widget_polygon_3d_->move(nw + 205, nh + 30);

    //for (int i = 0; i < kViewNum; i++)
    //{
    //    this->renwin_[i]->SetSize(nw - 20, nh - 20);
    //}
    //this->renwin_[k3D]->SetSize(nw, nh);
    //this->renwin_polygon_3d_->SetSize(nw, nh);
}

void VisualDicom::Import()
{
    QFileDialog fdlg;
    this->path_ = fdlg.getExistingDirectory(this, "选择导入DICOM文件路径");
    if (path_.isEmpty())
    {
        QMessageBox::warning(this, "警告", "文件路径不能为空！！！");
        return;
    }
    ui.edit_path->setText(path_);
    QMessageBox::information(this, "打开文件成功", path_);
    this->Update();
}

void VisualDicom::Export()
{
}

void VisualDicom::Select(MethodType type)
{
    type_ = type;
    bool op = (type != kPolygon);
    this->ui.comb_visual_volume->setVisible(op);
    this->ui.comb_visual_polygon->setVisible(!op);
    this->ui.widget_3d->setVisible(op);
    this->ui.widget_polygon_3d_->setVisible(!op);
}

void VisualDicom::Save()
{
}

void VisualDicom::Info()
{
    if (!reader_.Get())
    {
        QMessageBox::critical(this, "error", "数据集为空！！！");
        return;
    }
    if (dim_[0] > 2 || dim_[1] > 2 || dim_[2] > 2)
    {
        QString dim0 = QString::number(dim_[0]);
        QString dim1 = QString::number(dim_[1]);
        QString dim2 = QString::number(dim_[2]);
        QString space = " ";
        ui.text_info->append("Dimensions:" + space + dim0 + space + dim1 + space + dim2);

        QString fileExtensions = this->reader_->GetFileExtensions();
        ui.text_info->append("fileExtensions: " + fileExtensions);

        QString descriptiveName = this->reader_->GetDescriptiveName();
        ui.text_info->append("descriptiveName: " + descriptiveName);

        double* pixelSpacing = this->reader_->GetPixelSpacing();
        QString pixelS = QString::number(*pixelSpacing, 10, 5);
        ui.text_info->append("pixelSpacing: " + pixelS);

        int width = this->reader_->GetWidth();
        QString wid = QString::number(width);
        ui.text_info->append("width: " + wid);

        int height = this->reader_->GetHeight();
        QString heig = QString::number(height);
        ui.text_info->append("height: " + heig);

        float* imagePositionPatient = this->reader_->GetImagePositionPatient();
        QString imPP = QString::number(*imagePositionPatient, 10, 5);
        ui.text_info->append("imagePositionPatient: " + imPP);

        float* imageOrientationPatient = this->reader_->GetImageOrientationPatient();
        QString imOP = QString::number(*imageOrientationPatient, 10, 5);
        ui.text_info->append("imageOrientationPatient: " + imOP);

        int bitsAllocated = this->reader_->GetBitsAllocated();
        QString bitsA = QString::number(bitsAllocated);
        ui.text_info->append("bitsAllocated: " + bitsA);

        int pixelRepresentation = this->reader_->GetPixelRepresentation();
        QString pixelR = QString::number(pixelRepresentation);
        ui.text_info->append("pixelRepresentation: " + pixelR);

        int numberOfComponents = this->reader_->GetNumberOfComponents();
        QString numberO = QString::number(numberOfComponents);
        ui.text_info->append("numberOfComponents: " + numberO);

        QString transferSyntaxUID = this->reader_->GetTransferSyntaxUID();
        ui.text_info->append("transferSyntaxUID: " + transferSyntaxUID);

        float rescaleSlope = this->reader_->GetRescaleSlope();
        QString rescaleS = QString::number(rescaleSlope, 10, 5);
        ui.text_info->append("rescaleSlope: " + rescaleS);

        float rescaleOffset = this->reader_->GetRescaleOffset();
        QString rescaleO = QString::number(rescaleOffset, 10, 5);
        ui.text_info->append("rescaleOffset: " + rescaleO);

        QString patientName = this->reader_->GetPatientName();
        ui.text_info->append("patientName: " + patientName);

        QString studyUID = this->reader_->GetStudyUID();
        ui.text_info->append("studyUID: " + studyUID);

        QString studyID = this->reader_->GetStudyID();
        ui.text_info->append("studyID: " + studyID);

        float gantryAngle = this->reader_->GetGantryAngle();
        QString gantryA = QString::number(gantryAngle, 10, 5);
        ui.text_info->append("gantryAngle: " + gantryA);
    }
}

void VisualDicom::Display()
{
    if (!reader_.Get())
    {
        QMessageBox::warning(this, "warning", "没有可读取的数据！！！");
        return;
    }
    bool is_volume = ui.comb_visual_volume->isVisible();
    QComboBox* box = is_volume ? ui.comb_visual_volume : ui.comb_visual_polygon;
    int idx = box->currentIndex() + type_;
    switch (idx)
    {
        case kCubeMove: this->DisplayForCubeMove(); break;
        case kCubeSubdivision: this->DisplayForCubeSubdivision(); break;
        case kContourFilter: this->DisplayForContourFilter(); break;
        case kRaycast: this->DisplayForRaycast(); break;
        case kMaximumDensityProjection: this->DisplayForMaximumDensityProjection(); break;
        default: break;
    }
}

void VisualDicom::Filtering()
{
    res_filters = this->setting_->Result();
    int idx = ui.comb_filter->currentIndex();
    switch (idx)
    {
        case kNull: this->filter_data_ = reader_->GetOutput(); break;
        case kGaussian: this->GaussionFilter(); break;
        case kMidValue: this->MidValueFilter(); break;
        case kBilateral: this->BilateralFilter(); break;
        default: break;
    }
}

void VisualDicom::Cutting()
{
    int idx = ui.comb_cut->currentIndex();
    switch (idx)
    {
        case kThreshold: ThresholdCut(); break;
        //case kRegionalGrowth: RegionalGrowthCut(); break;
        case kBoxWidget: BoxWidgetCut(); break;
        case kDrawline: DrawlineCut(); break;
        default: break;
    }
}

void VisualDicom::SetViewVisible(bool flag)
{
    ui.widget_axial->setVisible(flag);
    ui.slider_axial->setVisible(flag);
    ui.label_axial->setVisible(flag);

    ui.widget_sagittal->setVisible(flag);
    ui.slider_sagittal->setVisible(flag);
    ui.label_sagittal->setVisible(flag);

    ui.widget_coronal->setVisible(flag);
    ui.slider_coronal->setVisible(flag);
    ui.label_coronal->setVisible(flag);

    ui.widget_3d->setVisible(flag && type_ == kVolume);
    ui.widget_polygon_3d_->setVisible(flag && type_ == kPolygon);
}

void VisualDicom::FourView()
{
    printf("FourView trigged\n");
    this->SetViewVisible(true);
    this->resize(width() + 1, height() + 1);
    this->resize(width() - 1, height() - 1);
}

void VisualDicom::SingleView(ViewIdx idx)
{
    this->SetViewVisible(false);

    int x = 205;
    int y = 25;
    int w = ui.central_widget->width() - 225;
    int h = ui.central_widget->height() - 45;
    printf("widget:%d, height:%d\n", w, h);

    switch (idx)
    {
        case VisualDicom::kAxial: 
        {
            ui.widget_axial->resize(w, h);
            ui.widget_axial->move(x, y);
            ui.slider_axial->resize(20, h);
            ui.slider_axial->move(x + w, 25);
            ui.label_axial->move(205, y + h);
            ui.widget_axial->setVisible(true);
            ui.slider_axial->setVisible(true);
            ui.label_axial->setVisible(true);
            break;
        }
        case VisualDicom::kSagittal: 
        {
            ui.widget_sagittal->resize(w, h);
            ui.widget_sagittal->move(x, y);
            ui.slider_sagittal->resize(20, h);
            ui.slider_sagittal->move(x + w, 25);
            ui.label_sagittal->move(205, y + h);
            ui.widget_sagittal->setVisible(true);
            ui.slider_sagittal->setVisible(true);
            ui.label_sagittal->setVisible(true);
            break;
        }
        case VisualDicom::kCoronal: 
        {
            ui.widget_coronal->resize(w, h);
            ui.widget_coronal->move(x, y);
            ui.slider_coronal->resize(20, h);
            ui.slider_coronal->move(x + w, 25);
            ui.label_coronal->move(205, y + h);
            ui.widget_coronal->setVisible(true);
            ui.slider_coronal->setVisible(true);
            ui.label_coronal->setVisible(true);
            break;
        }
        case VisualDicom::k3D: 
        {
            //QVTKOpenGLNativeWidget* widget = this->type_ == kVolume ? ui.widget_3d : ui.widget_polygon_3d_;
            ui.widget_3d->resize(w + 20, h + 20);
            ui.widget_3d->move(x, y);
            //ui.widget_3d->setVisible(this->type_ == kVolume);

            ui.widget_polygon_3d_->resize(w + 20, h + 20);
            ui.widget_polygon_3d_->move(x, y);
            //ui.widget_polygon_3d_->setVisible(this->type_ != kVolume);
            this->type_ == kVolume ? ui.widget_3d->setVisible(true) : ui.widget_polygon_3d_->setVisible(true);
            break;
        }
        case VisualDicom::kViewNum: 
        {
            this->FourView();
            break;
        }
        default: break;
    }
}

void VisualDicom::Init()
{
    this->reader_ = vtkSmartPointer<vtkDICOMImageReader>::New();
    this->property_ = vtkSmartPointer<vtkVolumeProperty>::New();
    this->opacity_fun_ = vtkSmartPointer<vtkPiecewiseFunction>::New();
    this->color_fun_ = vtkSmartPointer<vtkColorTransferFunction>::New();
    this->volume_ = vtkSmartPointer<vtkVolume>::New();
    this->actor_ = vtkSmartPointer<vtkActor>::New();

    for (int i = 0; i < kViewNum; i++)
    {
        this->render_[i] = vtkSmartPointer<vtkRenderer>::New();
        this->renwin_[i] = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        this->renwin_[i]->AddRenderer(render_[i]);
        this->renwin_[i]->SetSize(380, 380);
    }
    for (int i = 0; i < 3; i++)
    {
        this->riw_[i] = vtkSmartPointer<vtkImageViewer2>::New();
        this->text_prop_[i] = vtkSmartPointer<vtkTextProperty>::New();
        this->text_map_[i] = vtkSmartPointer<vtkTextMapper>::New();
        this->style_[i] = vtkSmartPointer<ImageInteractorStyle>::New();

        this->text_act_[i] = vtkSmartPointer<vtkActor2D>::New();
        this->interactor_[i] = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    }
    this->picker_ = vtkSmartPointer<vtkAreaPicker>::New();
    this->cut_style_ = vtkSmartPointer<InteractorStyle>::New();
    this->boxcut_style_ = vtkSmartPointer<vtkInteractorStyleRubberBandPick>::New();

    this->interactor_[4] = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    this->interactor_[4]->SetRenderWindow(renwin_[k3D]);
    this->interactor_[4]->SetInteractorStyle(boxcut_style_);

    this->box_widget_ = vtkSmartPointer<BoxWidget3D>::New();
    this->box_widget_->SetInteractor(interactor_[4]);
    this->box_widget_->SetRepresentation(vtkBoxRepresentation::New());

    this->callback_ = vtkSmartPointer<BoxWidget3DCallback>::New();
    this->callback_->SetVolume(volume_);
    this->box_widget_->AddObserver(vtkCommand::InteractionEvent, callback_);

    this->contour_filter_ = vtkSmartPointer<vtkContourFilter>::New();
    this->deci_ = vtkSmartPointer<vtkDecimatePro>::New();
    this->smoother_ = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    this->normals_ = vtkSmartPointer<vtkPolyDataNormals>::New();


    this->render_polygon_3d_ = vtkSmartPointer<vtkRenderer>::New();
    this->renwin_polygon_3d_ = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    this->mapper_ = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
    this->march_cube_ = vtkSmartPointer<vtkImageMarchingCubes>::New();
    this->contour_filter_ = vtkSmartPointer<vtkContourFilter>::New();
    this->recursive_dividing_cube_ = vtkSmartPointer<vtkRecursiveDividingCubes>::New();
    this->poly_mapper_ = vtkSmartPointer<vtkPolyDataMapper>::New();

    this->mapper_->SetSampleDistance(mapper_->GetSampleDistance() / 2);	//设置光线采样距离

    this->property_->SetInterpolationTypeToLinear();
    this->property_->ShadeOn();  //打开或者关闭阴影测试
    this->property_->SetAmbient(0.1);
    this->property_->SetDiffuse(0.9);  //漫反射
    this->property_->SetSpecular(0.2); //镜面反射
    this->property_->SetSpecularPower(10.0);

    //设置不透明度
    this->opacity_fun_->AddPoint(-3024, 0);
    this->opacity_fun_->AddPoint(129.54, 0);
    this->opacity_fun_->AddPoint(145.24, 0.17);
    this->opacity_fun_->AddPoint(169.92, 0.63);
    this->opacity_fun_->AddPoint(395.58, 0.81);
    this->opacity_fun_->AddPoint(1578.73, 0.81);
    this->opacity_fun_->AddPoint(3071, 0.81);

    //设置颜色属性
    this->color_fun_->AddRGBPoint(-3024, 0, 0, 0);
    this->color_fun_->AddRGBPoint(129.54, 0.55, 0.25, 0.15);
    this->color_fun_->AddRGBPoint(157.02, 1, 1, 1);
    this->color_fun_->AddRGBPoint(169.92, 0.99, 0.87, 0.39);
    this->color_fun_->AddRGBPoint(395.58, 1, 0.88, 0.66);
    this->color_fun_->AddRGBPoint(1578.73, 1, 0.95, 0.96);
    this->color_fun_->AddRGBPoint(3071, 0.83, 0.66, 1);

    this->renwin_[k3D]->AddRenderer(render_[k3D]);
    this->renwin_[k3D]->SetSize(400, 400);
    this->render_[k3D]->SetBackground(0.1, 0.4, 0.2);

    this->property_->SetScalarOpacity(opacity_fun_); //设置不透明度传输函数
    this->property_->SetColor(color_fun_);
    this->volume_->SetProperty(property_);
    this->volume_->SetMapper(mapper_);
    this->render_[k3D]->AddVolume(volume_);

    this->render_polygon_3d_->AddActor(actor_);
    this->renwin_polygon_3d_->AddRenderer(render_polygon_3d_);
    this->renwin_polygon_3d_->SetSize(400, 400);
    this->render_polygon_3d_->SetBackground(0.1, 0.4, 0.2);
    //this->render_polygon_3d_->SetViewport(0, 0, 1, 1);

    this->cut_style_->renwin_ = renwin_polygon_3d_;
    this->cut_style_->actor_ = actor_;
    this->cut_style_->mapper_ = poly_mapper_;
    this->cut_style_->renderer_ = render_polygon_3d_;
    //this->renwin_polygon_3d_->AddRenderer(cut_style_->renderer_);

    this->interactor_[k3D] = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    this->interactor_[k3D]->SetRenderWindow(renwin_polygon_3d_);
    this->interactor_[k3D]->SetPicker(picker_);
    this->interactor_[k3D]->SetInteractorStyle(cut_style_);

    ui.widget_axial->setRenderWindow(renwin_[kAxial]); 
    ui.widget_sagittal->setRenderWindow(renwin_[kSagittal]);
    ui.widget_coronal->setRenderWindow(renwin_[kCoronal]);

    this->actor_->GetProperty()->SetDiffuseColor(1.0, 1.0, 1.0);
    this->actor_->GetProperty()->SetSpecular(.3);
    this->actor_->GetProperty()->SetSpecularPower(20);
}

void VisualDicom::Update()
{
    this->reader_->SetDirectoryName(path_.toStdString().c_str());
    this->reader_->Update();
    this->data_ = reader_->GetOutput();
    this->data_->GetDimensions(dim_);
}

void VisualDicom::StateMessage()
{
    QString msg;
    res_filters = setting_->Result();
    int idx = ui.comb_filter->currentIndex();
    char buf[64];
    switch (idx)
    {
        case 0: msg = this->message_.arg("尚未选择", ""); break;
        case 1: 
        {
            int n = snprintf(buf, sizeof(buf), "标准差:%1.1f,卷积核:%d", res_filters->sigma_gaussion, res_filters->kernelsize_gaussion);
            buf[n] = 0;
            msg = this->message_.arg("高斯平滑").arg(buf);
            break;
        }
        case 2: 
        {
            int n = snprintf(buf, sizeof(buf), "卷积核:%d", res_filters->kernelsize_mid);
            buf[n] = 0;
            msg = this->message_.arg("中值滤波").arg(buf);
            break;
        }
        case 3: 
        {
            int n = snprintf(buf, sizeof(buf), "标准差:%1.1f,迭代次数:%d,扩散阈值:%2.1f", res_filters->sigma_bilateral, res_filters->iterations, res_filters->threshold);
            buf[n] = 0;
            msg = this->message_.arg("双边滤波").arg(buf);
            break;
        }
    }
    QString temp = cut_save_inside_ ? msg.arg("保留内部区域") : msg.arg("保留外部区域");
    this->info_left->setText(temp);
}

void VisualDicom::SliceView()
{
    for (int i = 0; i + 1 < kViewNum; i++)
    {
        this->riw_[i]->SetInputConnection(reader_->GetOutputPort());
        this->riw_[i]->SetRenderWindow(renwin_[i]);
    }
    this->riw_[0]->SetSliceOrientationToXY();
    this->riw_[0]->SetSlice(dim_[2] / 2);
    this->riw_[1]->SetSliceOrientationToXZ();
    this->riw_[1]->SetSlice(dim_[0] / 2);
    this->riw_[2]->SetSliceOrientationToYZ();
    this->riw_[2]->SetSlice(dim_[1] / 2);

    //横断面当前切片数
    this->text_prop_[0]->SetFontFamilyToCourier();
    this->text_prop_[0]->SetFontSize(15);
    this->text_prop_[0]->SetVerticalJustificationToBottom();
    this->text_prop_[0]->SetJustificationToLeft();

    char buf[64];
    for (int i = 0; i < 3; i++)
    {  
        int idx = (i == 0 ? 2 : (i - 1));
        int n = snprintf(buf, sizeof(buf), "SliceNumber:%d/%d\n", riw_[i]->GetSlice(), dim_[idx] - 1);
        buf[n] = 0;
        this->text_map_[i]->SetInput(buf);
        this->text_map_[i]->SetTextProperty(text_prop_[i]);
        this->text_act_[i]->SetMapper(text_map_[i]);
        //this->text_act_[i]->SetPosition(90, 10);
    }
    ui.slider_axial->setRange(0, dim_[2]);
    ui.slider_axial->setValue(dim_[2] / 2);
    ui.slider_sagittal->setRange(0, dim_[0]);
    ui.slider_sagittal->setValue(dim_[0] / 2);
    ui.slider_coronal->setRange(0, dim_[1]);
    ui.slider_coronal->setValue(dim_[1] / 2);

    //交互
    //窗口交互器
    for (int i = 0; i < 3; i++)
    {
        this->style_[i]->SetImageViewer(riw_[i]);
        this->style_[i]->SetStatusMapper(text_map_[i]);
        this->riw_[i]->SetupInteractor(interactor_[i]);
        this->interactor_[i]->SetInteractorStyle(style_[i]);
    }
    //交互风格
    //把文本添加到绘制窗口
    for (int i = 0; i < 3; i++)
    {
        this->riw_[i]->GetRenderer()->AddActor2D(text_act_[i]);
        this->riw_[i]->Render();
    }
}

void VisualDicom::DisplayForCubeMove()
{
    QMessageBox::information(this, "display", "移动立方体算法");
    this->Filtering();
    this->march_cube_->SetInputData(filter_data_);
    this->march_cube_->SetValue(0, 200);
    this->march_cube_->Update();
    //建立映射
    this->cut_style_->data_ = march_cube_->GetOutput();
    //this->cut_style_->output_ = march_cube_->GetOutputPort();
    this->poly_mapper_->SetInputData(march_cube_->GetOutput());
    this->poly_mapper_->ScalarVisibilityOff();
    this->actor_->SetMapper(poly_mapper_);

    this->ui.widget_polygon_3d_->setRenderWindow(renwin_polygon_3d_);
    this->renwin_polygon_3d_->Render();
} 

void VisualDicom::DisplayForCubeSubdivision()
{
    QMessageBox::information(this, "display", "剖分立方体算法");
    this->Filtering();
    this->recursive_dividing_cube_->SetInputData(filter_data_);
    this->recursive_dividing_cube_->SetValue(500); // 设置等值面的阈值
    this->recursive_dividing_cube_->SetDistance(1);
    this->recursive_dividing_cube_->SetIncrement(2);
    this->recursive_dividing_cube_->Update();

    vtkNew<vtkOutlineFilter> outlineFilter;
    outlineFilter->SetInputData(filter_data_);
    //建立映射
    this->cut_style_->data_ = recursive_dividing_cube_->GetOutput();
    //this->cut_style_->output_ = recursive_dividing_cube_->GetOutputPort();
    this->poly_mapper_->SetInputData(recursive_dividing_cube_->GetOutput());
    this->poly_mapper_->ScalarVisibilityOff();
    this->actor_->SetMapper(poly_mapper_);

    this->ui.widget_polygon_3d_->setRenderWindow(renwin_polygon_3d_);
    this->renwin_polygon_3d_->Render();
}

void VisualDicom::DisplayForContourFilter()
{
    QMessageBox::information(this, "display", "提取等值面算法");
    this->Filtering();
    //vtkNew<vtkOutlineFilter> outlineFilter;
    //outlineFilter->SetInputData(filter_data_);
    //outlineFilter->Update();
    this->contour_filter_->SetInputConnection(reader_->GetOutputPort());
    this->contour_filter_->SetValue(0, 200); // 设置等值面的阈值
    this->contour_filter_->Update();

    this->deci_->SetInputConnection(contour_filter_->GetOutputPort());
    this->deci_->SetTargetReduction(.3);
    this->deci_->PreserveTopologyOn();

    //this->smoother_->SetInputConnection(deci_->GetOutputPort());
    //this->smoother_->SetNumberOfIterations(50);//Specify the number of iterations for Laplacian smoothing

    //vtkAlgorithmOutput* output = smoother_->GetOutputPort();
    //this->normals_->SetInputConnection(output);
    //this->normals_->FlipNormalsOn();

    //建立映射
    this->poly_mapper_->SetInputConnection(deci_->GetOutputPort());
    this->poly_mapper_->ScalarVisibilityOff();
    this->actor_->SetMapper(poly_mapper_);
    this->actor_->GetProperty()->SetColor(1.0, 1.0, 0.0);
    this->actor_->GetProperty()->SetRepresentationToWireframe();

    this->render_polygon_3d_->AddActor(actor_);

    this->ui.widget_polygon_3d_->setRenderWindow(renwin_polygon_3d_);

    //this->cut_style_->selectedActor = actor_;
    //this->cut_style_->selectedMapper = poly_mapper_;
    //this->cut_style_->renderer = render_polygon_3d_;
    //this->cut_style_->renwin = renwin_polygon_3d_;

    //this->cut_style_->Data = smoother_->GetOutput();
    //this->cut_style_->bufs.Clear();
    //this->cut_style_->bufs.Enqueue(this->cut_style_->output = output);
    this->renwin_polygon_3d_->Render();
}

void VisualDicom::DisplayForRaycast() 
{
    QMessageBox::information(this, "display", "光线投射算法");
    this->Filtering();
    this->mapper_->SetInputData(filter_data_);
    this->mapper_->SetBlendModeToComposite();
    this->ui.widget_3d->setRenderWindow(renwin_[k3D]);
    this->renwin_[k3D]->Render();
}

void VisualDicom::DisplayForMaximumDensityProjection()
{
    QMessageBox::information(this, "display", "最大密度投影算法");
    this->Filtering();
    //设置不透明度
    this->mapper_->SetInputData(filter_data_);
    this->mapper_->SetBlendModeToMaximumIntensity();
    this->mapper_->SetAutoAdjustSampleDistances(0);
    this->mapper_->SetSampleDistance(0.2);

    this->ui.widget_3d->setRenderWindow(renwin_[k3D]);
    this->renwin_[k3D]->Render();
}

void VisualDicom::GaussionFilter()
{
    QMessageBox::information(this, "filter", "高斯平滑");
    vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter = vtkSmartPointer<vtkImageGaussianSmooth>::New();
    gaussianSmoothFilter->SetInputData(reader_->GetOutput());
    gaussianSmoothFilter->SetDimensionality(3);
    gaussianSmoothFilter->SetRadiusFactor(res_filters->kernelsize_gaussion);
    gaussianSmoothFilter->SetStandardDeviation(res_filters->sigma_gaussion);
    gaussianSmoothFilter->Update();
    this->filter_data_ = gaussianSmoothFilter->GetOutput();
}

void VisualDicom::BilateralFilter()
{
    QMessageBox::information(this, "filter", "双边滤波");
    vtkSmartPointer<vtkImageGaussianSmooth> gaussianFilter = vtkSmartPointer<vtkImageGaussianSmooth>::New();
    gaussianFilter->SetInputData(data_);        // 6. 设置滤波器输入
    // 7. 设置滤波器参数
    gaussianFilter->SetStandardDeviation(res_filters->sigma_bilateral);  // 设置标准差
    // 8. 创建双边滤波器
    vtkSmartPointer<vtkImageAnisotropicDiffusion3D> bilateralFilter = vtkSmartPointer<vtkImageAnisotropicDiffusion3D>::New();
    // 9. 设置滤波器输入
    bilateralFilter->SetInputConnection(gaussianFilter->GetOutputPort());
    // 10. 设置滤波器参数
    bilateralFilter->SetNumberOfIterations(res_filters->iterations); // 设置迭代次数
    bilateralFilter->SetDiffusionThreshold(res_filters->threshold); // 设置扩散阈值

    // 11. 执行滤波操作
    bilateralFilter->Update();
    this->filter_data_ = bilateralFilter->GetOutput();
}

void VisualDicom::MidValueFilter()
{
    QMessageBox::information(this, "filter", "中值滤波");
    vtkSmartPointer<vtkImageMedian3D> medianFilter = vtkSmartPointer<vtkImageMedian3D>::New();
    medianFilter->SetInputData(reader_->GetOutput());
    medianFilter->SetKernelSize(res_filters->kernelsize_mid, res_filters->kernelsize_mid, res_filters->kernelsize_mid);
    medianFilter->Update();
    this->filter_data_ = medianFilter->GetOutput();
}

void VisualDicom::RegionalGrowthCut()
{
    QMessageBox::information(this, "visual", "区域生长算法");
    //vtkSmartPointer<vtkImageData> OutPutMaskData = thresholdFilter->GetOutput();//分割并且替换数值后的输出结果
}

void VisualDicom::ThresholdCut()
{
    QMessageBox::information(this, "visual", "阈值分割算法");
    //std::call_once(g_setwin,
    //    []
    //    {
    //        g_window = std::make_unique<Threshold>(new Threshold);
    //        g_window->setWindowModality(Qt::NonModal);
    //    });
    //g_window->show();
    //g_window->setVisible(true);
    //res_threshold = g_window->Result();
    //*res_threshold = { 0 , 200 , false };

    //this->threshold_->setWindowModality(Qt::ApplicationModal);
    //this->threshold_->show();
    //this->threshold_->setModal(true);
    //this->threshold_->show();
    res_threshold = threshold_->Result();
    printf("Threshold:%d %d %s\n", res_threshold->low, res_threshold->high, res_threshold->is_inside ? "inside" : "outside");
    vtkNew<vtkImageHistogramStatistics> accumulateFilter;
    accumulateFilter->SetInputData(filter_data_);
    accumulateFilter->Update();
    //double res[2];
    double* range = accumulateFilter->GetAutoRange();
    printf("imagedata range:%f %f\n", range[0], range[1]);
    vtkSmartPointer<vtkImageThreshold> thresholdFilter = vtkSmartPointer<vtkImageThreshold>::New();
    thresholdFilter->SetInputData(filter_data_);//输入vtkImageData
    thresholdFilter->ThresholdBetween(res_threshold->low, res_threshold->high);//设置分割阈值
    //thresholdFilter->ReplaceInOn();//阈值内的像素值替换
    //thresholdFilter->ReplaceOutOn();//阈值外的像素值替换
    res_threshold->is_inside ? thresholdFilter->ReplaceOutOn() : thresholdFilter->ReplaceInOn();
    res_threshold->is_inside ? thresholdFilter->SetOutValue(0) : thresholdFilter->SetInValue(0);
    //thresholdFilter->SetInValue(res_threshold->is_inside);//阈值内像素值全部替换成1
    //thresholdFilter->SetOutValue(!res_threshold->is_inside);//阈值外像素值全部替换成0
    thresholdFilter->Update();//触发管道更新
    //this->filter_data_ = thresholdFilter->GetOutput();
    this->mapper_->SetInputData(thresholdFilter->GetOutput());
    this->mapper_->SetBlendModeToComposite();
    this->ui.widget_3d->setRenderWindow(renwin_[k3D]);
    this->renwin_[k3D]->Render();
}

void VisualDicom::BoxWidgetCut()
{
    QMessageBox::information(this, "visual", "包围盒裁剪");
    static bool is_on = false;
    is_on ? this->box_widget_->Off() : this->box_widget_->On();
    is_on = !is_on;
    this->renwin_[k3D]->Render();
}

void VisualDicom::DrawlineCut()
{
    QMessageBox::information(this, "visual", "矩形裁剪");
}


