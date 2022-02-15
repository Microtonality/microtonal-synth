#include <string>
using namespace std;
class Mapping {
public:
    int index;
    double frequency;
};
class MicrotonalConfig {
public:
    double base_frequency = 440.000000;
    double divisions = 12;
    Mapping frequencies[12];

    vector<double> getAllFrequencies() {
        vector<double> freq;
		for (int i = 0; i <= divisions; i++) {
			double step_calc = (i / divisions);
			freq.push_back(base_frequency * pow(2, step_calc));
		}
        return freq;
    }
    juce::XmlElement generateXML() {
        string writeToXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        writeToXML = writeToXML + "<microtonalConfig base_frequency=\"" + to_string(base_frequency) + "\" total_divisions=\"" + to_string(divisions) + "\">\n";
        for (int i = 0; i < 12; i++) {
            if (frequencies[i].frequency == NULL) continue;
            writeToXML = writeToXML + "\t<frequency index=\"" + to_string(i) + "\">" + to_string(frequencies[i].frequency) + "</frequency>\n";
        }
        writeToXML = writeToXML + "</microtonalConfig>";
        
        return *juce::XmlDocument(writeToXML).getDocumentElement();
    }
};
