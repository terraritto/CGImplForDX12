#pragma once
#include "./Base/App/AppBase.h"
#include "./Base/Graphics/Buffer/ConstantBuffer.h"
#include "./Base/Graphics/Geometry/Model.h"
#include "./Base/Graphics/RootSignature.h"
#include "./Base/Graphics/PS/GraphicsPipelineState.h"
#include "./Base/Graphics/Resource/Texture/Texture.h"

class LitSphereApp : public AppBase
{
protected:
	// AppBase override Interface
	virtual bool OnInitialize() override;
	virtual bool OnPreInitialize() override;
	virtual bool OnTerminate() override;
	virtual void OnUpdate(double deltaTime) override;
	virtual void OnRender() override;

protected:
	// DX Base
	RootSignature m_rootSignature;
	GraphicsPipelineState m_pso;
	std::weak_ptr<CommandQueue> m_graphicsQueue;
	WaitPoint m_frameWaitPoint;

	// Model
	Model m_model;

	// Transform
	ConstantBuffer m_transformBuffer;

	// Texture
	std::vector<std::weak_ptr<Texture>> m_textures;
};