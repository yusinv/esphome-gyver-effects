#include "gyver_effects.h"
#include "palettes.h"
#include "esphome/core/log.h"
#include "noise.h"
#include "dsps_fft2r.h"

namespace esphome {
namespace gyver_effects {

// clang-format off
const uint8_t FIRE_VALUE_MASK[11][8] PROGMEM = {
  {8  , 0  , 0  , 0  , 0  , 0  , 0  , 8   },
  {16 , 0  , 0  , 0  , 0  , 0  , 0  , 16  },
  {32 , 0  , 0  , 0  , 0  , 0  , 0  , 32  },
  {64 , 0  , 0  , 0  , 0  , 0  , 0  , 64  },
  {96 , 32 , 0  , 0  , 0  , 0  , 32 , 96  },
  {128, 64 , 32 , 0  , 0  , 32 , 64 , 128 },
  {160, 96 , 64 , 32 , 32 , 64 , 96 , 160 },
  {192, 128, 96 , 64 , 64 , 96 , 128, 192 },
  {255, 160, 128, 96 , 96 , 128, 160, 255 },
  {255, 192, 160, 128, 128, 160, 192, 255 },
  {255, 220, 185, 150, 150, 185, 220, 255 },
};

const uint8_t FIRE_HUE_MASK[11][8] PROGMEM = {
  {8 , 16, 32, 36, 36, 32, 16, 8 },
  {5 , 14, 29, 31, 31, 29, 14, 5 },
  {1 , 11, 19, 25, 25, 22, 11, 1 },
  {1 , 8 , 13, 19, 25, 19, 8 , 1 },
  {1 , 8 , 13, 16, 19, 16, 8 , 1 },
  {1 , 5 , 11, 13, 13, 13, 5 , 1 },
  {1 , 5 , 11, 11, 11, 11, 5 , 1 },
  {0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 },
  {0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 },
  {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 },
  {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
};
// clang-format on

static const size_t SAMPLE_RATE_HZ = 16000;
static const size_t INPUT_BUFFER_SIZE = 32 * SAMPLE_RATE_HZ / 1000;  // 32ms * 16kHz / 1000ms
static const size_t SPECTRUM_SIZE = INPUT_BUFFER_SIZE / 2;

void FireLightEffect::apply(light::AddressableLight &it, const Color &current_color) {
  auto current_hue = rgb_to_hue(current_color);
  if (this->count_ >= 100) {
    for (uint16_t y = std::min((uint16_t) 10, this->settings_->get_height()); y > 0; y--) {
      for (int x = 0; x < this->get_width(); x++) {
        it[this->get_pix(x, y)].set_effect_data(it[this->get_pix(x, y - 1)].get_effect_data());
      }
    }
    for (int x = 0; x < this->get_width(); x++) {
      auto view = it[this->get_pix(x, 0)];
      view.set_effect_data(64 + random_uint32() % 192);
      view = GHSV(current_hue + progmem_read_byte(&(FIRE_HUE_MASK[0][x % 8])),  // H
                  255,                                                          // S
                  view.get_effect_data()                                        // V
      );
    }
    this->count_ = 0;
  }
  this->count_ += this->speed_;

  int nextv;
  for (uint8_t y = this->get_height() - 1; y > 0; y--) {
    for (uint8_t x = 0; x < this->get_width(); x++) {
      auto view1 = it[this->get_pix(x, y)];
      auto view0 = it[this->get_pix(x, y - 1)];
      if (y < 11) {
        nextv = (((100.0 - this->count_) * view1.get_effect_data() + this->count_ * view0.get_effect_data()) / 100.0) -
                progmem_read_byte(&(FIRE_VALUE_MASK[y][x % 8]));

        view1 = GHSV(rgb_to_hue(current_color) + progmem_read_byte(&(FIRE_HUE_MASK[y][x % 8])),  // H
                     255,                                                                        // S
                     (uint8_t) std::max(0, nextv)                                                // V
        );
      } else if (y == 11) {
        if (random_uint32() % 20 == 0 && view0.get() != Color::BLACK) {
          view1 = view0.get();
        } else {
          view1 = Color::BLACK;
        }
      } else {
        if (view0.get() != Color::BLACK) {
          view1 = view0.get();
        } else {
          view1 = Color::BLACK;
        }
      }
    }
  }
  it.schedule_show();
}

uint8_t FireLightEffect::rgb_to_hue(const Color &current_color) {
  int max_color_value = std::max(std::max(current_color.red, current_color.green), current_color.blue);
  int min_color_value = std::min(std::min(current_color.red, current_color.green), current_color.blue);
  int delta = max_color_value - min_color_value;

  if (delta == 0) {
    return 0;
  } else if (max_color_value == current_color.red) {
    return (128 * (current_color.green - current_color.blue) / (delta * 3) + 256) % 256;
  } else if (max_color_value == current_color.green) {
    return ((128 * (current_color.blue - current_color.red) / delta + 256) / 3) % 256;
  } else if (max_color_value == current_color.blue) {
    return ((128 * (current_color.red - current_color.green) / delta + 512) / 3) % 256;
  }
  return 0;
}

void PerlinLightEffect::start() {
  if (this->palette_ptr_ != nullptr) {
    this->palette_ = std::make_unique<GRGBPalette16>(this->palette_ptr_);
  }
  AddressableLightEffect::start();
}

void PerlinLightEffect::stop() {
  this->palette_.reset();
  AddressableLightEffect::stop();
}

void PerlinLightEffect::apply(light::AddressableLight &it, const Color &current_color) {
  for (uint8_t y = 0; y < this->get_height(); y++) {
    for (uint8_t x = 0; x < this->get_width(); x++) {
      it[this->get_pix(x, y)] = ColorFromPalette(
          *(this->palette_),
          inoise8(x * (this->scale_ / 5) - this->get_width() * (this->scale_ / 5) / 2,
                  y * (this->scale_ / 5) - this->get_height() * (this->scale_ / 5) / 2, millis() * this->speed_ / 255),
          255, LINEARBLEND);
    }
  }
  it.schedule_show();
}


void ConfettiLightEffect::start() {
  if (this->palette_ptr_ != nullptr) {
    this->palette_ = std::make_unique<GRGBPalette16>(this->palette_ptr_);
  }
  AddressableLightEffect::start();
}

void ConfettiLightEffect::stop() {
  this->palette_.reset();
  AddressableLightEffect::stop();
}

void ConfettiLightEffect::apply(light::AddressableLight &it, const Color &current_color) {
  const uint32_t now = millis();
  if (now - this->last_update_ < this->update_interval_)
    return;
  this->last_update_ = now;
  int curr_val = static_cast<int>(this->cur_amount);
  this->cur_amount -= curr_val;
  for (int i = 0; i < curr_val; i++) {
    int x = random_uint32() % it.size();
    if (it[x].get_effect_data() == 0) {
      switch (this->type_) {
        case ConfettiType::Select: {
          it[x] = current_color;
          break;
        }
        case ConfettiType::Palette: {
          it[x] = ColorFromPalette(*(this->palette_), i * 255 / curr_val, 255, LINEARBLEND);
          break;
        }
        case ConfettiType::Random:
        default:
          it[x] = Color::random_color();
      }
      it[x].set_effect_data(this->fade_frames_);
    }
  }
  this->cur_amount += this->gen_speed_;
  for (auto c : it) {
    auto ff = c.get_effect_data();
    if (ff == 0) {
      c.set(Color::BLACK);
    } else {
      c.set_effect_data(ff - 1);
      c.fade_to_black(255 / ff);
    }
  }
  it.schedule_show();
}

void GradientLightEffect::start() {
  if (this->palette_ptr_ != nullptr) {
    this->palette_ = std::make_unique<GRGBPalette16>(this->palette_ptr_);
  }
  AddressableLightEffect::start();
}

void GradientLightEffect::stop() {
  this->palette_.reset();
  AddressableLightEffect::stop();
}

void GradientLightEffect::apply(light::AddressableLight &it, const Color &current_color) {

  uint8_t bright{255};

  const uint32_t now = millis();
  if (now - this->last_update_ < this->update_interval_)
    return;
  this->last_update_ = now;

  
  if (this->from_center_) {  // from center
    for (uint32_t y = this->get_height() / 2; y < this->get_height(); y++) {
      Color crgb =
          ColorFromPalette(*(this->palette_), y * this->scale_ / this->get_height() + now * (this->speed_ - 127) / 800,
                           bright, LINEARBLEND);
      for (uint16_t x = 0; x < this->get_width(); x++) {
        it[this->get_pix(x, y)] = crgb;
      }
    }
    for (uint32_t y = 0; y < this->get_height() / 2; y++) {
      for (uint16_t x = 0; x < this->get_width(); x++) {
        it[this->get_pix(x, y)] = it[this->get_pix(x, this->get_height() - 1 - y)].get();
      }
    }
  } else {
    for (uint32_t y = 0; y < this->get_height(); y++) {
      Color crgb = ColorFromPalette(*(this->palette_), y * 127 / this->get_height() + now * (this->speed_ - 127) / 800,
                                    bright, LINEARBLEND);
      for (uint16_t x = 0; x < this->get_width(); x++) {
        it[this->get_pix(x, y)] = crgb;
      }
    }
  }

  it.schedule_show();
}



void SpectrumLightEffect::start() {
  if (this->palette_ptr_ != nullptr) {
    this->palette_ = std::make_unique<GRGBPalette16>(this->palette_ptr_);
  }
  this->enable_mic_();
  AddressableLightEffect::start();
}

void SpectrumLightEffect::stop() {
  this->disable_mic_();
  this->palette_.reset();
  AddressableLightEffect::stop();
}

void SpectrumLightEffect::apply(light::AddressableLight &it, const Color &current_color) {
  
  this->process_mic_();

  const uint32_t now = millis();
  if (now - this->last_update_ < this->update_interval_)
    return;
  this->last_update_ = now;

  it.all().darken(this->fade_speed_);

  int bucket_size = SPECTRUM_SIZE / this->get_width();
  uint8_t hue_delta = 255 / this->get_width();
  for (int i=0;i<this->get_width();i++){
    float_t bucket_value{0};  
    for (int j=0;j<bucket_size;j++) {
      bucket_value += this->spectrum_[i*bucket_size+j];
    }
    int16_t y_val = ((float_t)this->get_height() * bucket_value / (float_t)bucket_size - this->noise_lvl_) / this->scale_;
    
    if(this->from_center_){
      int16_t height = this->get_height()/2;
      for (uint16_t y = 0; y < std::min(y_val,height); y++) {
        Color tc = Color::BLACK;
        switch (this->type_)
        {
        case SpectrumType::Hue:
          tc = GHSV(i*hue_delta,255,255).to_rgb();
          break;
        case SpectrumType::Palette:
          tc = ColorFromPalette(*(this->palette_), y * 255 / height, 255, LINEARBLEND);
          break;      
        default:
          tc = current_color;
          break;
        }
        it[this->get_pix(i,height + y)] = tc;
        it[this->get_pix(i,height - y)] = tc;
      }
    
    } else {
      for (int16_t y=0;y<std::min(y_val,(int16_t)this->get_height());y++){
        Color tc = Color::BLACK;
        switch (this->type_)
        {
        case SpectrumType::Hue:
          tc = GHSV(i*hue_delta,255,255).to_rgb();
          break;
        case SpectrumType::Palette:
          tc = ColorFromPalette(*(this->palette_), y * 255 / this->get_height(), 255, LINEARBLEND);
          break;      
        default:
          tc = current_color;
          break;
        }
        it[this->get_pix(i,y)] = tc;
      }
    }
  }
  
  it.schedule_show();
}

void ParticlesLightEffect::apply(light::AddressableLight &it, const Color &current_color) {
  const uint32_t now = millis();
  if (now - this->last_update_ < this->update_interval_)
    return;
  this->last_update_ = now;
  it.all().fade_to_black(70);
  // uint16_t rnd_value = 0;
  // for (uint8_t i = 0; i < this->amount_; i++) {
  //   // rnd_value = rnd_value * 2053 + 13849;  // random2053
  //   int home_x = inoise16(i * 100000000ul + 32 * now * this->speed_ / 255);
  //   home_x = remap(home_x, 15000, 50000, (uint16_t) 0, this->get_width());
  //   int offset_x = inoise8(i * 2500 + now * this->speed_ / 255 / 100) - 128;
  //   offset_x = this->get_width() / 2 * offset_x / 128;
  //   offset_x = random8()%8-4;
  //   int x = home_x + offset_x;
  //   // if(i == 1){
  //   // esph_log_d("TT","%d , %d, %d",home_x,offset_x,x);}
  //   int home_y = inoise16(i * 100000000ul + 2000000000ul + 32 * now * this->speed_ / 255);
  //   home_y = remap(home_y, 15000, 50000, (uint16_t) 0, this->get_height());
  //   int offset_y = inoise8(i * 2500 + 30000 + now * this->speed_ / 255 / 100 ) - 128;
  //   // if(i == 1){
  //   // esph_log_d("TP","%d",offset_y);}
  //   offset_y = this->get_height() / 2 * offset_y / 128;
  //   int y = home_y + offset_y;

  // if(i == 1){
  // esph_log_d("TX","%d , %d, %d",home_y,offset_y,y);}
  //  it[this->get_pix(x, y)] = current_color;
  // setPix(x, y, CUR_PRES.fromPal ?
  //         ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(i * 255 / amount), 255, LINEARBLEND) :
  //         CHSV(CUR_PRES.color, 255, 255)
  //       );
  //}
  it.schedule_show();
}

void BaseGyverLightEffect::enable_mic_()
{
  if (this->use_mic_) {
    this->spectrum_ = std::make_unique<float[]>(SPECTRUM_SIZE);
    this->mic_buffer_ = (int16_t *)aligned_alloc(16, (INPUT_BUFFER_SIZE + 16) * sizeof(int16_t) * 2);
    esp_err_t ret = dsps_fft2r_init_sc16(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    if (ret != ESP_OK) {
        esph_log_d("gyver_effects", "Not possible to initialize FFT esp-dsp from library!");
        return;
    }
    this->settings_->get_microphone()->start();
  }
}

void BaseGyverLightEffect::process_mic_()
{
  if (this->use_mic_)
  {
    auto mic = this->settings_->get_microphone();
    if (mic->is_running())
    {
      auto bytes_read = mic->read(this->mic_buffer_, INPUT_BUFFER_SIZE * sizeof(int16_t));
      if (bytes_read > 0)
      {
        for (int i = bytes_read / sizeof(int16_t) - 1; i >= 0; i--)
        {
          this->mic_buffer_[i * 2] = this->mic_buffer_[i];
          this->mic_buffer_[i * 2 + 1] = 0;
        }

        // Call FFT bit reverse
        dsps_fft2r_sc16_ae32(this->mic_buffer_, INPUT_BUFFER_SIZE);
        dsps_bit_rev_sc16_ansi(this->mic_buffer_, INPUT_BUFFER_SIZE);
        // Convert spectrum from two input channels to two
        // spectrums for two channels.
        dsps_cplx2reC_sc16(this->mic_buffer_, INPUT_BUFFER_SIZE);

        float_t mx = 0;
        for (int i = 0; i < SPECTRUM_SIZE; i++)
        {
          float_t sp = this->mic_buffer_[i * 2] * this->mic_buffer_[i * 2] + this->mic_buffer_[i * 2 + 1] * this->mic_buffer_[i * 2 + 1];
          sp = 10 * log10f(0.1 + sp);
          this->spectrum_[i] = 0.8 * this->spectrum_[i] + 0.2 * sp;
        }
      }
    }
  }
}

void BaseGyverLightEffect::disable_mic_()
{
  if (this->use_mic_){
    this->settings_->get_microphone()->stop();
    dsps_fft2r_deinit_sc16();
    if(this->mic_buffer_ != nullptr){
      free(this->mic_buffer_);
      this->mic_buffer_ = nullptr;
    }
    this->spectrum_.reset();
  }
}

} // namespace gyver_effects
}  // namespace esphome