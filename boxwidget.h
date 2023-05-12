#pragma once
#include <vtkCommand.h>
#include <vtkVolume.h>
#include <vtkPlanes.h>
#include <vtkTransform.h>
#include <vtkBoxWidget2.h>
#include <vtkBoxRepresentation.h>
#include <vtkGPUVolumeRayCastMapper.h>


class BoxWidget3DCallback final : public vtkCommand 
{
public:
    static BoxWidget3DCallback* New() { return new BoxWidget3DCallback; }
    vtkTypeMacro(BoxWidget3DCallback, vtkCommand);

    BoxWidget3DCallback() = default;
    ~BoxWidget3DCallback() = default;
    vtkVolume* GetVolume() const { return volume_; }
    void SetVolume(vtkVolume* t_volume) { this->volume_ = t_volume; }
    void Execute(vtkObject* caller, unsigned long eventId, void* callData) override;

private:
    vtkVolume* volume_ = {};
    vtkNew<vtkPlanes> planes_{};
    vtkNew<vtkTransform> transform_{};
};

void BoxWidget3DCallback::Execute(vtkObject* caller, unsigned long, void*) 
{
    auto* const boxWidget = vtkBoxWidget2::SafeDownCast(caller);
    auto* const  boxRepresentation =
        vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation());
    boxRepresentation->SetInsideOut(1);
    boxRepresentation->GetPlanes(planes_);
    volume_->GetMapper()->SetClippingPlanes(planes_);
}


class BoxWidget3D : public vtkBoxWidget2 
{
public:
    static BoxWidget3D* New() { return new BoxWidget3D; }
    vtkTypeMacro(BoxWidget3D, vtkBoxWidget2);

    BoxWidget3D()
    {
        CreateDefaultRepresentation();
        GetRepresentation()->SetPlaceFactor(1);
    }
    ~BoxWidget3D() = default;
};
