#include "plugin.hpp"

struct ThresholdExpander : Module
{
    enum ParamId
    {
        THRESHOLD_AMOUNT_PARAM,
        PARAMS_LEN
    };
    enum InputId
    {
        THRESHOLD_INPUT,
        INPUTS_LEN
    };
    enum OutputId
    {
        OUTPUTS_LEN
    };
    enum LightId
    {
        LIGHTS_LEN
    };

    ThresholdExpander()
    {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(THRESHOLD_AMOUNT_PARAM, -1.f, 1.f, 0.f, "Threshold CV Amount");
        configInput(THRESHOLD_INPUT, "Threshold CV");
    }

    void process(const ProcessArgs &args) override
    {
    }
};

struct ThresholdExpanderWidget : ModuleWidget
{
    ThresholdExpanderWidget(ThresholdExpander *module)
    {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/ThresholdExpander.svg")));

        // addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 35.88)), module, ThresholdExpander::THRESHOLD_AMOUNT_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 49.768)), module, ThresholdExpander::THRESHOLD_INPUT));
    }
};

Model *modelThresholdExpander = createModel<ThresholdExpander, ThresholdExpanderWidget>("ThresholdExpander");