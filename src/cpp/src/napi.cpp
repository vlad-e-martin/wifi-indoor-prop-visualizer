#include <napi.h>
#include "IndoorSimulator.h"
#include <memory>
#include <vector>

class IndoorSimulatorWrapper : public Napi::ObjectWrap<IndoorSimulatorWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        // Define the JS simulator class and its native method
        Napi::Function func = DefineClass(env, "IndoorSimulator", {
            InstanceMethod("generateHeatmap", &IndoorSimulatorWrapper::GenerateHeatmap)
        });

        Napi::FunctionReference* constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        env.SetInstanceData(constructor);

        exports.Set("IndoorSimulator", func);
        return exports;
    }

    // C++ back-end of the JS constructor
    IndoorSimulatorWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<IndoorSimulatorWrapper>(info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1 || !info[0].IsString()) {
            Napi::TypeError::New(env, "String expected for layout JSON").ThrowAsJavaScriptException();
            return;
        }

        std::string layoutJson = info[0].As<Napi::String>().Utf8Value();
        double roomHeight = info.Length() > 1 && info[1].IsNumber() ? info[1].As<Napi::Number>().DoubleValue() : 3.0;

        // Instantiate the core C++ simulation engine
        m_simulator = std::make_unique<RfSimulation::IndoorSimulator>(layoutJson, roomHeight);
    }

private:
    std::unique_ptr<RfSimulation::IndoorSimulator> m_simulator;

    // C++ back-end of the JS function simulatorWrapper.generateHeatmap(...)
    Napi::Value GenerateHeatmap(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 8) {
            Napi::TypeError::New(env, "Expected 8 arguments").ThrowAsJavaScriptException();
            return env.Null();
        }

        // Extract JS arguments into C++ primitives
        double txX = info[0].As<Napi::Number>().DoubleValue();
        double txY = info[1].As<Napi::Number>().DoubleValue();
        double txZ = info[2].As<Napi::Number>().DoubleValue();
        double freq = info[3].As<Napi::Number>().DoubleValue();
        double power = info[4].As<Napi::Number>().DoubleValue();
        int gridW = info[5].As<Napi::Number>().Int32Value();
        int gridH = info[6].As<Napi::Number>().Int32Value();
        double res = info[7].As<Napi::Number>().DoubleValue();

        // Call C++ back-end to calculate heatmap results
        std::vector<double> heatmap = m_simulator->generateHeatmap(txX, txY, txZ, freq, power, gridW, gridH, res);

        // Convert C++ types back into JS outputs
        Napi::Float64Array jsArray = Napi::Float64Array::New(env, heatmap.size());
        for (size_t i = 0; i < heatmap.size(); i++) {
            jsArray[i] = heatmap[i];
        }

        return jsArray;
    }
};

// Initialize the Node.js Addon
Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    return IndoorSimulatorWrapper::Init(env, exports);
}

NODE_API_MODULE(rf_simulator, InitAll)