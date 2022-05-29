#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>

using namespace std;

void outputEquations(int interval, int phase1Length, int phase2Length, ostream &stream = cout) {
	int i = 1;
	for (; i < phase1Length; i++) {
		stream << i << "=" << (i - 1) << "+" << max(0, i % interval == 1 ? i - interval : i - 1 - interval) << "\n";
	}
	bool endOfPhase1GradeIncrease = phase1Length % interval == 2;
	int phase2Offset = endOfPhase1GradeIncrease ? 0 : 1;
	if (phase2Length > 0) {
		stream << i++ << "=" << ((phase1Length - 1) % interval) << "+1\n";
		phase2Offset++;
		for (int j = (phase1Length - 1) % interval + interval; j < phase1Length; j += interval, i++) {
			stream << i << "=" << (i - 1) << "+" << j << "\n";
			phase2Offset++;
		}
	}
	for (int j = 1; j < phase2Length; j++) {
		int pureGradeGem = (j == 1 && !endOfPhase1GradeIncrease) ? 1 : i - phase2Offset;
		stream << i++ << "=" << pureGradeGem << "+" << pureGradeGem << "\n";
		for (int k = 1; k < phase2Offset; k++, i++) {
			stream << i << "=" << (i - 1) << "+" << (i - phase2Offset) << "\n";
		}
	}
}

namespace GemComponent {
	enum GemComponent {
		DAMAGE,
		LEECH,
		CRIT,
		BLEED,
		BBOUND,
		LENGTH
	};
}

struct ComponentData {
	const char *name;
	double a0, a1, a2, b0, b1, b2;
};

ComponentData componentData[] = {
	{"Damage", 0.87, 0.86, 0.85, 0.71, 0.7 , 0.69},
	{"Leech" , 0.88, 0.89, 0.9 , 0.5 , 0.44, 0.38},
	{"Crit"  , 0.88, 0.88, 0.88, 0.5 , 0.44, 0.44},
	{"Bleed" , 0.81, 0.8 , 0.79, 0.35, 0.28, 0.26},
	{"Bbound", 0.78, 0.79, 0.8 , 0.31, 0.29, 0.27},
};

struct Gem {
	int grade = 0;
	double value = 1;
	double components[GemComponent::LENGTH];

	Gem() {
		for (int i = 0; i < GemComponent::LENGTH; i++) {
			components[i] = 1.0;
		}
	}

	Gem(Gem &a, Gem &b) {
		int delta = abs(a.grade - b.grade);
		grade = max(a.grade, b.grade) + (delta == 0 ? 1 : 0);
		value = a.value + b.value;
		for (int i = 0; i < GemComponent::LENGTH; i++) {
			double component1 = a.components[i], component2 = b.components[i];
			if (component1 < component2) swap(component1, component2);
			if (delta == 0)      components[i] = componentData[i].a0 * component1 + componentData[i].b0 * component2;
			else if (delta == 1) components[i] = componentData[i].a1 * component1 + componentData[i].b1 * component2;
			else                 components[i] = componentData[i].a2 * component1 + componentData[i].b2 * component2;
		}
		components[GemComponent::DAMAGE] = max(components[GemComponent::DAMAGE], max(a.components[GemComponent::DAMAGE], b.components[GemComponent::DAMAGE]));
	}

	double getGrowth(double (Gem:: *getPower)()) {
		return log((this->*getPower)()) / log(value);
	}

	double getLeechPower() {
		return components[GemComponent::LEECH];
	}

	double getCritPower() {
		return components[GemComponent::DAMAGE] * components[GemComponent::CRIT];
	}

	double getBleedPower() {
		return components[GemComponent::BLEED];
	}

	double getManaPower() {
		return components[GemComponent::LEECH] * components[GemComponent::BBOUND];
	}

	double getKillPower() {
		return components[GemComponent::DAMAGE] * components[GemComponent::CRIT] * components[GemComponent::BBOUND] * components[GemComponent::BBOUND];
	}
};

namespace GemType {
	enum GemType {
		LEECH,
		CRIT,
		BLEED,
		MANA,
		KILL,
		LENGTH
	};
}

struct GemTypeData {
	char filePrefix, gemCharacter;
	double (Gem:: *getPower)();
	int components;
};

GemTypeData gemTypeData[] = {
	{'l', 'o', &Gem::getLeechPower, (1 << GemComponent::LEECH )},
	{'c', 'y', &Gem::getCritPower , (1 << GemComponent::DAMAGE) | (1 << GemComponent::CRIT  )},
	{'b', 'r', &Gem::getBleedPower, (1 << GemComponent::BLEED )},
	{'m', 'm', &Gem::getManaPower , (1 << GemComponent::LEECH ) | (1 << GemComponent::BBOUND)},
	{'k', 'k', &Gem::getKillPower , (1 << GemComponent::DAMAGE) | (1 << GemComponent::CRIT  ) | (1 << GemComponent::BBOUND)},
};

struct GemConstructionData {
	Gem gem;
	int interval, phase1Length, phase2Length;

	void output(int gemType, ostream &stream = cout) {
		stream << "Value:	"  << gem.value << '\n';
		stream << "Growth:	"  << gem.getGrowth(gemTypeData[gemType].getPower) << '\n';
		stream << "Grade:	+" << gem.grade << '\n';
		int componentsUsed = 0;
		for (int i = 0; i < GemComponent::LENGTH; i++) {
			if (gemTypeData[gemType].components & (1 << i)) {
				stream << componentData[i].name << ":	" << gem.components[i] << '\n';
				componentsUsed++;
			}
		}
		if (componentsUsed > 1) {
			stream << "Power:	" << (gem.*gemTypeData[gemType].getPower)() << '\n';
		}
		stream << '\n';
		stream << "Equations:\n";
		stream << "0=g1" << gemTypeData[gemType].gemCharacter << '\n';
		outputEquations(interval, phase1Length, phase2Length, stream);
	}

	void outputToFile(int gemType) {
		ostringstream stream{};
		stream << gem.value;
		string filename = string{ "Output/" } + gemTypeData[gemType].filePrefix + 'c' + to_string(gem.grade) + '-' + stream.str() + ".txt";
		ofstream file{ filename, ios::trunc };
		output(gemType, file);
		file.close();
	}
};

int main() {
	int maxGrade = 199;
	vector<Gem> phase1GemsByInterval[3];
	vector<GemConstructionData> bestGems[GemType::LENGTH];
	for (int i = 0; i < 3; i++) {
		phase1GemsByInterval[i].push_back(Gem{});
	}
	for (int i = 0; i < GemType::LENGTH; i++) {
		bestGems[i].push_back(GemConstructionData{});
	}
	vector<Gem> phase2Gems;
	for (int phase1Grade = 1; phase1Grade <= maxGrade; phase1Grade++) {
		for (int interval = 4; interval <= 6; interval++) {
			vector<Gem> &phase1Gems = phase1GemsByInterval[interval - 4];
			for (int i = 0; i < interval; i++) {
				phase1Gems.push_back(Gem{ phase1Gems.back(), phase1Gems[phase1Grade == 1 ? 0 : (i == 0 ? phase1Gems.size() - interval : phase1Gems.size() - 1 - interval)] });
				Gem gem = phase1Gems.back();
				double previousGrowths[GemType::LENGTH];
				for (int j = 0; j < GemType::LENGTH; j++) {
					double growth = gem.getGrowth(gemTypeData[j].getPower);
					previousGrowths[j] = growth;
					if (phase1Grade >= bestGems[j].size()) {
						bestGems[j].push_back(GemConstructionData{ gem, interval, (int)phase1Gems.size(), 0 });
					} else if (bestGems[j][phase1Grade].gem.getGrowth(gemTypeData[j].getPower) < growth) {
						bestGems[j][phase1Grade] = GemConstructionData{ gem, interval, (int)phase1Gems.size(), 0 };
					}
				}
				if (i != interval - 1) {
					phase2Gems.clear();
					phase2Gems.push_back(phase1Gems[0]);
					for (int j = i + 1; j < phase1Gems.size(); j += interval) {
						phase2Gems.push_back(phase1Gems[j]);
					}
					for (int phase2Grade = phase1Grade + 1; phase2Grade <= maxGrade; phase2Grade++) {
						phase2Gems[0] = Gem{ phase2Gems[0], phase2Gems[0] };
						for (int j = 1; j < phase2Gems.size(); j++) {
							phase2Gems[j] = Gem{ phase2Gems[j], phase2Gems[j - 1] };
						}
						gem = phase2Gems.back();
						bool useful = false;
						for (int j = 0; j < GemType::LENGTH; j++) {
							double growth = gem.getGrowth(gemTypeData[j].getPower);
							if (growth > previousGrowths[j]) {
								useful = true;
							}
							previousGrowths[j] = growth;
							if (phase2Grade >= bestGems[j].size()) {
								bestGems[j].push_back(GemConstructionData{ gem, interval, (int)phase1Gems.size(), phase2Grade - phase1Grade });
							} else if (bestGems[j][phase2Grade].gem.getGrowth(gemTypeData[j].getPower) < growth) {
								bestGems[j][phase2Grade] = GemConstructionData{ gem, interval, (int)phase1Gems.size(), phase2Grade - phase1Grade };
							}
						}
						if (!useful) {
							break;
						}
					}
				}
			}
		}
		for (int i = 0; i < GemType::LENGTH; i++) {
			bestGems[i][phase1Grade].outputToFile(i);
		}
	}
	return 0;
}
