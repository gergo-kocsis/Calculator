#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <utility>
#include <stack>
#include <cassert>

std::optional<std::string> exportNextElement(const std::string& stringExpression, const std::string& prevElement, int& i)
{
	char c = stringExpression[i];
	std::string ret = "";

	// Skip spaces, these are ignored
	while (c == ' ')
		++i;

	// These operators can exist on their own, and can be returned
	if (c == '+' || c == '*' || c == '/' || c == '(' || c == ')')
	{
		++i; 
		return std::string{ c };
	}

	// If the next character is a '-', we need to check that it's in a valid place
	else if (c == '-' && (prevElement == "" || prevElement == "("))
	{
		ret += c;
		c = stringExpression[++i]; // Part of a negative number, now let's check for a number
	}

	// Extract every digit of a number
	else if (std::isdigit(c))
	{
		while (std::isdigit(c))
		{
			ret += c;
			c = stringExpression[++i];
		}
		return ret;
	}

	// Next valid element could not be found
	return std::nullopt;
}

bool createExpression(const std::string& stringExpression, std::vector<std::string>& expression)
{
	int i = 0;
	std::string prevElement = "";
	while (i < stringExpression.size())
	{
		std::optional<std::string> nextElement = exportNextElement(stringExpression, prevElement, i);
		if (!nextElement.has_value())
			return false;

		expression.push_back(nextElement.value());
		prevElement = nextElement.value();
	}

	return true;
}

int main()
{
	std::vector<std::pair<std::string, int>> testCases =
	{
		{"1+2", 3},
	};

	for (std::pair<std::string, int>& tc : testCases)
	{
		std::vector<std::string> expression;
		if (!createExpression(tc.first, expression))
			assert(false);
	}

	return 0;
}