//
// Created by fangl on 2024/3/22.
//

#pragma once


static constexpr int kWindowWidth = 1366;
static constexpr int kWindowHeight = 768;

static constexpr float kMaxFrameTime = 0.5f;
static constexpr int kMaxMaterialsNumber = 200;

static constexpr int kMaxGlobalPoolSetSize = 1024;
static constexpr int kMaxBindlessResources = 1024;
static constexpr int kBindlessTextureBinding = 0;

static const std::string kDefaultPipelineCacheDirectory =
    "./PipelineCache";
static const std::string kDefaultPipelineCachePath =
    "./PipelineCache/pipeline_cache";

