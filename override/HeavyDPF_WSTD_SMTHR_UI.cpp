/**
 * Copyright (c) Wasted Audio 2023 - GPL-3.0-or-later
 */

#include "DistrhoUI.hpp"
#include "ResizeHandle.hpp"
#include "veramobd.hpp"
#include "wstdcolors.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------
class ImGuiPluginUI : public UI
{
    int fcrshr = 512;
    float ffldr = 1.0f;
    bool flmtr = 1.0f != 0.0f;
    float fmix = 50.0f;
    float fsmthr = 1.0f;
    int fsqnc = 0.0;

    ResizeHandle fResizeHandle;

    // ----------------------------------------------------------------------------------------------------------------

public:
   /**
      UI class constructor.
      The UI should be initialized to a default state that matches the plugin side.
    */
    ImGuiPluginUI()
        : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT, true),
          fResizeHandle(this)
    {
        setGeometryConstraints(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT, true);

        ImGuiIO& io(ImGui::GetIO());

        ImFontConfig fc;
        fc.FontDataOwnedByAtlas = true;
        fc.OversampleH = 1;
        fc.OversampleV = 1;
        fc.PixelSnapH = true;

        io.Fonts->AddFontFromMemoryCompressedTTF((void*)veramobd_compressed_data, veramobd_compressed_size, 16.0f * getScaleFactor(), &fc);
        io.Fonts->AddFontFromMemoryCompressedTTF((void*)veramobd_compressed_data, veramobd_compressed_size, 21.0f * getScaleFactor(), &fc);
        io.Fonts->AddFontFromMemoryCompressedTTF((void*)veramobd_compressed_data, veramobd_compressed_size, 12.5f * getScaleFactor(), &fc);
        io.Fonts->Build();
        io.FontDefault = io.Fonts->Fonts[1];

        fResizeHandle.hide();
    }

protected:
    // ----------------------------------------------------------------------------------------------------------------
    // DSP/Plugin Callbacks

   /**
      A parameter has changed on the plugin side.@n
      This is called by the host to inform the UI about parameter changes.
    */
    void parameterChanged(uint32_t index, float value) override
    {
        switch (index) {
            case 0:
                flmtr = value != 0.0f;
                break;
            case 1:
                fmix = value;
                break;
            case 2:
                fsmthr = value;
                break;

            default: return;
        }

        repaint();
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Widget Callbacks

   /**
      ImGui specific onDisplay function.
    */
    void onImGuiDisplay() override
    {
        const float width = getWidth();
        const float height = getHeight();
        const float margin = 0.0f;
        auto scaleFactor = getScaleFactor();

        ImGui::SetNextWindowPos(ImVec2(margin, margin));
        ImGui::SetNextWindowSize(ImVec2(width - 2 * margin, height - 2 * margin));

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style.ChildBorderSize = 1.0f * scaleFactor;

        style.Colors[ImGuiCol_TitleBgActive] = (ImVec4)WstdTitleBgActive;
        style.Colors[ImGuiCol_WindowBg] = (ImVec4)WstdWindowBg;

        ImGuiIO& io(ImGui::GetIO());
        ImFont* defaultFont = ImGui::GetFont();
        ImFont* titleBarFont = io.Fonts->Fonts[2];
        ImFont* mediumFont = io.Fonts->Fonts[3];

        auto intense         = (fsmthr * 7.5f - 20.0f) / 5.0f;
        auto SmthrActive     = ColorBright(Blue, intense);
        auto SmthrHovered    = ColorBright(BlueBr, intense);

        auto SyncSw          = ColorBright(WhiteDr, intense);
        auto SyncGr          = ColorBright(Grey, intense);
        auto SyncGrHovered   = ColorBright(GreyBr, intense);
        auto SyncAct         = ColorBright(GreenDr, intense);
        auto SyncActHovered  = ColorBright(Green, intense);
        auto MixActive       = ColorMix(Green, Yellow, intense, fmix);
        auto MixHovered      = ColorMix(GreenBr, YellowBr, intense, fmix);

        const float hundred = 100 * scaleFactor;
        const float toggleWidth = 20 * scaleFactor;
        const float comboWidth = 42 * scaleFactor;

        auto elevstep = 0.1f;
        auto percstep = 1.0f;

        if (io.KeyShift)
        {
            elevstep = 0.01f;
            percstep = 0.1f;
        }

        ImGui::PushFont(titleBarFont);
        if (ImGui::Begin("WSTD SMTHR", nullptr, ImGuiWindowFlags_NoResize + ImGuiWindowFlags_NoCollapse + ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::Dummy(ImVec2(0.0f, 6.0f * scaleFactor));
            ImGui::PushFont(defaultFont);
            auto ImGuiKnob_Flags = ImGuiKnobFlags_DoubleClickReset + ImGuiKnobFlags_ValueTooltip + ImGuiKnobFlags_NoInput + ImGuiKnobFlags_ValueTooltipHideOnClick;
            auto ImGuiKnob_FlagsLog = ImGuiKnob_Flags + ImGuiKnobFlags_Logarithmic;

            ImGui::PushStyleColor(ImGuiCol_ButtonActive,    (ImVec4)SmthrActive);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   (ImVec4)SmthrHovered);
            if (ImGuiKnobs::Knob(
                "Smthr", &fsmthr, 1.0f, 13.37f, elevstep, "%.2f",
                ImGuiKnobVariant_SteppedTick, hundred, ImGuiKnob_Flags, 11))
            {
                if (ImGui::IsItemActivated())
                {
                    editParameter(2, true);
                    if (ImGui::IsMouseDoubleClicked(0))
                        fsmthr = 1.0f;
                }
                setParameterValue(2, fsmthr);
            }
            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                // Title text
                ImGui::PushStyleColor(ImGuiCol_Text, TextClr);
                CenterTextX("Lmtr", toggleWidth);
                ImGui::PopStyleColor();

                ImGui::Dummy(ImVec2(0.0f, 35.0f) * scaleFactor);

                // knob
                ImGui::PushStyleColor(ImGuiCol_Text,            (ImVec4)SyncSw);

                // inactive colors
                ImGui::PushStyleColor(ImGuiCol_FrameBg,         (ImVec4)SyncGr);
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,  (ImVec4)SyncGrHovered);

                // active colors
                ImGui::PushStyleColor(ImGuiCol_Button,          (ImVec4)SyncAct);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   (ImVec4)SyncActHovered);
                if (ImGui::Toggle("##Lmtr", &flmtr, ImGuiToggleFlags_Animated))
                {
                    if (ImGui::IsItemActivated())
                    {
                        editParameter(0, true);
                        setParameterValue(0, flmtr);
                    }
                }
                ImGui::PopStyleColor(5);
            }
            ImGui::EndGroup();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_ButtonActive,    (ImVec4)MixActive);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   (ImVec4)MixHovered);
            if (ImGuiKnobs::Knob(
                "Mix", &fmix, 0.0f, 100.0f, percstep, "%.1f%%", ImGuiKnobVariant_SteppedTick, hundred, ImGuiKnob_Flags, 11))
            {
                if (ImGui::IsItemActivated())
                {
                    editParameter(1, true);
                    if (ImGui::IsMouseDoubleClicked(0))
                        fmix = 50.0f;
                }
                setParameterValue(1, fmix);
            }
            ImGui::PopStyleColor(2);


            if (ImGui::IsItemDeactivated())
            {
                editParameter(0, false);
                editParameter(1, false);
                editParameter(2, false);
            }
        }
        ImGui::End();
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImGuiPluginUI)
};

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new ImGuiPluginUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
