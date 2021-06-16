#include <cmath>

#include "rash/rash.hpp"

#include "cuckoo.hpp"


class CuckooVisualization {
public:
    CuckooHashTable<> table;
    std::vector<CuckooHashTable<>::Slot> trace;

    void insert(uint32_t key) {
        auto ptr = table.get(key);
        if (ptr)
            return;

        trace.clear();
        table.m_internal.push(0, key, 0, &trace);
    }

    void remove(uint32_t key) {
        trace.clear();
        table.remove(key);
    }

    void render(Application &app, NVGcontext *vg) const {
        constexpr float TOP = 100.0f;
        constexpr float LEFT = 200.0f;
        constexpr float WIDTH = 100.0f;
        constexpr float PAD = 10.0f;
        constexpr float MARGIN = 500.0f;

        nvgBeginPath(vg);
        for (int i = 0; i < 8; i++) {
            nvgRect(vg, LEFT, TOP + WIDTH * i, WIDTH, WIDTH);
            nvgRect(vg, LEFT + MARGIN, TOP + WIDTH * i, WIDTH, WIDTH);
        }
        nvgStrokeColor(vg, nvgRGB(0, 0, 0));
        nvgStroke(vg);

        auto draw_number = [&](float offset, int i, uint32_t x) {
            nvgBeginPath(vg);
            app.nvg_text_mode(nvgRGB(0, 0, 0), 32.0f, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            auto literal = std::to_string(x);
            nvgText(vg, LEFT + offset + WIDTH / 2, TOP + WIDTH * i + WIDTH / 2, literal.data(), nullptr);
            nvgFill(vg);
        };

        nvgBeginPath(vg);
        app.nvg_text_mode(nvgRGB(0, 0, 0), 30.0f, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgText(vg, LEFT + WIDTH / 2, TOP - PAD, "H1(x) = x mod 8", nullptr);
        nvgText(vg, LEFT + MARGIN + WIDTH / 2, TOP - PAD, "H2(x) = (x div 8) mod 8", nullptr);
        nvgFill(vg);

        auto &t = table.m_internal.table;
        for (int i = 0; i < 8; i++) {
            auto idx = std::to_string(i);

            nvgBeginPath(vg);
            app.nvg_text_mode(nvgRGB(0, 0, 0), 26.0f, NVG_ALIGN_MIDDLE | NVG_ALIGN_RIGHT);
            nvgText(vg, LEFT - PAD, TOP + WIDTH * i + WIDTH / 2, idx.data(), nullptr);
            nvgFill(vg);

            nvgBeginPath(vg);
            app.nvg_text_mode(nvgRGB(0, 0, 0), 26.0f, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
            nvgText(vg, LEFT + MARGIN + WIDTH + PAD, TOP + WIDTH * i + WIDTH / 2, idx.data(), nullptr);
            nvgFill(vg);

            if (t.used[i])
                draw_number(0.0f, i, t.keys[i]);
            if (t.used[8 + i])
                draw_number(MARGIN, i, t.keys[8 + i]);
        }

        auto arrow = [&](float sx, float sy, float tx, float ty) {
            constexpr float SIZE = 8.0f;

            float len = hypotf(sx - tx, sy - ty);
            float vx = (tx - sx) / len;
            float vy = (ty - sy) / len;

            float l = PAD / fabsf(vx);
            sx += vx * l;
            sy += vy * l;
            tx -= vx * l;
            ty -= vy * l;

            vx *= SIZE;
            vy *= SIZE;
            float ux = vy, uy = -vx;

            nvgBeginPath(vg);
            nvgMoveTo(vg, sx, sy);
            nvgLineTo(vg, tx - vx, ty - vy);
            nvgStrokeColor(vg, nvgRGB(0, 0, 0));
            nvgStrokeWidth(vg, 3.0f);
            nvgStroke(vg);

            nvgBeginPath(vg);
            nvgMoveTo(vg, tx, ty);
            nvgLineTo(vg, tx - 3 * vx + ux, ty - 3 * vy + uy);
            nvgLineTo(vg, tx - 3 * vx - ux, ty - 3 * vy - uy);
            nvgFillColor(vg, nvgRGB(0, 0, 0));
            nvgFill(vg);
        };

        auto position = [&](const CuckooHashTable<>::Slot &s) -> std::tuple<float, float> {
            auto i = s.index;
            if (s.side)
                return {LEFT + MARGIN + PAD, TOP + WIDTH * i + WIDTH / 2};
            else
                return {LEFT + WIDTH - PAD, TOP + WIDTH * i + WIDTH / 2};
        };

        for (size_t i = 0; i + 1 < trace.size(); i++) {
            auto [sx, sy] = position(trace[i]);
            auto [tx, ty] = position(trace[i + 1]);
            arrow(sx, sy, tx, ty);
        }
    }
};

int main() {
    CuckooVisualization visual;
    uint32_t new_key = 0, new_value = 0;

    Application app("Cuckoo Hashing Demo");
    app.run([&] {
        app.gl_clear(1.0f, 1.0f, 1.0f);

        auto vg = app.get_nanovg_context();
        nvgBeginFrame(vg, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f);
        visual.render(app, vg);
        nvgEndFrame(vg);

        ImGui::Begin("控制台");

        ImGui::InputScalar("键", ImGuiDataType_U32, &new_key);

        ImGui::InputScalar("值", ImGuiDataType_U32, &new_value);

        if (ImGui::Button("插入")) {
            visual.insert(new_key);
        }
        ImGui::SameLine();
        if (ImGui::Button("删除")) {
            visual.remove(new_key);
        }

        ImGui::End();
    });

    return 0;
}
