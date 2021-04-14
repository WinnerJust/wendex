#include <string>
#include <vector>
#include <utility>
#include <algorithm>


int levenstein(std::string s1, std::string s2) {
	//First column and row should be empty string
	//Because otherwise to fill the first column and the first row we will need a condition

	std::vector<std::vector<int>> l(s1.size() + 1, std::vector<int>(s2.size() + 1));
	for (int i = 1; i < s1.size() + 1; i++)
		l[i][0] = 1;
	for (int i = 1; i < s2.size() + 1; i++)
		l[0][i] = 1;

	for (int i = 1; i < s1.size() + 1; i++) {
		for (int j = 1; j < s2.size() + 1; j++) {
			std::vector<int> res = { l[i - 1][j] + 1, l[i][j - 1] + 1, l[i - 1][j - 1] + (s1[i] != s2[j]) };
			l[i][j] = *std::min_element(res.begin(), res.end());
		}
	}

	return l[s1.size()][s2.size()];
}