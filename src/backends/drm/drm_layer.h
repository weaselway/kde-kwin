/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2022 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once
#include "core/colorpipeline.h"
#include "core/outputlayer.h"
#include "drm_plane.h"

#include <deque>
#include <memory>
#include <optional>

namespace KWin
{

class SurfaceItem;
class DrmFramebuffer;
class GLTexture;
class DrmPipeline;
class DrmOutput;

class DrmOutputLayer : public OutputLayer
{
public:
    explicit DrmOutputLayer(BackendOutput *output, OutputLayerType type);
    explicit DrmOutputLayer(BackendOutput *output, OutputLayerType type, int zpos, int minZpos, int maxZpos);
    virtual ~DrmOutputLayer();
};

class DrmPipelineLayer : public DrmOutputLayer
{
public:
    explicit DrmPipelineLayer(DrmPlane *plane);
    explicit DrmPipelineLayer(DrmPlane::TypeIndex type);

    FormatModifierMap supportedDrmFormats() const override;
    QList<QSize> recommendedSizes() const override;
    FormatModifierMap supportedAsyncDrmFormats() const override;

    virtual std::shared_ptr<DrmFramebuffer> currentBuffer() const = 0;

    DrmPlane *plane() const;

    /**
     * Identifies the contents of currentBuffer(). A commit that puts them on
     * the screen reports the serial back through DrmPlane::setPresentedFrameSerial()
     */
    uint64_t frameSerial() const;
    /**
     * What differs between currentBuffer() and the frame that is on the screen,
     * in buffer coordinates; std::nullopt if that's not known.
     * Frames that are still on their way to the screen count as not presented,
     * so this may be more than necessary, but never less
     */
    std::optional<Region> bufferDamage() const;

protected:
    /**
     * To be called whenever currentBuffer() gets new contents, with what changed
     * relative to the previous contents, or std::nullopt if that's not known
     */
    void addFrameDamage(const std::optional<Region> &damage);

    DrmPipeline *pipeline() const;
    DrmGpu *gpu() const;
    DrmOutput *drmOutput() const;

    DrmPlane *m_plane = nullptr;

private:
    struct FrameDamage
    {
        uint64_t serial;
        std::optional<Region> damage;
    };
    // nothing is known about what's on the screen before the first frame
    std::deque<FrameDamage> m_frameDamage = {FrameDamage{.serial = 1, .damage = std::nullopt}};
    uint64_t m_frameSerial = 1;
};

}
