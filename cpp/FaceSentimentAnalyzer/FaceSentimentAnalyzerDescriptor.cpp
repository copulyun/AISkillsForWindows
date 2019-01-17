﻿// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"
#include "FaceSentimentAnalyzerConst.h"
#include "FaceSentimentAnalyzerDescriptor.h"
#include "FaceSentimentAnalyzerSkill.h"

using namespace winrt::Microsoft::AI::Skills::SkillInterfacePreview;

namespace winrt::Contoso::FaceSentimentAnalyzer::implementation
{
    // {678BD455-4190-45D3-B5DA-41543283C092}
    const guid FaceSentimentAnalyzerId = guid(0x678bd455, 0x4190, 0x45d3, { 0xb5, 0xda, 0x41, 0x54, 0x32, 0x83, 0xc0, 0x92 });

    //
    // FaceSentimentAnalyzerDescriptor constructor
    //
    FaceSentimentAnalyzerDescriptor::FaceSentimentAnalyzerDescriptor()
    {
        m_version = SkillVersion::Create(
            0,  // major version
            1,  // minor version
            L"Contoso Developer", // Author name 
            L"Contoso Publishing" // Publisher name
        );

        m_inputSkillDesc = single_threaded_vector<ISkillFeatureDescriptor>();
        m_outputSkillDesc = single_threaded_vector<ISkillFeatureDescriptor>();

        // Describe input feature
        m_inputSkillDesc.Append(
            SkillFeatureImageDescriptor::Create(
                SKILL_INPUTNAME_IMAGE,
                L"the input image onto which the sentiment analysis runs",
                true, // isRequired (since this is an input, it is required to be bound before the evaluation occurs)
                -1, // width
                -1, // height
                -1, // maxDimension
                Windows::Graphics::Imaging::BitmapPixelFormat::Nv12,
                Windows::Graphics::Imaging::BitmapAlphaMode::Ignore)
        );

        // Describe first output feature
        m_outputSkillDesc.Append(
            SkillFeatureTensorDescriptor::Create(
                SKILL_OUTPUTNAME_FACERECTANGLE,
                L"a face bounding box in relative coordinates (left, top, right, bottom)",
                false, // isRequired (since this is an output, it automatically get populated after the evaluation occurs)
                single_threaded_vector<int64_t>({ 4 }).GetView(), // tensor shape
                SkillElementKind::Float)
        );

        // Describe second output feature
        m_outputSkillDesc.Append(
            SkillFeatureTensorDescriptor::Create(
                SKILL_OUTPUTNAME_FACESENTIMENTSCORES,
                L"the prediction score for each class",
                false, // isRequired (since this is an output, it automatically get populated after the evaluation occurs)
                single_threaded_vector<int64_t>({ 1, 8 }).GetView(), // tensor shape
                SkillElementKind::Float)
        );
    }

    //
    // Retrieves a list of supported ISkillExecutionDevice to run the skill logic on
    //
    Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IVectorView<ISkillExecutionDevice>> FaceSentimentAnalyzerDescriptor::GetSupportedExecutionDevicesAsync()
    {
        m_devices = single_threaded_vector<ISkillExecutionDevice>();
        m_devices.Append(SkillExecutionDeviceCPU::Create());
        auto gpuDevices = SkillExecutionDeviceGPU::GetAvailableGpuExecutionDevices();
        for (auto iter : gpuDevices)
        {
            // Expose only D3D12 devices since WinML supports only those
            if (iter.as<SkillExecutionDeviceGPU>().MaxSupportedFeatureLevel() >= D3DFeatureLevelKind::D3D_FEATURE_LEVEL_12_0)
            {
                m_devices.Append(iter);
            }
        }
        co_await resume_background();
        return m_devices.GetView();
    }

    //
    // Factory method for instantiating and initializing the skill
    //
    Windows::Foundation::IAsyncOperation<ISkill> FaceSentimentAnalyzerDescriptor::CreateSkillAsync(ISkillExecutionDevice const executionDevice)
    {
        co_await resume_background();
        auto desc = this->operator winrt::Windows::Foundation::IInspectable().as<ISkillDescriptor>();
        auto skill = FaceSentimentAnalyzerSkill::CreateAsync(desc, executionDevice).get();
        return skill;
    }

    winrt::guid FaceSentimentAnalyzerDescriptor::Id()
    {
        return FaceSentimentAnalyzerId;
    }

    hstring FaceSentimentAnalyzerDescriptor::Name()
    {
        return L"FaceSentimentAnalyzer";
    }
    
    hstring FaceSentimentAnalyzerDescriptor::Description()
    {
        return L"Finds a face in the image and infers its predominant sentiment from a set of 8 possible labels";
    }

    SkillVersion FaceSentimentAnalyzerDescriptor::Version()
    {
        return m_version;
    }

    Windows::Foundation::Collections::IVectorView<ISkillFeatureDescriptor> FaceSentimentAnalyzerDescriptor::InputFeatureDescriptors()
    {
        return m_inputSkillDesc.GetView();
    }

    Windows::Foundation::Collections::IVectorView<ISkillFeatureDescriptor> FaceSentimentAnalyzerDescriptor::OutputFeatureDescriptors()
    {
        return m_outputSkillDesc.GetView();
    }

    Windows::Foundation::Collections::IMapView<hstring, hstring> FaceSentimentAnalyzerDescriptor::Metadata()
    {
        return single_threaded_map<hstring, hstring>().GetView();
    }

    
}
