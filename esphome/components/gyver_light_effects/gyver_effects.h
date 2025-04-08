#pragma once

#include <utility>
#include <vector>

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/light/addressable_light.h"
#include "esphome/components/light/addressable_light_effect.h"
#include "esphome/components/microphone/microphone.h"
#include "palettes.h"

namespace esphome {
namespace gyver_effects {

class GyverLightSettings {
 public:
  void set_pixel_mapper(std::function<int(int, int)> &&pixel_mapper_f) { this->pixel_mapper_f_ = pixel_mapper_f; }
  std::function<int(int, int)> get_pixel_mapper() { return this->pixel_mapper_f_; }
  void set_width(uint16_t width) { this->width_ = width; }
  uint16_t get_width() { return this->width_; }
  void set_height(uint16_t height) { this->height_ = height; }
  uint16_t get_height() { return this->height_; }
  void set_microphone(microphone::Microphone *mic) { this->mic_ = mic; }
  microphone::Microphone* get_microphone() { return this->mic_; }

 protected:
  std::function<int(int, int)> pixel_mapper_f_;
  uint16_t width_{16};
  uint16_t height_{16};
  microphone::Microphone *mic_{nullptr};
};

class BaseGyverLightEffect : public light::AddressableLightEffect {
 public:
  explicit BaseGyverLightEffect(const std::string &name) : light::AddressableLightEffect(name) {}
  virtual void apply(light::AddressableLight &it, const Color &current_color) override = 0;
  void set_settings(GyverLightSettings *settings) { this->settings_ = settings; }
  size_t get_pix(uint16_t x, uint16_t y) { return this->settings_->get_pixel_mapper()(x, y); }
  uint16_t get_width() { return this->settings_->get_width(); }
  uint16_t get_height() { return this->settings_->get_width(); }

 protected:
  GyverLightSettings *settings_;
  bool use_mic_{false};
  std::unique_ptr<float_t[]> spectrum_{nullptr};
  int16_t *mic_buffer_{nullptr};
  void enable_mic_();
  void disable_mic_();
  void process_mic_();
};

class FireLightEffect : public BaseGyverLightEffect {
 public:
  explicit FireLightEffect(const std::string &name) : BaseGyverLightEffect(name) {}
  void apply(light::AddressableLight &it, const Color &current_color) override;
  void set_speed(uint8_t speed) { this->speed_ = speed; }

 protected:
  uint8_t speed_{10};
  uint8_t count_{0};
  uint8_t rgb_to_hue(const Color &current_color);
};

class PerlinLightEffect : public BaseGyverLightEffect {
 public:
  explicit PerlinLightEffect(const std::string &name) : BaseGyverLightEffect(name) {}
  void apply(light::AddressableLight &it, const Color &current_color) override;
  void start() override;
  void stop() override;
  void set_speed(uint8_t speed) { this->speed_ = speed; }
  void set_scale(uint8_t scale) { this->scale_ = scale; }
  void set_palette(TProgmemRGBGradientPalettePtr palette_ptr) { this->palette_ptr_ = palette_ptr; }

 protected:
  uint8_t speed_{10};
  uint8_t scale_{100};
  TProgmemRGBGradientPalettePtr palette_ptr_{nullptr};
  std::unique_ptr<GRGBPalette16>  palette_{nullptr};
};

class ConfettiLightEffect : public BaseGyverLightEffect {
 public:
  enum class ConfettiType { Random, Palette, Select };
  explicit ConfettiLightEffect(const std::string &name) : BaseGyverLightEffect(name) {}
  void start() override;
  void stop() override;
  void apply(light::AddressableLight &it, const Color &current_color) override;
  void set_fade_frames(uint8_t fade_frames) { this->fade_frames_ = fade_frames; }
  void set_gen_speed(float_t gen_speed) { this->gen_speed_ = gen_speed; }
  void set_update_interval(uint32_t update_interval) { this->update_interval_ = update_interval; }
  void set_palette(TProgmemRGBGradientPalettePtr palette_ptr) { this->palette_ptr_ = palette_ptr; }
  void set_type(ConfettiType type) { this->type_ = type; }

 protected:
  uint8_t fade_frames_{10};
  float_t gen_speed_{1.0};
  float_t cur_amount{0.0};
  uint32_t update_interval_{};
  uint32_t last_update_{0};
  TProgmemRGBGradientPalettePtr palette_ptr_{nullptr};
  std::unique_ptr<GRGBPalette16> palette_{nullptr};
  ConfettiType type_{ConfettiType::Random};
};

class GradientLightEffect : public BaseGyverLightEffect {
 public:
  explicit GradientLightEffect(const std::string &name) : BaseGyverLightEffect(name) {}
  void start() override;
  void stop() override;
  void apply(light::AddressableLight &it, const Color &current_color) override;
  void set_speed(uint8_t speed) { this->speed_ = speed; }
  void set_scale(uint8_t scale) { this->scale_ = scale; }
  void set_from_center(bool from_center) { this->from_center_ = from_center; }
  void set_update_interval(uint32_t update_interval) { this->update_interval_ = update_interval; }
  void set_palette(TProgmemRGBGradientPalettePtr palette_ptr) { this->palette_ptr_ = palette_ptr; }
  void set_use_microphone(bool use_mic) { this->use_mic_ = use_mic; }

 protected:
  uint8_t speed_{127};
  uint8_t scale_{127};
  bool from_center_{true};
  bool use_mic_{false};
  uint32_t update_interval_{};
  uint32_t last_update_{0};
  TProgmemRGBGradientPalettePtr palette_ptr_{nullptr};
  std::unique_ptr<GRGBPalette16> palette_{nullptr};
  std::unique_ptr<float_t[]> spectrum_{nullptr};
  int16_t *mic_buffer_{nullptr};
};

class ParticlesLightEffect : public BaseGyverLightEffect {
 public:
  explicit ParticlesLightEffect(const std::string &name) : BaseGyverLightEffect(name) {}
  void apply(light::AddressableLight &it, const Color &current_color) override;
  void set_speed(uint8_t speed) { this->speed_ = speed; }
  void set_amount(uint8_t amount) { this->amount_ = amount; }
  void set_from_center(bool from_center) { this->from_center_ = from_center; }
  void set_update_interval(uint32_t update_interval) { this->update_interval_ = update_interval; }
  void set_palette(GRGBPalette16 palette) { this->palette_ = palette; }

 protected:
  uint8_t speed_{127};
  uint8_t amount_{127};
  bool from_center_{true};
  uint32_t update_interval_{};
  uint32_t last_update_{0};
  GRGBPalette16 palette_;
};


class SpectrumLightEffect : public BaseGyverLightEffect {
  public:
   explicit SpectrumLightEffect(const std::string &name) : BaseGyverLightEffect(name) {}
   void start() override;
   void stop() override;
   void apply(light::AddressableLight &it, const Color &current_color) override;
   enum class SpectrumType { Hue, Palette, Select };
   void set_fade_speed(uint8_t speed) { this->fade_speed_ = speed; }
   void set_scale(uint8_t scale) { this->scale_ = scale; }
   void set_from_center(bool from_center) { this->from_center_ = from_center; }
   void set_update_interval(uint32_t update_interval) { this->update_interval_ = update_interval; }
   void set_palette(TProgmemRGBGradientPalettePtr palette_ptr) { this->palette_ptr_ = palette_ptr; }
   void set_use_microphone(bool use_mic) { this->use_mic_ = use_mic; }
   void set_type(SpectrumType type) { this->type_ = type; }
 
  protected:
   uint8_t fade_speed_{16};
   float_t scale_{100.0f};
   float_t noise_lvl_{40.0f};
   bool from_center_{false};
   uint32_t update_interval_{};
   uint32_t last_update_{0};
   TProgmemRGBGradientPalettePtr palette_ptr_{nullptr};
   std::unique_ptr<GRGBPalette16> palette_{nullptr};
   SpectrumType type_{SpectrumType::Select};
 };

}  // namespace gyver_effects
}  // namespace esphome