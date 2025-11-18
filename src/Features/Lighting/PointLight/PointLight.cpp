#include "PointLight.h"
#include <drawable/object3d/Object3d.h>

#include <sstream>
#include <cstdarg>

void PointLight::Initialize()
{
    pIcon_ = std::make_unique<Object3d>();
    pIcon_->Initialize();
    pIcon_->SetScale({ 0.05f, 0.05f, 0.05f });
    pIcon_->SetPointLight(this);
    pIcon_->SetModel(pIconModel_);

    auto& option = pIcon_->GetOption();
    option.lightingData->enableLighting = false;

    std::stringstream ss;
    ss << "PointLight##0x" << std::hex << this;
    pIcon_->SetName(ss.str());
}

void PointLight::Update()
{
    auto& option = pIcon_->GetOption();
    option.transform.translate = position_;
    pIcon_->Update();
}

void PointLight::Draw()
{
    pIcon_->Draw1F();
}

void PointLight::Finalize()
{
    pIcon_->Finalize();
}

PointLight& PointLight::SetIconModel(IModel* pModel)
{
    pIconModel_ = pModel;
    return *this;
}

PointLightForGPU PointLight::GetDataForGPU() const
{
    PointLightForGPU data;
    data.enablePointLight = enable_ ? 1 : 0;
    data.color = color_;
    data.position = position_;
    data.intensity = this->intensity_;
    return data;
}
