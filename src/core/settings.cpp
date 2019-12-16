// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <utility>
#include "audio_core/dsp_interface.h"
#include "core/core.h"
#include "core/gdbstub/gdbstub.h"
#include "core/hle/kernel/shared_page.h"
#include "core/hle/service/hid/hid.h"
#include "core/hle/service/ir/ir_rst.h"
#include "core/hle/service/ir/ir_user.h"
#include "core/hle/service/mic_u.h"
#include "core/settings.h"
#include "video_core/renderer_base.h"
#include "video_core/video_core.h"

namespace Settings {

Values values = {};

void Apply() {
    GDBStub::SetServerPort(values.gdbstub_port);
    GDBStub::ToggleServer(values.use_gdbstub);

    VideoCore::g_hw_renderer_enabled = values.use_hw_renderer;
    VideoCore::g_shader_jit_enabled = values.use_shader_jit;
    VideoCore::g_hw_shader_enabled = values.use_hw_shader;
    VideoCore::g_hw_shader_accurate_mul = values.shaders_accurate_mul;
    VideoCore::g_enable_disk_shader_cache = values.enable_disk_shader_cache;

    if (VideoCore::g_renderer) {
        VideoCore::g_renderer->UpdateCurrentFramebufferLayout();
    }

    VideoCore::g_renderer_bg_color_update_requested = true;
    VideoCore::g_renderer_sampler_update_requested = true;
    VideoCore::g_renderer_shader_update_requested = true;

    Core::System& system = Core::System::GetInstance();
    if (system.IsPoweredOn()) {
        Core::DSP().SetSink(values.sink_id, values.audio_device_id);
        Core::DSP().EnableStretching(values.enable_audio_stretching);

        std::shared_ptr<Service::HID::Module> hid = Service::HID::GetModule(system);
        if (hid) {
            hid->ReloadInputDevices();
        }

        Service::SM::ServiceManager& sm = system.ServiceManager();

        std::shared_ptr<Service::IR::IR_USER> ir_user =
            sm.GetService<Service::IR::IR_USER>("ir:USER");
        if (ir_user) {
            ir_user->ReloadInputDevices();
        }

        std::shared_ptr<Service::IR::IR_RST> ir_rst = sm.GetService<Service::IR::IR_RST>("ir:rst");
        if (ir_rst) {
            ir_rst->ReloadInputDevices();
        }

        std::shared_ptr<Service::CAM::Module> cam = Service::CAM::GetModule(system);
        if (cam) {
            cam->ReloadCameraDevices();
        }

        Service::MIC::ReloadMic(system);
    }
}

template <typename T>
void LogSetting(const std::string& name, const T& value) {
    LOG_INFO(Config, "{}: {}", name, value);
}

void LogSettings() {
    using namespace Service::CAM;

    LOG_INFO(Config, "Citra Valentin Configuration:");
    LogSetting("use_cpu_jit", Settings::values.use_cpu_jit);
    LogSetting("use_hw_renderer", Settings::values.use_hw_renderer);
    LogSetting("use_hw_shader", Settings::values.use_hw_shader);
    LogSetting("shaders_accurate_mul", Settings::values.shaders_accurate_mul);
    LogSetting("use_shader_jit", Settings::values.use_shader_jit);
    LogSetting("resolution_factor", Settings::values.resolution_factor);
    LogSetting("use_frame_limit", Settings::values.use_frame_limit);
    LogSetting("frame_limit", Settings::values.frame_limit);
    LogSetting("min_vertices_per_thread", Settings::values.min_vertices_per_thread);
    LogSetting("pp_shader_name", Settings::values.pp_shader_name);
    LogSetting("filter_mode", Settings::values.filter_mode);
    LogSetting("render_3d", static_cast<int>(Settings::values.render_3d));
    LogSetting("factor_3d", Settings::values.factor_3d);
    LogSetting("layout_option", static_cast<int>(Settings::values.layout_option));
    LogSetting("swap_screen", Settings::values.swap_screen);
    LogSetting("dump_textures", Settings::values.dump_textures);
    LogSetting("custom_textures", Settings::values.custom_textures);
    LogSetting("enable_dsp_lle", Settings::values.enable_dsp_lle);
    LogSetting("enable_dsp_lle_multithread", Settings::values.enable_dsp_lle_multithread);
    LogSetting("sink_id", Settings::values.sink_id);
    LogSetting("enable_audio_stretching", Settings::values.enable_audio_stretching);
    LogSetting("audio_device_id", Settings::values.audio_device_id);
    LogSetting("mic_input_type", static_cast<int>(Settings::values.mic_input_type));
    LogSetting("mic_input_device", Settings::values.mic_input_device);
    LogSetting("camera_name[OuterRightCamera]", Settings::values.camera_name[OuterRightCamera]);
    LogSetting("camera_config[OuterRightCamera]", Settings::values.camera_config[OuterRightCamera]);
    LogSetting("camera_flip[OuterRightCamera]", Settings::values.camera_flip[OuterRightCamera]);
    LogSetting("camera_name[InnerCamera]", Settings::values.camera_name[InnerCamera]);
    LogSetting("camera_config[InnerCamera]", Settings::values.camera_config[InnerCamera]);
    LogSetting("camera_flip[InnerCamera]", Settings::values.camera_flip[InnerCamera]);
    LogSetting("camera_name[OuterLeftCamera]", Settings::values.camera_name[OuterLeftCamera]);
    LogSetting("camera_config[OuterLeftCamera]", Settings::values.camera_config[OuterLeftCamera]);
    LogSetting("camera_flip[OuterLeftCamera]", Settings::values.camera_flip[OuterLeftCamera]);
    LogSetting("use_virtual_sd", Settings::values.use_virtual_sd);
    LogSetting("is_new_3ds", Settings::values.is_new_3ds);
    LogSetting("region_value", Settings::values.region_value);
    LogSetting("use_gdbstub", Settings::values.use_gdbstub);
    LogSetting("gdbstub_port", Settings::values.gdbstub_port);
    LogSetting("sharper_distant_objects", Settings::values.sharper_distant_objects);
    LogSetting("ignore_format_reinterpretation", Settings::values.ignore_format_reinterpretation);
    LogSetting("custom_screen_refresh_rate", Settings::values.custom_screen_refresh_rate);
    LogSetting("screen_refresh_rate", static_cast<int>(Settings::values.screen_refresh_rate));
    LogSetting("use_custom_cpu_ticks", Settings::values.use_custom_cpu_ticks);
    LogSetting("custom_cpu_ticks", Settings::values.custom_cpu_ticks);
    LogSetting("cpu_clock_percentage", Settings::values.cpu_clock_percentage);
}

void LoadProfile(int index) {
    Settings::values.current_input_profile = Settings::values.input_profiles[index];
    Settings::values.current_input_profile_index = index;
}

void SaveProfile(int index) {
    Settings::values.input_profiles[index] = Settings::values.current_input_profile;
}

void CreateProfile(std::string name) {
    Settings::InputProfile profile = values.current_input_profile;
    profile.name = std::move(name);
    Settings::values.input_profiles.push_back(std::move(profile));
    Settings::values.current_input_profile_index =
        static_cast<int>(Settings::values.input_profiles.size()) - 1;
    Settings::LoadProfile(Settings::values.current_input_profile_index);
}

void DeleteProfile(int index) {
    Settings::values.input_profiles.erase(Settings::values.input_profiles.begin() + index);
    Settings::LoadProfile(0);
}

void RenameCurrentProfile(std::string new_name) {
    Settings::values.current_input_profile.name = std::move(new_name);
}

} // namespace Settings
