#include "plugin.hpp"


struct Threshold : Module {
	enum ParamId {
		THRESHOLD_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RESET_INPUT,
		CLOCK_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		CHARGE_LIGHT,
		LIGHTS_LEN
	};

    float charge = 0.f;

    dsp::PulseGenerator pulseGenerator;
    bool clockPulse = false;

    dsp::SchmittTrigger clockTrigger;
    dsp::SchmittTrigger resetTrigger;

	Threshold() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(THRESHOLD_PARAM, 0.f, 1.f, 0.f, "Threshold");
		configInput(RESET_INPUT, "Reset");
		configInput(CLOCK_INPUT, "Clock");
		configOutput(OUT_OUTPUT, "Trigger");
	}

	void process(const ProcessArgs& args) override {

        if (inputs[RESET_INPUT].isConnected()) {
            if (resetTrigger.process(inputs[RESET_INPUT].getVoltage(), 0.1f, 1.f)) {
                this->charge = 0;
            }
        }

        if (inputs[CLOCK_INPUT].isConnected()) {
            if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage(), 0.1f, 1.f)) {
                this->charge += 1.0 / (1 + round(63 * params[THRESHOLD_PARAM].getValue()));
            }
        }

        if (this->charge >= 1) {
            pulseGenerator.trigger(1e-3f);
            this->charge = 0;
        }

        clockPulse = pulseGenerator.process(args.sampleTime);
        outputs[OUT_OUTPUT].setVoltage(clockPulse ? 10.0f : 0.0f);    
  
        lights[CHARGE_LIGHT].setBrightness(charge);
	}
};


struct ThresholdWidget : ModuleWidget {
	ThresholdWidget(Threshold* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Threshold.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 43.524)), module, Threshold::THRESHOLD_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 64.25)), module, Threshold::RESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 84.976)), module, Threshold::CLOCK_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 105.701)), module, Threshold::OUT_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(15.24, 22.799)), module, Threshold::CHARGE_LIGHT));
	}
};


Model* modelThreshold = createModel<Threshold, ThresholdWidget>("Threshold");