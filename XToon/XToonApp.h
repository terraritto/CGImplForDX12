#pragma once
#include "./Base/App/AppBase.h"
#include "./Base/Graphics/Buffer/ConstantBuffer.h"
#include "./Base/Graphics/Geometry/Model.h"
#include "./Base/Graphics/RootSignature.h"
#include "./Base/Graphics/PS/GraphicsPipelineState.h"
#include "./Base/Graphics/Resource/Texture/Texture.h"

class XToonApp : public AppBase
{
protected:
	// AppBase override Interface
	virtual bool OnInitialize() override;
	virtual bool OnPreInitialize() override;
	virtual bool OnTerminate() override;
	virtual void OnUpdate(double deltaTime) override;
	virtual void OnRender() override;

protected:
	struct alignas(256) TransformParam
	{
		XMMATRIX m_world;
		XMMATRIX m_view;
		XMMATRIX m_proj;
		XMFLOAT4 m_directional;
		XMFLOAT4 m_cameraPos;
		float m_exponent;
		bool m_useHighlight;
	};

protected:
	// DX Base
	RootSignature m_rootSignature;
	GraphicsPipelineState m_pso;
	std::weak_ptr<CommandQueue> m_graphicsQueue;
	WaitPoint m_frameWaitPoint;

	// Model
	Model m_model;

	// Transform
	TransformParam m_transformParameter;
	ConstantBuffer m_transformBuffer;

	// Texture
	std::vector<std::weak_ptr<Texture>> m_textures;
};