#pragma once
#include <vtkCommand.h>
#include <vtkResliceCursorWidget.h>
#include <vtkInteractorStyleImage.h>
#include <vtkResliceCursorActor.h>
#include <vtkImagePlaneWidget.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkInteractorStyleDrawPolygon.h>
#include <vtkPlaneSource.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTextActor.h>
#include <vtkSelectPolyData.h>
#include <vtkPolyDataToImageStencil.h>


class ImageInteractorStyle : public vtkInteractorStyleImage
{
public:
    static ImageInteractorStyle* New()
    {
        return new ImageInteractorStyle;
    }
    vtkTypeMacro(ImageInteractorStyle, vtkInteractorStyleImage);
    
    void SetImageViewer(vtkImageViewer2* imageViewer) 
    {
        viewer_ = imageViewer;
        minslice_ = imageViewer->GetSliceMin();
        maxslice_ = imageViewer->GetSliceMax();
        slice_ = maxslice_ / 2;
    }

    void SetStatusMapper(vtkTextMapper* statusMapper) 
    {
        mapper_ = statusMapper;
    }

    void SetSliceIndex(int idx)
    {
        if (idx < minslice_ || idx > maxslice_) return;
        this->viewer_->SetSlice(slice_ = idx);
        char buf[64];
        int n = snprintf(buf, sizeof(buf), "SliceNumber:%d/%d", slice_, maxslice_);
        buf[n] = 0;
        //cout << buf << endl;
        mapper_->SetInput(buf);
        viewer_->Render();
    }

protected:
	void MoveSliceForward() 
    {
		if (slice_ < maxslice_) 
        {
			viewer_->SetSlice(++slice_);
            char buf[64];
            int n = snprintf(buf, sizeof(buf), "SliceNumber:%d/%d", slice_, maxslice_);
            buf[n] = 0;
			mapper_->SetInput(buf);
			viewer_->Render();
		}
	}

	void MoveSliceBackward() 
    {
		if (slice_ > minslice_) 
        {
			viewer_->SetSlice(--slice_);
            char buf[64];
            int n = snprintf(buf, sizeof(buf), "SliceNumber:%d/%d", slice_, maxslice_);
            buf[n] = 0;
            mapper_->SetInput(buf);
			viewer_->Render();
		}
	}

    virtual void OnKeyDown() override
    {
        std::string key = this->GetInteractor()->GetKeySym();
        if (key.compare("Up") == 0) 
        {
            MoveSliceForward();
        }
        else if (key.compare("Down") == 0) 
        {
            MoveSliceBackward();
        }
        vtkInteractorStyleImage::OnKeyDown();
    }

protected:
    vtkImageViewer2* viewer_;
    vtkTextMapper* mapper_;
    VisualDicom* parent_;
    int slice_;
    int minslice_;
    int maxslice_;
};

class DrawlineInteractorStyle : public vtkInteractorStyleDrawPolygon
{
public:
    enum CutStyle { Inside = 0 , Outside };
    static DrawlineInteractorStyle* New()
    {
        return new DrawlineInteractorStyle;
    }
    vtkTypeMacro(DrawlineInteractorStyle, vtkInteractorStyleDrawPolygon);

    void SetInputData(vtkImageData* data)
    {
        vtkNew<vtkOutlineFilter> outlineFilter;
        outlineFilter->SetInputData(data);
        this->polydata_ = outlineFilter->GetOutput();
    }
    vtkPolyData* GetOutput() const { return polydata_; }

    void SetMapper(vtkGPUVolumeRayCastMapper* mapper) { this->mapper_ = mapper; }
    void SetRenderWindow(vtkRenderWindow* renwin) { this->renwin_ = renwin; }
    void SetCutting(bool flag) { this->is_default_style_ = !flag; }
    void SetCutStyle(CutStyle flag) { this->is_inside_ = flag; }

protected:
    virtual void OnLeftButtonDown() override
    {
        if (is_default_style_) return vtkInteractorStyleDrawPolygon::OnLeftButtonDown();
        if (is_started_) return;
        cout << __FUNCTION__ << endl;
        this->points_ = vtkSmartPointer<vtkPoints>::New();
        this->Modified();
        this->GetInteractor()->GetPicker()->Pick(this->GetInteractor()->GetEventPosition()[0],
            this->GetInteractor()->GetEventPosition()[1],
            0, this->GetDefaultRenderer());
        this->points_->InsertNextPoint(this->GetInteractor()->GetPicker()->GetPickPosition());
        this->is_started_ = true;
    }

    virtual void OnMouseMove() override
    {
        if (is_default_style_) return vtkInteractorStyleDrawPolygon::OnMouseMove();
        if (!is_started_) return;
        cout << __FUNCTION__ << endl;
        this->GetInteractor()->GetPicker()->Pick(this->GetInteractor()->GetEventPosition()[0],
            this->GetInteractor()->GetEventPosition()[1],
            0, this->GetDefaultRenderer());
        this->points_->InsertNextPoint(this->GetInteractor()->GetPicker()->GetPickPosition());
    }

    virtual void OnLeftButtonUp() override
    {
        if (is_default_style_) return vtkInteractorStyleDrawPolygon::OnMouseMove();
        if (!is_started_) return;
        cout << __FUNCTION__ << endl;
        vtkNew<vtkSelectPolyData> selectPolyData;
        selectPolyData->SetInputData(polydata_);
        selectPolyData->SetLoop(points_);
        selectPolyData->GenerateUnselectedOutputOn();
        selectPolyData->SetSelectionModeToSmallestRegion();
        selectPolyData->Update();
        
        vtkAlgorithmOutput* data = is_inside_ ? selectPolyData->GetOutputPort() : selectPolyData->GetUnselectedOutputPort();
        this->is_started_ = false;

        this->mapper_->SetInputConnection(data);
        this->renwin_->Render();
    }

    virtual void OnKeyDown() override
    {
        //cout << __FUNCTION__ << endl;
        //if (is_default_style_) return vtkInteractorStyle::OnKeyDown();
        //bool ctrl = this->GetInteractor()->GetControlKey();
        //std::string key = this->GetInteractor()->GetKeySym();
        //this->is_default_style_ = !(ctrl && key.compare("r"));
        //if (key.compare("Up") == 0)
        //{
        //    MoveSliceForward();
        //}
        //else if (key.compare("Down") == 0)
        //{
        //    MoveSliceBackward();
        //}
        vtkInteractorStyle::OnKeyDown();
    }

protected:
    vtkSmartPointer<vtkPoints> points_;
    vtkSmartPointer<vtkPolyData> polydata_;
    vtkSmartPointer<vtkGPUVolumeRayCastMapper> mapper_;
    vtkSmartPointer<vtkRenderWindow> renwin_;
    CutStyle is_inside_ = Inside;
    bool is_started_ = false;
    bool is_default_style_ = true;
};

