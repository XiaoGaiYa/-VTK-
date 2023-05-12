#pragma once
#include <vtkInteractorStyleDrawPolygon.h>
#include <vtkInteractorStyleRubberBandPick.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPoints.h>
#include <vtkPolygon.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkAreaPicker.h>
#include <vtkClipPolyData.h>
#include <vtkDataSetMapper.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkAlgorithmOutput.h>


class QVTKOpenGLNativeWidget;
class vtkGenericOpenGLRenderWindow;
template<typename Type>
class RingBuf
{
public:
	explicit RingBuf(size_t capacity) : capa_(capacity)
	{
		this->datas_.resize(capacity);
	}

	~RingBuf() { this->datas_.clear(); }

	void Enqueue(Type* data)
	{
		// 将新数据写入缓冲区
		datas_[indx_] = data;
		// 当前索引后移
		indx_ = (indx_ + 1) % capa_;
		// 如果当前索引等于头索引，说明已达到上限，需要移动头索引
		if (indx_ == head_)
		{
			head_ = (head_ + 1) % capa_;
		}
		// 尾索引始终指向下一个空闲的位置
		tail_ = (indx_ + 1) % capa_;
	}

	Type* BackTrace()
	{
		return this->datas_[indx_];
	}

	bool TryUndo()
	{
		if (!IsCanUndo()) return false;
		return (indx_ = (indx_ - 1 + capa_) % capa_);
	}

	bool TryRedo()
	{
		if (!IsCanRedo()) return false;
		return (indx_ = (indx_ + 1 + capa_) % capa_);
	}

	inline void Clear() { datas_.clear(); }
	inline bool IsCanUndo() const { return indx_ != head_; }
	inline bool IsCanRedo() const { return indx_ != tail_; }

private:
	std::vector<vtkSmartPointer<Type>> datas_;
	size_t head_ = 0;
	size_t tail_ = 0;
	size_t indx_ = 0;
	size_t capa_ = 0;
};

class InteractorStyle : public vtkInteractorStyleRubberBandPick //重载vtkInteractorStyleRubberBandPick
{
public:
	static InteractorStyle* New();
	vtkTypeMacro(InteractorStyle, vtkInteractorStyleRubberBandPick);

	InteractorStyle() : bufs_(10)//buf(10), planess(10)
	{

	}

	inline void Clear() { this->bufs_.Clear(); }

	virtual void OnKeyDown() override
	{
		vtkInteractorStyleRubberBandPick::OnKeyDown();
		std::string key = this->Interactor->GetKeySym();
		if (key == "r" || key == "R") this->is_run_ = !this->is_run_;
		if (!this->Interactor->GetControlKey()) return;
		if (key == "z" || key == "Z")
			if (bufs_.TryUndo())//buf.TryUndo() || planess.TryUndo())
			{
				this->is_mod_ = true;
			}
		if (key == "y" || key == "Y")
			if (bufs_.TryRedo())//buf.TryRedo() || planess.TryRedo())
			{
				this->is_mod_ = true;
			}
		if (is_mod_)
		{
			this->ReRender(bufs_.BackTrace());
		}
		this->is_mod_ = false;
	}

	virtual void OnLeftButtonUp() override//重写左键按下消息
	{
		if (!is_run_) return vtkInteractorStyleRubberBandPick::OnLeftButtonUp();
		vtkPlanes* planes = static_cast<vtkAreaPicker*>(this->GetInteractor()->GetPicker())->GetFrustum();//获得鼠标框选矩形

		vtkClipPolyData* clipper = vtkClipPolyData::New();//裁剪polydata
		this->bufs_.Enqueue(data_);
		clipper->SetInputData(data_);
		clipper->SetClipFunction(planes);//!!!!很重要的一步，添加自定义隐函数
		clipper->GenerateClipScalarsOn();
		clipper->GenerateClippedOutputOn();
		clipper->SetValue(is_inside_ ? -0.5 : 0.5);

		this->mapper_->SetInputConnection(clipper->GetOutputPort());
		this->actor_->SetMapper(mapper_);

		this->renderer_->AddActor(actor_);

		this->data_ = clipper->GetOutput();
		this->renwin_->AddRenderer(renderer_);

		this->renwin_->Render();
		this->is_run_ = false;
		vtkInteractorStyleRubberBandPick::OnLeftButtonUp();
	}

	vtkSmartPointer<vtkGenericOpenGLRenderWindow> renwin_;
	vtkSmartPointer<vtkPolyData> data_;
	RingBuf<vtkPolyData> bufs_;
	vtkSmartPointer<vtkPolyDataMapper> mapper_;
	vtkSmartPointer<vtkActor> actor_;
	vtkSmartPointer<vtkRenderer> renderer_;
	bool is_run_ = false;
	bool is_mod_ = false;
	bool is_inside_ = false;

private:
	void ReRender(vtkPolyData* clip)
	{
		this->mapper_->SetInputData(clip);
		this->actor_->SetMapper(mapper_);
		this->renderer_->AddActor(actor_);
		this->renwin_->AddRenderer(renderer_);
		this->renwin_->Render();
	}
};
vtkStandardNewMacro(InteractorStyle);


class RegionCutting : public vtkInteractorStyleDrawPolygon
{
public:
	static RegionCutting* New();
	vtkTypeMacro(RegionCutting, vtkInteractorStyleDrawPolygon);


	virtual void OnLeftButtonUp()//重写左键按下消息
	{
		// Forward events  
		vtkInteractorStyleDrawPolygon::OnLeftButtonUp();

		vtkPlanes* frustum = static_cast<vtkAreaPicker*>(this->GetInteractor()->GetPicker())->GetFrustum();//获得鼠标框选矩形

		vtkClipPolyData* clipper = vtkClipPolyData::New();//裁剪polydata
		clipper->SetInputData(this->Data);
		clipper->SetClipFunction(frustum);//!!!!很重要的一步，添加自定义隐函数
		clipper->GenerateClipScalarsOn();
		clipper->GenerateClippedOutputOn();
		clipper->SetValue(0.5);
		this->selectedMapper->SetInputConnection(clipper->GetOutputPort());
		this->selectedMapper->ScalarVisibilityOff();
		this->selectedActor->SetMapper(selectedMapper);
		this->selectedActor->GetProperty()->SetColor(1.0, 0.0, 0.0); //(R,G,B)  
		this->selectedActor->GetProperty()->SetRepresentationToWireframe();

		renderer->SetBackground(0.6, 0.8, 0.8); // Blue
		renderer->AddActor(selectedActor);
		renderer->SetViewport(0.5, 0, 1, 1);

		this->Interactor->GetRenderWindow()->AddRenderer(renderer);
		this->GetInteractor()->GetRenderWindow()->Render();
	}

	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkPolyData> Data;
	vtkSmartPointer<vtkDataSetMapper> selectedMapper;
	vtkSmartPointer<vtkActor> selectedActor;

private:
	bool is_run = false;
};

