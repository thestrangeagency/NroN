#include "plugin.hpp"

struct Threshold : Module
{

    float charge = 0.f;
    bool active = false;

    dsp::PulseGenerator pulseGenerator;
    bool clockPulse = false;

    dsp::SchmittTrigger clockTrigger;
    dsp::SchmittTrigger resetTrigger;
    dsp::SchmittTrigger activateTrigger;

    enum ParamId
    {
        THRESHOLD_PARAM,
        MODE_PARAM,
        PARAMS_LEN
    };
    enum InputId
    {
        RESET_INPUT,
        ACTIVATE_INPUT,
        CLOCK_INPUT,
        INPUTS_LEN
    };
    enum OutputId
    {
        LEVEL_OUTPUT,
        OUT_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId
    {
        CHARGE_LIGHT,
        ACTIVE_LIGHT,
        LIGHTS_LEN
    };

    Threshold()
    {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(THRESHOLD_PARAM, 0.f, 1.f, 0.f, "Threshold");
        configSwitch(MODE_PARAM, 0.0f, 1.0f, 1.0f, "Mode", {"Active", "Activated"});

        configInput(RESET_INPUT, "Reset");
        configInput(ACTIVATE_INPUT, "Activate");
        configInput(CLOCK_INPUT, "Clock");
        configOutput(LEVEL_OUTPUT, "Charge");
        configOutput(OUT_OUTPUT, "Trigger");
    }

    void process(const ProcessArgs &args) override
    {

        bool activated = params[MODE_PARAM].getValue() < 0.5f || active;

        Module *rightModule = getRightExpander().module;
        if (rightModule && rightModule->model == modelThresholdExpander)
        {
            float threshold_cv_amount = rightModule->getParam(ThresholdExpander::THRESHOLD_AMOUNT_PARAM).getValue();
            float threshold_cv = rightModule->getInput(ThresholdExpander::THRESHOLD_INPUT).getVoltage();
        }

        if (inputs[RESET_INPUT].isConnected())
        {
            if (resetTrigger.process(inputs[RESET_INPUT].getVoltage(), 0.1f, 1.f))
            {
                this->charge = 0;
                this->active = false;
            }
        }

        if (inputs[ACTIVATE_INPUT].isConnected())
        {
            if (activateTrigger.process(inputs[ACTIVATE_INPUT].getVoltage(), 0.1f, 1.f))
            {
                this->active = true;
            }
        }

        if (inputs[CLOCK_INPUT].isConnected())
        {
            if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage(), 0.1f, 1.f))
            {

                if (activated)
                    this->charge += 1.0 / (1 + round(63 * params[THRESHOLD_PARAM].getValue()));
            }
        }

        if (this->charge >= 1)
        {
            pulseGenerator.trigger(1e-3f);
            this->charge = 0;
            this->active = false;
        }

        clockPulse = pulseGenerator.process(args.sampleTime);
        outputs[OUT_OUTPUT].setVoltage(clockPulse ? 10.0f : 0.0f);

        lights[ACTIVE_LIGHT].setBrightness(activated ? 1.f : 0.f);
        lights[CHARGE_LIGHT].setBrightness(charge);

        outputs[LEVEL_OUTPUT].setVoltage(charge * 10.f);
    }
};

struct ThresholdWidget : ModuleWidget
{
    ThresholdWidget(Threshold *module)
    {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Threshold.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 49.768)), module, Threshold::THRESHOLD_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(15.24, 63.656)), module, Threshold::MODE_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 77.544)), module, Threshold::RESET_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 91.432)), module, Threshold::ACTIVATE_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 105.32)), module, Threshold::CLOCK_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 35.88)), module, Threshold::LEVEL_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 119.208)), module, Threshold::OUT_OUTPUT));

        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(9.948, 21.992)), module, Threshold::ACTIVE_LIGHT));
        addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(20.532, 21.992)), module, Threshold::CHARGE_LIGHT));
    }
};

Model *modelThreshold = createModel<Threshold, ThresholdWidget>("Threshold");