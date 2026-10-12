#include "XToonApp.h"
#include "./Base/App/ImguiManager.h"
#include "./Base/Graphics/GraphicsProxy.h"
#include "./Base/Util/Logger.h"
#include "./Base/Graphics/Resource/Texture/TextureManager.h"

bool XToonApp::OnInitialize()
{
    HRESULT hr = S_OK;

    auto device = GraphicsProxy::GetD3D12Device();

    m_graphicsQueue = GraphicsProxy::GetGraphicsQueue();

    // RootSignature
    {
        // Root Param
        std::vector<D3D12_ROOT_PARAMETER> params;
        params.push_back(PARAM_CBV{ D3D12_SHADER_VISIBILITY_ALL, 0, 0 });

        auto rangeType00 = RANGE_SRV{ 0,0 };
        params.push_back(PARAM_TABLE{ D3D12_SHADER_VISIBILITY_PIXEL, 1, &rangeType00 });

        std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
        samplers.push_back(PARAM_STATIC_SAMPLER{ D3D12_SHADER_VISIBILITY_PIXEL, 0, 0,
            PARAM_STATIC_SAMPLER::SamplerType::LinearClamp });

        // Root signature
        D3D12_ROOT_SIGNATURE_DESC desc;
        desc.NumParameters = static_cast<UINT>(params.size());
        desc.pParameters = params.data();
        desc.NumStaticSamplers = samplers.size();
        desc.pStaticSamplers = samplers.data();
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        if (m_rootSignature.Initialize(device, &desc) == false)
        {
            return false;
        }
    }

    // PSO
    {
        // input layout
        D3D12_INPUT_ELEMENT_DESC inputElements[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        // RasterizerState
        D3D12_RASTERIZER_DESC rsDesc;
        rsDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rsDesc.CullMode = D3D12_CULL_MODE_BACK;
        rsDesc.FrontCounterClockwise = TRUE;
        rsDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rsDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rsDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rsDesc.DepthClipEnable = TRUE;
        rsDesc.MultisampleEnable = FALSE;
        rsDesc.AntialiasedLineEnable = FALSE;
        rsDesc.ForcedSampleCount = 0;
        rsDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        // RT BlendState
        D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc =
        {
            FALSE, FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL
        };

        // BlendState
        D3D12_BLEND_DESC blendDesc;
        blendDesc.AlphaToCoverageEnable = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
        {
            blendDesc.RenderTarget[i] = rtBlendDesc;
        }

        // Shader
        m_pso.SetReloadPathVS(L"XToon/Resource/Shader/SimpleVS.hlsl");
        m_pso.SetReloadPathPS(L"XToon/Resource/Shader/SimplePS.hlsl");

        // Depth Stencil Operate
        D3D12_DEPTH_STENCILOP_DESC stencilOpDesc = {};
        stencilOpDesc.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        stencilOpDesc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        stencilOpDesc.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        stencilOpDesc.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

        // Depth Stencil
        D3D12_DEPTH_STENCIL_DESC stencilDesc = {};
        stencilDesc.DepthEnable = TRUE;
        stencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        stencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        stencilDesc.StencilEnable = FALSE;
        stencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        stencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        stencilDesc.FrontFace = stencilOpDesc;
        stencilDesc.BackFace = stencilOpDesc;

        // Sample
        DXGI_SAMPLE_DESC sampleDesc;
        sampleDesc.Count = 1;
        sampleDesc.Quality = 0;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_rootSignature.Get();
        desc.BlendState = blendDesc;
        desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState = rsDesc;
        desc.DepthStencilState = stencilDesc;
        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.InputLayout = { inputElements, _countof(inputElements) };
        desc.NumRenderTargets = 1;
        desc.RTVFormats[0] = m_swapChainFormat;
        desc.DSVFormat = m_depthStencilFormat;
        desc.SampleDesc = sampleDesc;


        if (!m_pso.Initialize(&desc))
        {
            return false;
        }
    }

    m_graphicsCommandList.Reset();
    auto command = m_graphicsCommandList.GetD3D12CommandList();

    // setup model
    std::string path = "XToon/../Resource/Mesh/Bunny/bunny.obj";
    if (!m_model.Initialize(command, path))
    {
        ELOGA("Error : Model::Initialize() Failed.");
        return false;
    }

    // setup transform   
    if (!m_transformBuffer.Initialize(sizeof(TransformParam)))
    {
        ELOGA("Error : TransformBuffer Init Failed.");
        return false;
    }

    // Camera
    constexpr auto fovY = DirectX::XMConvertToRadians(37.5f);
    m_camera.SetFov(37.5f);
    m_camera.SetAspect(static_cast<float>(m_width) / static_cast<float>(m_height));
    m_camera.SetPosition(DirectX::XMVectorSet(0.0f, 0.1f, 0.3f, 1.0f));

    // Texture
    {
        auto& manager = TextureManager::Instance();
        std::weak_ptr<Texture> textureRef;
        std::wstring path;

        path = L"XToonSilhouette.png";
        assert(manager.Load(command, path, textureRef));
        m_textures.push_back(textureRef);

        path = L"XToonHighlight.png";
        assert(manager.Load(command, path, textureRef));
        m_textures.push_back(textureRef);
    }

    // transform
    m_transformParameter.m_directional = XMFLOAT4{ 0,-1,-1,1 };
    m_transformParameter.m_exponent = 1.0;
    m_transformParameter.m_useHighlight = 1.0;

    command->Close();

    auto graphicsQueue = m_graphicsQueue.lock();
    if (!graphicsQueue) { return false; }

    ID3D12CommandList* commandLists[] = { command };
    graphicsQueue->Execute(1, commandLists);

    // create wait point
    m_frameWaitPoint = graphicsQueue->Signal();

    // wait to finish command complete
    GraphicsProxy::FrameSync();

    return true;
}

bool XToonApp::OnPreInitialize()
{
    m_deviceDesc.IsUseMeshlet = false;

    return true;
}

bool XToonApp::OnTerminate()
{
    m_model.Terminate();
    m_transformBuffer.Terminate();

    m_pso.Terminate();
    m_rootSignature.Terminate();

    auto& TextureManager = TextureManager::Instance();
    for (auto& textureRef : m_textures)
    {
        if (auto p = textureRef.lock())
        {
            TextureManager.RemoveRef(p->GetPath());
        }
    }

    return true;
}

void XToonApp::OnUpdate(double deltaTime)
{

    // Update Transform
    {
        // view/proj
        XMMATRIX view = m_camera.GetView();
        XMMATRIX proj = m_camera.GetProjection();

        // frustum
        m_transformParameter.m_view = view;
        m_transformParameter.m_proj = proj;

        // rotate
        double angle = GetGlobalRelativeTime();
        m_transformParameter.m_world = DirectX::XMMatrixRotationY(angle) * DirectX::XMMatrixIdentity();

        // camera
        m_transformParameter.m_cameraPos = XMFLOAT4{ m_camera.GetPosition().m128_f32 };

        // update transform
        m_transformBuffer.Swap();
        m_transformBuffer.Update(&m_transformParameter, sizeof(TransformParam));
    }

    // ImGui
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Param");
        auto framerate = ImGui::GetIO().Framerate;
        ImGui::Text("Framerate: %.3f ms", 1000.0f / framerate);
        ImGui::SliderFloat3("Directional", reinterpret_cast<float*>(&m_transformParameter.m_directional), -1.0f, 1.0f);
        ImGui::SliderFloat("Exponent", reinterpret_cast<float*>(&m_transformParameter.m_exponent), 1.0f, 
            m_transformParameter.m_useHighlight ? 1000.0f : 10.0f);
        ImGui::Checkbox("IsHighlight", &m_transformParameter.m_useHighlight);
        ImGui::End();
    }
}

void XToonApp::OnRender()
{
    uint32_t index = GetCurrentBackBufferIndex();

    auto graphicsQueue = m_graphicsQueue.lock();
    if (!graphicsQueue) { return; }

    m_graphicsCommandList.Reset();
    auto command = m_graphicsCommandList.GetD3D12CommandList();

    // set RootSignature/PSO
    command->SetGraphicsRootSignature(m_rootSignature.Get());
    command->SetPipelineState(m_pso.Get());

    // set viewport
    command->RSSetViewports(1, &m_viewport);

    // set scissor
    command->RSSetScissorRects(1, &m_scissorRect);

    // set resource barrier : Present -> RenderTarget
    m_colorTarget[index].ChangeState(command, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // Get handle
    auto handleRTV = m_colorTarget[index].GetCpuHandleRTV();
    auto handleDSV = m_depthTarget[index].GetCpuHandleDSV();

    // set RenderTarget
    command->OMSetRenderTargets(1, &handleRTV, FALSE, &handleDSV);

    // Clear RenderTarget
    const FLOAT clearColor[] = { 0.39f, 0.58f, 0.92f, 1.0f };
    command->ClearRenderTargetView(handleRTV, m_clearColor, 0, nullptr);

    // Clear Depth Stencil View
    command->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, m_clearDepth, m_clearStencil, 0, nullptr);

    // set descriptor heap table
    for (auto i = 0u; i < m_model.GetMeshCount(); i++)
    {
        auto mesh = m_model.GetMesh(i).lock();

        // vertex/index
        auto vb = mesh->GetPositions().GetView();
        auto ib = mesh->GetIndices().GetView();
        command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        command->IASetVertexBuffers(0, 1, &vb);
        command->IASetIndexBuffer(&ib);

        // transform
        command->SetGraphicsRootConstantBufferView(0, m_transformBuffer.GetGpuAddress());

        // texture
        int index = static_cast<int>(m_transformParameter.m_useHighlight);
        command->SetGraphicsRootDescriptorTable(1, m_textures[index].lock()->GetGpuHandleSRV());

        // draw
        command->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
    }

    // Imgui
    ImguiManager::Instance().Draw(command);

    // set resource barrier : RenderTarget -> Present
    m_colorTarget[index].ChangeState(command, D3D12_RESOURCE_STATE_PRESENT);

    // finish
    command->Close();

    // if it doesn't finish previous frame Command Execute, wait for finished.
    if (m_frameWaitPoint.IsValid())
    {
        graphicsQueue->Sync(m_frameWaitPoint);
    }

    // Execute
    ID3D12CommandList* commandLists[] = { command };
    graphicsQueue->Execute(1, commandLists);

    // create wait point
    m_frameWaitPoint = graphicsQueue->Signal();

    // display
    Present(0);

    // wait to finish command complete
    GraphicsProxy::FrameSync();
}
