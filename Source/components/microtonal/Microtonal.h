#include <string>
using namespace std;
class Mapping {
public:
    int index = NULL;
    double frequency = NULL;
};
class MicrotonalConfig {
public:
    double base_frequency;
    double divisions;
    Mapping frequencies[12];
    /* default constructor */
    MicrotonalConfig() {
        this->base_frequency = 261.63;
        this->divisions = 12;
    };
    /* pass base frequency and total divisions */
    MicrotonalConfig(double basef, double div) {
        this->base_frequency = basef;
        this->divisions = div;
    };
    /* Pass base frequency, total divisions, and int array of indices for values */
    MicrotonalConfig(double basef, double div, int* indices, int length) {
        this->base_frequency = basef;
        this->divisions = div;
        vector<double> allFreq = this->getAllFrequencies();
        for (int i = 0; i < length; i++) {
            frequencies[i].frequency = allFreq[indices[i]];
            frequencies[i].index = indices[i];
        }
    };
    /* Pass base frequency, total divisions, and double array of frequencies */
    MicrotonalConfig(double basef, double div, double* f, int length) {
        this->base_frequency = basef;
        this->divisions = div;
        vector<double> allFreq = this->getAllFrequencies();
        for (int i = 0; i < length; i++) {
            frequencies[i].frequency = f[i];
            frequencies[i].index = getIndex(allFreq, f[i]);
        }
    };
    int getIndex(vector<double> v, double K)
    {
        auto it = find(v.begin(), v.end(), K);
        if (it != v.end()){
            int index = it - v.begin();
            return index;
        }
        else {
            return -1;
        }
    }
    bool isMapped() {
        for (Mapping m : frequencies) {
            if (m.frequency != NULL) return true;
        }
        return false;
    }
    vector<double> getAllFrequencies() {
        vector<double> freq;
		for (int i = 0; i <= divisions; i++) {
			double step_calc = (i / divisions);
			freq.push_back(base_frequency * pow(2, step_calc));
		}
        return freq;
    }
    juce::ValueTree generateValueTree() {
        // string writeToXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        juce::ValueTree t{"Preset"};
        t.setProperty("base_frequency", juce::String(base_frequency), nullptr);
        t.setProperty("total_divisions", juce::String(divisions), nullptr);
        for (int i = 0; i < 12; i++) {
            if (frequencies[i].frequency == NULL) continue;
            juce::ValueTree child{ "frequency" };
            child.setProperty("index", juce::String(i), nullptr);
            child.setProperty("value", juce::String(frequencies[i].frequency), nullptr);
            t.appendChild(child, nullptr);
        }
        
        return t;
    }
};
